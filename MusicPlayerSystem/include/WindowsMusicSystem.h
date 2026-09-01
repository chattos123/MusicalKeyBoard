#pragma once
#include "IMusicSystem.h"
#include <mutex>
#include <thread>
#include <atomic>
#include <list>

// Forward declare COM structs/interfaces
struct IMMDeviceEnumerator;
struct IMMDevice;
struct IAudioClient;
struct IAudioRenderClient;
typedef struct tWAVEFORMATEX WAVEFORMATEX;

class MPS_API WindowsMusicSystem : public IMusicSystem 
{
private:
#ifdef _WIN32
    IMMDeviceEnumerator* m_enumerator = nullptr;
    IMMDevice* m_device = nullptr;
    IAudioClient* m_audioClient = nullptr;
    IAudioRenderClient* m_renderClient = nullptr;
    WAVEFORMATEX* m_pwfx = nullptr;
    unsigned int m_bufferFrameCount = 0;
    bool m_isFloat = false;
    double m_sampleRate = 48000.0;
    bool m_isInitialized = false;

     // Real-time Mixer State
    struct ActiveVoice 
    {
        std::vector<float> samples;
        size_t cursor = 0;
    };

    std::mutex m_mixerMutex;
    std::list<ActiveVoice> m_activeVoices;
    std::atomic<bool> m_mixerRunning{ false };
    std::thread m_mixerThread;

    void MixerWorker();
#endif

public:
    WindowsMusicSystem();
    ~WindowsMusicSystem() override;

    bool Setup() override;
    void Clear() override;
    double GetSampleRate() const override;
    bool RenderAudio(const std::vector<float>& monoSamples) override;
    void MixAudioAsync(const std::vector<float>& monoSamples) override;
};