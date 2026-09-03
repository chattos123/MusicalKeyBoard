/**
 * @file DrumLoopEngine.h
 * @author Soumyajit C
 * @brief Provides a loop engine for managing drum patterns asynchronously.
 * @date 2026-09-03
 *
 * The DrumLoopEngine class manages background drum loops by running
 * a dedicated thread that plays drum patterns at a specified tempo (BPM).
 * It integrates with the DrumKit class to trigger drum hits in sequence.
 */

#pragma once

#include <memory>
#include <string>
#include <thread>
#include <atomic>
#include "DrumKit.h"

/**
 * @class DrumLoopEngine
 * @brief Engine for running drum loops asynchronously.
 *
 * Responsibilities:
 * - Start and stop drum loop playback.
 * - Run a background thread that interprets a pattern string and triggers drum hits.
 * - Maintain synchronization with the specified beats per minute (BPM).
 *
 * Usage:
 * - Construct with a shared pointer to a DrumKit.
 * - Call Start(pattern, bpm) to begin looping.
 * - Call Stop() to terminate the loop.
 */
class DrumLoopEngine 
{
private:
    std::shared_ptr<DrumKit> m_drums;       ///< Associated drum kit
    std::atomic<bool> m_running{ false };   ///< Loop running flag
    std::thread m_loopThread;               ///< Background loop thread

    /**
     * @brief Worker function that runs the drum loop.
     * @param pattern [in] Drum pattern string (e.g., "x---x---").
     * @param bpm [in] Tempo in beats per minute.
     *
     * This function executes in a separate thread, interpreting the pattern
     * and triggering drum hits at the correct timing.
     */
    void LoopWorker(std::string pattern, double bpm);

public:
    /**
     * @brief Constructs a DrumLoopEngine with the given drum kit.
     * @param drums [in] Shared pointer to a DrumKit instance.
     */
    explicit DrumLoopEngine(std::shared_ptr<DrumKit> drums);

    /**
     * @brief Destructor. Ensures the loop thread is stopped and cleaned up.
     */
    ~DrumLoopEngine();

    /**
     * @brief Starts the drum loop with the given pattern and tempo.
     * @param pattern [in] Drum pattern string.
     * @param bpm [in] Tempo in beats per minute.
     */
    void Start(const std::string& pattern, double bpm);

    /**
     * @brief Stops the drum loop and joins the background thread.
     */
    void Stop();
};