/**
 * @file WindowsMusicSystem.h
 * @author Soumyajit C
 * @brief Windows-specific implementation of IMusicSystem using WASAPI.
 * @date 2026-09-03
 *
 * This class provides a low-latency audio engine for Windows using the
 * Core Audio API (WASAPI). It supports real-time audio rendering and
 * asynchronous mixing of multiple voices.
 */

#pragma once
#include "IMusicSystem.h"
#include <mutex>
#include <thread>
#include <atomic>
#include <list>

// Forward declare COM structs/interfaces to avoid heavy includes in header
struct IMMDeviceEnumerator;
struct IMMDevice;
struct IAudioClient;
struct IAudioRenderClient;
typedef struct tWAVEFORMATEX WAVEFORMATEX;

/**
 * @class WindowsMusicSystem
 * @brief Windows implementation of IMusicSystem using WASAPI.
 *
 * Responsibilities:
 * - Initialize and configure WASAPI audio client.
 * - Manage audio buffers and render client.
 * - Provide sample rate information.
 * - Render audio samples and mix voices asynchronously.
 *
 * Internal details:
 * - Uses COM interfaces IMMDeviceEnumerator, IMMDevice, IAudioClient, IAudioRenderClient.
 * - Runs a dedicated mixer thread to combine active voices in real-time.
 */
class MPS_API WindowsMusicSystem : public IMusicSystem 
{
private:
#ifdef _WIN32
    // COM interface pointers
    IMMDeviceEnumerator* m_enumerator = nullptr;   ///< Device enumerator
    IMMDevice* m_device = nullptr;                 ///< Default audio device
    IAudioClient* m_audioClient = nullptr;         ///< WASAPI audio client
    IAudioRenderClient* m_renderClient = nullptr;  ///< WASAPI render client
    WAVEFORMATEX* m_pwfx = nullptr;                ///< Audio format
    unsigned int m_bufferFrameCount = 0;           ///< Buffer size in frames
    bool m_isFloat = false;                        ///< True if float format
    double m_sampleRate = 48000.0;                 ///< Sample rate in Hz
    bool m_isInitialized = false;                  ///< Initialization state

    /**
     * @struct ActiveVoice
     * @brief Represents an active audio stream being mixed.
     */
    struct ActiveVoice 
    {
        std::vector<float> samples; ///< Audio samples
        size_t cursor = 0;          ///< Current playback position
    };

    std::mutex m_mixerMutex;                  ///< Protects active voices list
    std::list<ActiveVoice> m_activeVoices;    ///< List of active voices
    std::atomic<bool> m_mixerRunning{ false };///< Mixer thread running flag
    std::thread m_mixerThread;                ///< Mixer worker thread

    /**
     * @brief Mixer worker thread function.
     * Continuously mixes active voices and writes to WASAPI buffer.
     */
    void MixerWorker();
#endif

public:
    /**
     * @brief Constructs a WindowsMusicSystem instance.
     */
    WindowsMusicSystem();

    /**
     * @brief Destructor. Cleans up audio resources.
     */
    ~WindowsMusicSystem() override;

    /**
     * @brief Initializes the WASAPI audio subsystem.
     * @return true if setup succeeded, false otherwise.
     */
    bool Setup() override;

    /**
     * @brief Clears and shuts down the audio subsystem.
     */
    void Clear() override;

    /**
     * @brief Retrieves the audio sample rate.
     * @return Sample rate in Hz.
     */
    double GetSampleRate() const override;

    /**
     * @brief Renders audio samples by submitting them to the mixer.
     * @param monoSamples [in] Vector of mono audio samples.
     * @return true if submission succeeded.
     */
    bool RenderAudio(const std::vector<float>& monoSamples) override;

    /**
     * @brief Mixes audio samples asynchronously in real-time.
     * @param monoSamples [in] Vector of mono audio samples.
     */
    void MixAudioAsync(const std::vector<float>& monoSamples) override;
};
