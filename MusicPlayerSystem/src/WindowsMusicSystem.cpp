/**
 * @file WindowsMusicSystem.cpp
 * @author Soumyajit C
 * @brief Implementation of the Windows WASAPI low-latency audio rendering system.
 *        Implementation of the WindowsMusicSystem class using WASAPI for low-latency audio playback.
 * @details Manages COM lifecycle, initializes the default multimedia audio endpoint,
 *          configures low-latency WASAPI buffer periods in shared mode, mixes polyphonic
 *          dynamic voice streams, applies a soft saturation limiter (tanh), and feeds
 *          PCM audio data directly to the audio hardware client.
 */

#include "WindowsMusicSystem.h"

#include <algorithm>
#include <cmath>
#include <iostream>

#ifdef _WIN32
#include <audioclient.h>
#include <mmdeviceapi.h>
#include <windows.h>

namespace
{
    /// Conversion factor: 1 second represented in 100-nanosecond reference time units.
    constexpr REFERENCE_TIME kRefTimesPerSec = 10000000;

    /// Fallback device period: 10 milliseconds in 100-nanosecond reference time units.
    constexpr REFERENCE_TIME kFallbackDevicePeriodHns = 100000;

    /// 16-bit signed integer peak scalar value (short max amplitude).
    constexpr float kPcm16BitMaxScalar = 32767.0f;

    /// Fallback sleep duration in milliseconds when buffer queries stall.
    constexpr DWORD kBufferStallSleepMs = 2;

    /// COM Class and Interface Identifiers defined explicitly to avoid external GUID library dependencies.
    static const GUID s_clsidMmDeviceEnumerator =
        { 0xBCDE0395, 0xE52F, 0x467C, { 0x8E, 0x3D, 0xC4, 0x57, 0x92, 0x91, 0x69, 0x2E } };

    static const GUID s_iidImmDeviceEnumerator =
        { 0xA95664D2, 0x9614, 0x4F35, { 0xA7, 0x46, 0xDE, 0x8D, 0xB6, 0x36, 0x17, 0xE6 } };

    static const GUID s_iidIaudioClient =
        { 0x1CB9AD4C, 0xDBFA, 0x4C32, { 0xB1, 0x78, 0xC2, 0xF5, 0x68, 0xA7, 0x03, 0xB2 } };

    static const GUID s_iidIaudioRenderClient =
        { 0xF294ACFC, 0x3146, 0x4483, { 0xA7, 0xBF, 0xAD, 0xDC, 0xA7, 0xC2, 0x60, 0xE2 } };

    static const GUID s_ksDataFormatSubtypeIeeeFloat =
        { 0x00000003, 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71 } };
} // anonymous namespace
#endif

WindowsMusicSystem::WindowsMusicSystem() = default;

WindowsMusicSystem::~WindowsMusicSystem()
{
    Clear();
}

/**
 * @brief Initializes the COM runtime, WASAPI shared renderer, and mixer thread.
 * @details Resolves the default multimedia render device, queries optimal endpoint format,
 *          negotiates minimum shared buffer periods to minimize acoustic latency, primes the
 *          render buffer with silence to eliminate startup popping artifacts, and launches
 *          the continuous mixer worker thread.
 * @return True if initialized successfully or already active; false upon API failure.
 */
bool WindowsMusicSystem::Setup()
{
#ifdef _WIN32
    if (m_isInitialized)
    {
        return true;
    }

    // Initialize COM library on the calling thread for multithreaded object concurrency.
    HRESULT hResult = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hResult) && hResult != RPC_E_CHANGED_MODE)
    {
        std::cerr << "[WindowsMusicSystem] Failed to initialize COM library. HR: " 
                  << std::hex << hResult << '\n';
        return false;
    }

    // Instantiate MMDeviceEnumerator to discover system audio endpoints.
    hResult = CoCreateInstance(
        s_clsidMmDeviceEnumerator,
        nullptr,
        CLSCTX_ALL,
        s_iidImmDeviceEnumerator,
        reinterpret_cast<void**>(&m_enumerator));
    if (FAILED(hResult))
    {
        std::cerr << "[WindowsMusicSystem] Failed to create MMDeviceEnumerator.\n";
        return false;
    }

    // Query default output endpoint configured for multimedia/console playback.
    hResult = m_enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &m_device);
    if (FAILED(hResult))
    {
        std::cerr << "[WindowsMusicSystem] Failed to acquire default audio endpoint.\n";
        return false;
    }

    // Activate the core WASAPI client interface.
    hResult = m_device->Activate(
        s_iidIaudioClient,
        CLSCTX_ALL,
        nullptr,
        reinterpret_cast<void**>(&m_audioClient));
    if (FAILED(hResult))
    {
        std::cerr << "[WindowsMusicSystem] Failed to activate IAudioClient interface.\n";
        return false;
    }

    // Determine the native hardware mix format preferred by the audio engine.
    hResult = m_audioClient->GetMixFormat(&m_pwfx);
    if (FAILED(hResult))
    {
        std::cerr << "[WindowsMusicSystem] Failed to get device mix format.\n";
        return false;
    }

    m_sampleRate = static_cast<double>(m_pwfx->nSamplesPerSec);

    // 1. Query device periods to select the minimum hardware-supported buffer duration.
    REFERENCE_TIME defaultDevicePeriodHns = 0;
    REFERENCE_TIME minimumDevicePeriodHns = 0;

    hResult = m_audioClient->GetDevicePeriod(&defaultDevicePeriodHns, &minimumDevicePeriodHns);
    if (FAILED(hResult) || minimumDevicePeriodHns == 0)
    {
        minimumDevicePeriodHns = kFallbackDevicePeriodHns;
    }

    // 2. Initialize WASAPI stream in shared mode using low-latency period.
    hResult = m_audioClient->Initialize(
        AUDCLNT_SHAREMODE_SHARED,
        AUDCLNT_STREAMFLAGS_NOPERSIST,
        minimumDevicePeriodHns,
        0,
        m_pwfx,
        nullptr);
    if (FAILED(hResult))
    {
        std::cerr << "[WindowsMusicSystem] Failed to initialize IAudioClient in shared mode.\n";
        return false;
    }

    hResult = m_audioClient->GetBufferSize(&m_bufferFrameCount);
    if (FAILED(hResult))
    {
        std::cerr << "[WindowsMusicSystem] Failed to get buffer frame count.\n";
        return false;
    }

    // Acquire the render client service responsible for writing PCM samples.
    hResult = m_audioClient->GetService(
        s_iidIaudioRenderClient,
        reinterpret_cast<void**>(&m_renderClient));
    if (FAILED(hResult))
    {
        std::cerr << "[WindowsMusicSystem] Failed to acquire IAudioRenderClient service.\n";
        return false;
    }

    // 3. Pre-fill the initial audio buffer with silence to prevent startup click/pop artifacts.
    BYTE* pInitialBuffer = nullptr;
    hResult = m_renderClient->GetBuffer(m_bufferFrameCount, &pInitialBuffer);
    if (SUCCEEDED(hResult))
    {
        m_renderClient->ReleaseBuffer(m_bufferFrameCount, AUDCLNT_BUFFERFLAGS_SILENT);
    }

    // Detect format representation: IEEE 32-bit floating point vs. Signed 16-bit integer PCM.
    m_isFloat = (m_pwfx->wFormatTag == WAVE_FORMAT_IEEE_FLOAT) ||
                (m_pwfx->wFormatTag == WAVE_FORMAT_EXTENSIBLE &&
                 reinterpret_cast<WAVEFORMATEXTENSIBLE*>(m_pwfx)->SubFormat == s_ksDataFormatSubtypeIeeeFloat);

    // Start playback clock on the device endpoint.
    hResult = m_audioClient->Start();
    if (FAILED(hResult))
    {
        std::cerr << "[WindowsMusicSystem] Failed to start IAudioClient playback.\n";
        return false;
    }

    m_isInitialized = true;
    m_mixerRunning = true;
    m_mixerThread = std::thread(&WindowsMusicSystem::MixerWorker, this);

    std::cout << "[WindowsMusicSystem] Low-Latency WASAPI Engine Running ("
              << m_sampleRate << " Hz, Buffer: " << m_bufferFrameCount << " frames)\n";
    return true;
#else
    return true;
#endif
}

/**
 * @brief Halts audio processing, stops background threads, and releases all COM resources.
 * @details Guarantees synchronous termination of the mixer worker thread before teardown,
 *          stops the active audio stream, frees allocated wave format descriptors, and uninitializes COM.
 */
void WindowsMusicSystem::Clear()
{
#ifdef _WIN32
    if (!m_isInitialized)
    {
        return;
    }

    // Signal mixer worker thread to exit and wait for graceful termination.
    m_mixerRunning = false;
    if (m_mixerThread.joinable())
    {
        m_mixerThread.join();
    }

    if (m_audioClient)
    {
        m_audioClient->Stop();
    }

    if (m_pwfx)
    {
        CoTaskMemFree(m_pwfx);
        m_pwfx = nullptr;
    }

    if (m_renderClient)
    {
        m_renderClient->Release();
        m_renderClient = nullptr;
    }

    if (m_audioClient)
    {
        m_audioClient->Release();
        m_audioClient = nullptr;
    }

    if (m_device)
    {
        m_device->Release();
        m_device = nullptr;
    }

    if (m_enumerator)
    {
        m_enumerator->Release();
        m_enumerator = nullptr;
    }

    CoUninitialize();
    m_isInitialized = false;

    std::cout << "[WindowsMusicSystem] COM Mixer Engine Stopped.\n";
#endif
}

/**
 * @brief Retrieves the active output sampling frequency.
 * @return Sample rate in Hertz (typically 44100.0 or 48000.0).
 */
double WindowsMusicSystem::GetSampleRate() const
{
#ifdef _WIN32
    return m_sampleRate;
#else
    return 48000.0;
#endif
}

/**
 * @brief Enqueues a synthesized mono audio stream for asynchronous background mixing.
 * @param monoSamples Vector of floating-point audio samples normalized in [-1.0, 1.0].
 * @note Thread-safe. Acquires m_mixerMutex to safely append to the active voice list.
 */
void WindowsMusicSystem::MixAudioAsync(const std::vector<float>& monoSamples)
{
#ifdef _WIN32
    if (monoSamples.empty())
    {
        return;
    }

    std::lock_guard<std::mutex> lock(m_mixerMutex);
    m_activeVoices.push_back({ monoSamples, 0 });
#endif
}

/**
 * @brief Unified audio render interface dispatching to the asynchronous voice mixer.
 * @param monoSamples Vector of floating-point audio samples to render.
 * @return True when successfully enqueued.
 */
bool WindowsMusicSystem::RenderAudio(const std::vector<float>& monoSamples)
{
    MixAudioAsync(monoSamples);
    return true;
}

#ifdef _WIN32
/**
 * @brief Real-time background audio mixing and device streaming loop.
 * @details Continuously checks WASAPI buffer padding, calculates available frame capacity,
 *          locks and sums all concurrent playing voices, applies hyperbolic tangent (tanh)
 *          soft-clipping to eliminate digital overflow, converts sample formats (IEEE Float / Int16),
 *          duplicates mono channels across the hardware channel topology, and throttles loop
 *          cadence via adaptive thread sleep.
 */
void WindowsMusicSystem::MixerWorker()
{
    while (m_mixerRunning)
    {
        UINT32 unrenderedFramePadding = 0;
        if (FAILED(m_audioClient->GetCurrentPadding(&unrenderedFramePadding)))
        {
            Sleep(5);
            continue;
        }

        const UINT32 framesAvailable = m_bufferFrameCount - unrenderedFramePadding;
        if (framesAvailable == 0)
        {
            Sleep(kBufferStallSleepMs);
            continue;
        }

        BYTE* pBufferData = nullptr;
        if (FAILED(m_renderClient->GetBuffer(framesAvailable, &pBufferData)))
        {
            Sleep(kBufferStallSleepMs);
            continue;
        }

        auto* pFloatBuffer = reinterpret_cast<float*>(pBufferData);
        auto* pInt16Buffer = reinterpret_cast<short*>(pBufferData);
        const int channelCount = m_pwfx->nChannels;

        // Note: Lock is held during the frame slice write to maintain sample-synchronous voice state.
        {
            std::lock_guard<std::mutex> lock(m_mixerMutex);

            for (UINT32 frameIndex = 0; frameIndex < framesAvailable; ++frameIndex)
            {
                float mixedSample = 0.0f;

                // Accumulate active voice channels and cull finished tracks
                for (auto voiceIt = m_activeVoices.begin(); voiceIt != m_activeVoices.end(); )
                {
                    if (voiceIt->cursor < voiceIt->samples.size())
                    {
                        mixedSample += voiceIt->samples[voiceIt->cursor++];
                        ++voiceIt;
                    }
                    else
                    {
                        voiceIt = m_activeVoices.erase(voiceIt);
                    }
                }

                // Soft-saturation curve (tanh): Prevents harsh digital clipping when multiple notes sound simultaneously.
                mixedSample = std::tanh(mixedSample);
                const short pcmInt16Sample = static_cast<short>(mixedSample * kPcm16BitMaxScalar);

                // Duplicate mono channel into all interleaved output hardware channels (Stereo/Surround)
                const UINT32 baseChannelOffset = frameIndex * channelCount;
                for (int channelIndex = 0; channelIndex < channelCount; ++channelIndex)
                {
                    if (m_isFloat)
                    {
                        pFloatBuffer[baseChannelOffset + channelIndex] = mixedSample;
                    }
                    else
                    {
                        pInt16Buffer[baseChannelOffset + channelIndex] = pcmInt16Sample;
                    }
                }
            }
        }

        m_renderClient->ReleaseBuffer(framesAvailable, 0);

        // Sleep for approximately half the time it takes the hardware to consume the rendered slice.
        const auto sleepDurationMs = static_cast<DWORD>((1000.0 * framesAvailable) / m_sampleRate / 2.0);
        Sleep(std::max<DWORD>(1, sleepDurationMs));
    }
}
#endif