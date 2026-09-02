---

# Architecture & Design Document: MusicPlayerTesterApp

`MusicPlayerTesterApp` hosts the HTTP server and real-time dispatcher for the **Virtual 61-Key Synthesizer & Sargam Workstation**. It handles platform-independent socket communications, HTTP protocol decoding, static asset delivery, and multi-threaded audio task scheduling.

---

## 1. Class Architecture & Roles

```mermaid
classDiagram
    direction TB

    class SocketPlatform {
        <<namespace / abstraction>>
        +socket_t
        +InvalidSocket : const
        +SocketError : const
        +CloseSocketHandle(socket_t sock) int
        +OpenBrowser(string url) void
    }

    class HttpServer {
        -int m_port
        -int m_backlog
        -socket_t m_listenSocket
        -atomic~bool~ m_running
        -bool m_wsaInitialized
        +HttpServer(int port, int backlog)
        +~HttpServer()
        +Start() bool
        +Run(RequestHandler handler) void
        +Stop() void
        +GetPort() int
        +IsRunning() bool
        -Cleanup() void
    }

    class HttpRequest {
        +string method
        +string path
        +string query
        +string version
        +bool isValid
        +Parse(string rawRequest)$ HttpRequest
    }

    class HttpRoutes {
        <<namespace / constants>>
        +Methods : struct
        +Paths : struct
        +QueryParams : struct
        +Defaults : struct
    }

    class HttpUtils {
        <<namespace / helpers>>
        +LoadFile(string path)$ string
        +UrlDecode(string str)$ string
        +GetQueryParam(string query, string key)$ string
        +GetMimeType(string path)$ string
        +ResolveAssetPath(string relativePath)$ string
        +TryLoadAsset(string relativePath, string& outContent)$ bool
        +MakeHttpResponse(string contentType, string body, string statusLine)$ string
    }

    class MainDispatcher {
        <<executable / coordinator>>
        +main() int
        +ParsePlayParameters()
        +ParseLoopParameters()
        +MapPathToAssetFilename()
    }

    class ThreadPool {
        <<imported from MusicBuilderBL>>
        +Enqueue(Task task) void
    }

    class DrumLoopEngine {
        <<imported from MusicBuilderBL>>
        +Start(string pattern, double bpm) void
        +Stop() void
    }

    class InstrumentManager {
        <<imported from MusicBuilderBL>>
        +GetInstrument(string name) shared_ptr~IMusicInstrument~
    }

    HttpServer ..> SocketPlatform : uses platform types & wrappers
    MainDispatcher ..> HttpServer : instantiates & drives loop
    MainDispatcher ..> HttpRequest : parses inbound byte stream
    MainDispatcher ..> HttpRoutes : compares route tokens
    MainDispatcher ..> HttpUtils : parses params & formats responses
    MainDispatcher --> ThreadPool : enqueues non-blocking audio tasks
    MainDispatcher --> DrumLoopEngine : triggers rhythm sequencer
    MainDispatcher --> InstrumentManager : resolves sound models

```

### Class & Module Responsibilities

* **`SocketPlatform.h`**: Encapsulates OS differences between Windows (`winsock2.h`, `ws2_32.lib`) and Linux/POSIX (`<sys/socket.h>`, `<unistd.h>`). It standardizes the socket handle type as `socket_t` and provides cross-platform helpers like `CloseSocketHandle` and `PlatformUtils::OpenBrowser`.
* **`HttpServer`**: Manages the socket lifecycle. Handles bind, listen, client TCP connection acceptance, and TCP non-buffering (`TCP_NODELAY`). It delegates incoming payload interpretation to a functional `RequestHandler` callback.
* **`HttpRequest`**: Represents an HTTP 1.1 request line. Splits the incoming wire format into `method`, target `path`, trailing `query` parameters, and HTTP `version`.
* **`HttpRoutes.h`**: Central repository for routing identifiers, query parameter keys, HTTP methods, and default responses. Eliminates hardcoded magic strings from the application logic.
* **`HttpUtils`**: Provides utility routines including percent-encoding decoding (`UrlDecode`), query string extraction (`GetQueryParam`), MIME-type matching, asset path resolution across directory structures, and HTTP wire formatting (`MakeHttpResponse`).
* **`main.cpp` (Main Dispatcher)**: Entry point and controller. Boots the audio backend (`WindowsMusicSystem` or `LinuxMusicSystem`), binds domain services (`DrumKit`, `InstrumentManager`, `DrumLoopEngine`, `ThreadPool`), and translates HTTP actions into audio engine events.

---

## 2. Sequence Diagrams

### Melodic Note Playback (`GET /play?inst=Harmonium&freq=261.63&vol=0.8&dur=1.2`)

```mermaid
sequenceDiagram
    autonumber
    actor User as Web Browser (UI)
    participant Server as HttpServer
    participant Main as Main (main.cpp)
    participant Req as HttpRequest
    participant Pool as ThreadPool
    participant Mgr as InstrumentManager
    participant Inst as IMusicInstrument (Harmonium)

    User->>Server: HTTP GET /play?inst=Harmonium&freq=261.63...
    Server->>Main: RequestHandler(rawBuffer, rawResponse)
    Main->>Req: Parse(rawBuffer)
    Req-->>Main: HttpRequest { method: "GET", path: "/play", query: "..." }
    
    Main->>Mgr: GetInstrument("Harmonium")
    Mgr-->>Main: shared_ptr<IMusicInstrument>
    
    Main->>Pool: Enqueue(PlayTask{ inst, freq, dur, vol })
    Note over Pool,Inst: Async execution offloads audio synthesis from the HTTP thread
    
    Main->>Main: HttpUtils::MakeHttpResponse("text/plain", "OK")
    Main-->>Server: return true (keepAlive) + formatted response
    Server->>User: 200 OK (Content-Type: text/plain)
    
    par Async Audio Thread
        Pool->>Inst: PlayNote(261.63, 1.2, 0.8)
    end

```

---

### Static Asset Delivery (`GET /styles.css`)

```mermaid
sequenceDiagram
    autonumber
    actor User as Web Browser (UI)
    participant Server as HttpServer
    participant Main as Main (main.cpp)
    participant Utils as HttpUtils
    participant FS as File System

    User->>Server: HTTP GET /styles.css
    Server->>Main: RequestHandler(rawBuffer, rawResponse)
    Main->>Utils: TryLoadAsset("styles.css", outContent)
    Utils->>FS: ResolveAssetPath() across prefixes (web/, ../web/, etc.)
    FS-->>Utils: File found & content read
    Utils-->>Main: true (content buffer populated)
    
    Main->>Utils: GetMimeType("styles.css")
    Utils-->>Main: "text/css"
    
    Main->>Utils: MakeHttpResponse("text/css", outContent)
    Utils-->>Main: "HTTP/1.1 200 OK\r\nContent-Type: text/css\r\n..."
    Main-->>Server: return true + rawResponse
    Server->>User: 200 OK (Embedded CSS)

```

---

### Graceful System Shutdown (`POST /shutdown`)

```mermaid
sequenceDiagram
    autonumber
    actor User as UI Exit Button
    participant Server as HttpServer
    participant Main as Main (main.cpp)
    participant Loop as DrumLoopEngine
    participant Audio as WindowsMusicSystem

    User->>Server: POST /shutdown (or sendBeacon)
    Server->>Main: RequestHandler(rawBuffer, rawResponse)
    Main->>Main: HttpUtils::MakeHttpResponse("text/plain", "EXIT")
    Main-->>Server: return false (Signal shutdown)
    Server->>User: 200 OK (EXIT)
    Server->>Server: Close client socket & break accept loop
    Server-->>Main: Run() exits
    
    Main->>Loop: Stop()
    Main->>Server: Stop() & Cleanup()
    Main->>Audio: Clear()
    Note over Main: Server and audio subsystems release system hardware cleanly

```

---

## 3. Component Architecture & Data Flow

```mermaid
flowchart LR
    subgraph Frontend [Browser Layer]
        HTML[index.html]
        CSS[styles.css]
        JS[app.js]
        I18N[locales.js]
    end

    subgraph Network [MusicPlayerTesterApp]
        SP[SocketPlatform]
        HS[HttpServer]
        HR[HttpRequest Parser]
        ROUTER{Route Matching}
        HU[HttpUtils]
    end

    subgraph Domain [MusicBuilderBL]
        TP[ThreadPool]
        DLE[DrumLoopEngine]
        IM[InstrumentManager]
    end

    subgraph Hardware [Audio Driver Layer]
        SYS[WindowsMusicSystem / LinuxMusicSystem]
    end

    Frontend -- HTTP Requests --> HS
    HS -- Platform Sockets --> SP
    HS --> HR
    HR --> ROUTER
    ROUTER -- Static File Req --> HU
    HU -- 200 OK Content --> HS
    ROUTER -- /play --> TP
    ROUTER -- /drumHit --> TP
    ROUTER -- /loop --> DLE
    ROUTER -- Lookup --> IM
    TP --> IM
    IM --> SYS
    DLE --> SYS
    HS -- HTTP Response --> Frontend

```