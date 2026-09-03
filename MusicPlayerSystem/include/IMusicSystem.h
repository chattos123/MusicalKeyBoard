/**
 * @file IMusicSystem.h
 * @author Soumyajit C
 * @brief Interface definition for the Music System abstraction layer.
 * @date 2026-09-03
 *
 * This interface defines the contract for audio system implementations
 * (e.g., Windows WASAPI, Linux ALSA/PulseAudio). It provides methods
 * for setup, cleanup, audio rendering, and asynchronous mixing.
 */

#pragma once
#include "MusicPlayerSystemExport.h"
#include <vector>

/**
 * @class IMusicSystem
 * @brief Abstract interface for platform-specific audio systems.
 *
 * Implementations of this interface provide:
 * - Initialization and cleanup of audio subsystems.
 * - Access to the audio sample rate.
 * - Rendering of audio samples to the output device.
 * - Non-blocking asynchronous audio mixing for real-time playback.
 *
 * This interface is exported via `MPS_API` for cross-platform usage.
 */
class MPS_API IMusicSystem 
{
public:
    /**
     * @brief Virtual destructor for safe cleanup of derived classes.
     */
    virtual ~IMusicSystem() = default;

    /**
     * @brief Initializes the audio subsystem.
     * @return true if setup succeeded, false otherwise.
     */
    virtual bool Setup() = 0;

    /**
     * @brief Clears and shuts down the audio subsystem.
     */
    virtual void Clear() = 0;

    /**
     * @brief Retrieves the audio sample rate.
     * @return Sample rate in Hz (e.g., 44100.0).
     */
    virtual double GetSampleRate() const = 0;

    /**
     * @brief Renders audio samples to the output device.
     * @param monoSamples [in] Vector of mono audio samples (float values).
     * @return true if rendering succeeded, false otherwise.
     */
    virtual bool RenderAudio(const std::vector<float>& monoSamples) = 0;

    /**
     * @brief Mixes audio samples asynchronously in real-time.
     * @param monoSamples [in] Vector of mono audio samples (float values).
     *
     * @remarks
     * - This function is non-blocking.
     * - Intended for real-time voice or instrument mixing.
     */
    virtual void MixAudioAsync(const std::vector<float>& monoSamples) = 0;
};
