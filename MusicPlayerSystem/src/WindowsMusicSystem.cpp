#include "WindowsMusicSystem.h"
#include <iostream>
#include <algorithm>
#include <cmath>

#ifdef _WIN32
#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>

#define REFTIMES_PER_SEC 10000000

static const GUID CLSID_MMDeviceEnumerator_Val = 
    { 0xBCDE0395, 0xE52F, 0x467C, { 0x8E, 0x3D, 0xC4, 0x57, 0x92, 0x91, 0x69, 0x2E } };

static const GUID IID_IMMDeviceEnumerator_Val = 
    { 0xA95664D2, 0x9614, 0x4F35, { 0xA7, 0x46, 0xDE, 0x8D, 0xB6, 0x36, 0x17, 0xE6 } };

static const GUID IID_IAudioClient_Val = 
    { 0x1CB9AD4C, 0xDBFA, 0x4C32, { 0xB1, 0x78, 0xC2, 0xF5, 0x68, 0xA7, 0x03, 0xB2 } };

static const GUID IID_IAudioRenderClient_Val = 
    { 0xF294ACFC, 0x3146, 0x4483, { 0xA7, 0xBF, 0xAD, 0xDC, 0xA7, 0xC2, 0x60, 0xE2 } };

static const GUID KSDATAFORMAT_SUBTYPE_IEEE_FLOAT_Val = 
    { 0x00000003, 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71 } };
#endif

WindowsMusicSystem::WindowsMusicSystem() = default;

WindowsMusicSystem::~WindowsMusicSystem() {
    Clear();
}

bool WindowsMusicSystem::Setup() 
{
#ifdef _WIN32
    if (m_isInitialized) return true;

    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr) && hr != RPC_E_CHANGED_MODE) return false;

    hr = CoCreateInstance(CLSID_MMDeviceEnumerator_Val, NULL, CLSCTX_ALL,
        IID_IMMDeviceEnumerator_Val, (void**)&m_enumerator);
    if (FAILED(hr)) return false;

    hr = m_enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &m_device);
    if (FAILED(hr)) return false;

    hr = m_device->Activate(IID_IAudioClient_Val, CLSCTX_ALL, NULL, (void**)&m_audioClient);
    if (FAILED(hr)) return false;

    hr = m_audioClient->GetMixFormat(&m_pwfx);
    if (FAILED(hr)) return false;

    m_sampleRate = static_cast<double>(m_pwfx->nSamplesPerSec);

    // 1. Query device periods: minimum low-latency buffer (typically 3-10ms)
    REFERENCE_TIME hnsDefaultDevicePeriod = 0;
    REFERENCE_TIME hnsMinimumDevicePeriod = 0;

    hr = m_audioClient->GetDevicePeriod(&hnsDefaultDevicePeriod, &hnsMinimumDevicePeriod);
    if (FAILED(hr) || hnsMinimumDevicePeriod == 0) 
    {
        hnsMinimumDevicePeriod = 100000; // Fallback: 10ms (in 100-ns units)
    }

    // 2. Initialize WASAPI with low-latency buffer size (10ms)
    hr = m_audioClient->Initialize(
        AUDCLNT_SHAREMODE_SHARED,
        AUDCLNT_STREAMFLAGS_NOPERSIST,
        hnsMinimumDevicePeriod,
        0,
        m_pwfx,
        NULL
    );
    if (FAILED(hr)) return false;

    hr = m_audioClient->GetBufferSize(&m_bufferFrameCount);
    if (FAILED(hr)) return false;

    hr = m_audioClient->GetService(IID_IAudioRenderClient_Val, (void**)&m_renderClient);
    if (FAILED(hr)) return false;

    // 3. Pre-fill the initial buffer with silence to prevent startup glitches
    BYTE* pData = nullptr;
    hr = m_renderClient->GetBuffer(m_bufferFrameCount, &pData);
    if (SUCCEEDED(hr)) {
        m_renderClient->ReleaseBuffer(m_bufferFrameCount, AUDCLNT_BUFFERFLAGS_SILENT);
    }

    m_isFloat = (m_pwfx->wFormatTag == WAVE_FORMAT_IEEE_FLOAT) ||
        (m_pwfx->wFormatTag == WAVE_FORMAT_EXTENSIBLE &&
            reinterpret_cast<WAVEFORMATEXTENSIBLE*>(m_pwfx)->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT_Val);

    hr = m_audioClient->Start();
    if (FAILED(hr)) return false;

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

void WindowsMusicSystem::Clear() {
#ifdef _WIN32
    if (!m_isInitialized) return;

    m_mixerRunning = false;
    if (m_mixerThread.joinable()) {
        m_mixerThread.join();
    }

    if (m_audioClient) m_audioClient->Stop();
    if (m_pwfx) { CoTaskMemFree(m_pwfx); m_pwfx = nullptr; }
    if (m_renderClient) { m_renderClient->Release(); m_renderClient = nullptr; }
    if (m_audioClient) { m_audioClient->Release(); m_audioClient = nullptr; }
    if (m_device) { m_device->Release(); m_device = nullptr; }
    if (m_enumerator) { m_enumerator->Release(); m_enumerator = nullptr; }
    CoUninitialize();
    m_isInitialized = false;
    std::cout << "[WindowsMusicSystem] COM Mixer Engine Stopped.\n";
#endif
}

double WindowsMusicSystem::GetSampleRate() const {
#ifdef _WIN32
    return m_sampleRate;
#else
    return 48000.0;
#endif
}

// Thread-safe voice submission
void WindowsMusicSystem::MixAudioAsync(const std::vector<float>& monoSamples) {
#ifdef _WIN32
    if (monoSamples.empty()) return;
    std::lock_guard<std::mutex> lock(m_mixerMutex);
    m_activeVoices.push_back({ monoSamples, 0 });
#endif
}

bool WindowsMusicSystem::RenderAudio(const std::vector<float>& monoSamples) {
    MixAudioAsync(monoSamples);
    return true;
}

#ifdef _WIN32
void WindowsMusicSystem::MixerWorker() {
    while (m_mixerRunning) {
        UINT32 padding = 0;
        if (FAILED(m_audioClient->GetCurrentPadding(&padding))) {
            Sleep(5);
            continue;
        }

        UINT32 framesAvailable = m_bufferFrameCount - padding;
        if (framesAvailable == 0) {
            Sleep(2);
            continue;
        }

        BYTE* pData = nullptr;
        if (FAILED(m_renderClient->GetBuffer(framesAvailable, &pData))) {
            Sleep(2);
            continue;
        }

        // Mix all active voice tracks into this buffer slice
        for (UINT32 i = 0; i < framesAvailable; ++i) {
            float mixedSample = 0.0f;

            {
                std::lock_guard<std::mutex> lock(m_mixerMutex);
                for (auto it = m_activeVoices.begin(); it != m_activeVoices.end(); ) {
                    if (it->cursor < it->samples.size()) {
                        mixedSample += it->samples[it->cursor];
                        it->cursor++;
                        ++it;
                    } else {
                        it = m_activeVoices.erase(it);
                    }
                }
            }

            // Soft saturation limiter to prevent digital clipping when adding multiple notes + drums
            mixedSample = std::tanh(mixedSample);
            short sample16 = static_cast<short>(mixedSample * 32767.0f);

            for (int ch = 0; ch < m_pwfx->nChannels; ++ch) {
                if (m_isFloat) {
                    ((float*)pData)[i * m_pwfx->nChannels + ch] = mixedSample;
                } else {
                    ((short*)pData)[i * m_pwfx->nChannels + ch] = sample16;
                }
            }
        }

        m_renderClient->ReleaseBuffer(framesAvailable, 0);
        Sleep(static_cast<DWORD>(1000.0 * framesAvailable / m_sampleRate / 2));
    }
}
#endif