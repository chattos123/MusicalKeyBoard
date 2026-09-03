// import { I18N, SWARA_SCRIPTS } from './locales.js';

// // Scale interval definitions
// const SCALE_INTERVALS = {
//     "Bilawal":          [0, 2, 4, 5, 7, 9, 11],
//     "Kafi":             [0, 2, 3, 5, 7, 9, 10],
//     "Bhairav":          [0, 1, 4, 5, 7, 8, 11],
//     "Yaman":            [0, 2, 4, 6, 7, 9, 11],
//     "Asavari":          [0, 2, 3, 5, 7, 8, 10],
//     "Khamaj":           [0, 2, 4, 5, 7, 9, 10],
//     "Bhairavi":         [0, 1, 3, 5, 7, 8, 10],
//     "Todi":             [0, 1, 3, 6, 7, 8, 11],
//     "Chromatic":        [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11],
//     "PentatonicMajor":  [0, 2, 4, 7, 9],
//     "PentatonicMinor":  [0, 3, 5, 7, 10],
//     "Blues":            [0, 3, 5, 6, 7, 10]
// };

// const NOTE_NAMES = ["C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"];
// const BLACK_KEY_PITCHES = [1, 3, 6, 8, 10];

// const QWERTY_OFFSETS = [
//     { offset: 0,  key: "a" },
//     { offset: 1,  key: "w" },
//     { offset: 2,  key: "s" },
//     { offset: 3,  key: "e" },
//     { offset: 4,  key: "d" },
//     { offset: 5,  key: "f" },
//     { offset: 6,  key: "t" },
//     { offset: 7,  key: "g" },
//     { offset: 8,  key: "y" },
//     { offset: 9,  key: "h" },
//     { offset: 10, key: "u" },
//     { offset: 11, key: "j" },
//     { offset: 12, key: "k" },
//     { offset: 13, key: "o" },
//     { offset: 14, key: "l" },
//     { offset: 15, key: "p" },
//     { offset: 16, key: ";" }
// ];

// // Equalizer Presets
// const EQ_PRESETS = {
//     "Flat":      { 60: 0,  250: 0,  1000: 0,  4000: 0,  12000: 0 },
//     "Rock":      { 60: 5,  250: 2,  1000: -1, 4000: 3,  12000: 6 },
//     "Disco":     { 60: 6,  250: 3,  1000: 0,  4000: 2,  12000: 5 },
//     "Indian":    { 60: 2,  250: 4,  1000: 1,  4000: 5,  12000: 3 },
//     "BassBoost": { 60: 8,  250: 5,  1000: 0,  4000: -2, 12000: -4 },
//     "Vocal":     { 60: -3, 250: 1,  1000: 5,  4000: 4,  12000: 1 }
// };

// // DOM references
// const keyboard = document.getElementById("keyboard");
// const instSelect = document.getElementById("melodicInstrument");
// const notationModeSelect = document.getElementById("notationMode");
// const rootKeySelect = document.getElementById("rootKey");
// const scaleModeSelect = document.getElementById("scaleMode");
// const qwertyOctaveSelect = document.getElementById("qwertyOctave");
// const volSlider = document.getElementById("vol");
// const loopBtn = document.getElementById("toggleLoopBtn");
// const loopPattern = document.getElementById("loopPattern");
// const bpmInput = document.getElementById("bpm");
// const localeSelect = document.getElementById("localeSelect");
// const settingsModal = document.getElementById("settingsModal");
// const showQwertyCheck = document.getElementById("showQwertyCheck");
// const enableVisualizerCheck = document.getElementById("enableVisualizerCheck");
// const eqPresetSelect = document.getElementById("eqPreset");

// let isLooping = false;
// let isMouseDown = false;
// let showQwertyLabels = true;
// const activeKeyElements = new Set();
// let keyElements = [];

// // Equalizer state (10 bins tailored for the 80x88px canvas)
// const eqGains = { 60: 0, 250: 0, 1000: 0, 4000: 0, 12000: 0 };
// const canvas = document.getElementById("eqCanvas");
// const ctx = canvas.getContext("2d");
// const spectrumBins = new Array(10).fill(2);

// // =============================================================================
// // Settings Modal & Preferences
// // =============================================================================

// export function openSettingsModal() {
//     settingsModal.classList.add("open");
// }

// export function closeSettingsModal() {
//     settingsModal.classList.remove("open");
//     saveUserSettings();
// }

// export function toggleQwertyOverlay(show) {
//     showQwertyLabels = show;
//     rebuildKeyboard();
// }

// function saveUserSettings() {
//     const settings = {
//         locale: localeSelect.value,
//         showQwerty: showQwertyCheck.checked,
//         enableVisualizer: enableVisualizerCheck.checked
//     };
//     localStorage.setItem("user_synthesizer_settings", JSON.stringify(settings));
// }

// function loadUserSettings() {
//     const raw = localStorage.getItem("user_synthesizer_settings");
//     let settings = { locale: "en-US", showQwerty: true, enableVisualizer: true };
//     if (raw) {
//         try { settings = Object.assign(settings, JSON.parse(raw)); } catch(e){}
//     }
//     localeSelect.value = settings.locale || "en-US";
//     showQwertyCheck.checked = settings.showQwerty;
//     enableVisualizerCheck.checked = settings.enableVisualizer;
//     showQwertyLabels = settings.showQwerty;
//     applyLocale(settings.locale || "en-US");
// }

// // =============================================================================
// // I18N Locale Applicator
// // =============================================================================

// function applyLocale(locale) {
//     const strings = I18N[locale] || I18N["en-US"];
//     document.getElementById("lblAppTitle").innerText = strings.appTitle;
//     document.getElementById("lblEqHeader").innerText = strings.eqHeader;
//     document.getElementById("lblRhythmHeader").innerText = strings.rhythmHeader;
//     document.getElementById("lblKeyboardHeader").innerText = strings.keyboardHeader;
//     document.getElementById("modalTitle").innerText = strings.modalTitle;
//     document.getElementById("setLocaleTitle").innerText = strings.setLocaleTitle;
//     document.getElementById("setLocaleSub").innerText = strings.setLocaleSub;
//     document.getElementById("setKeybTitle").innerText = strings.setKeybTitle;
//     document.getElementById("setKeybSub").innerText = strings.setKeybSub;
//     document.getElementById("setEqualizerTitle").innerText = strings.setEqualizerTitle;
//     document.getElementById("setEqualizerSub").innerText = strings.setEqualizerSub;
//     document.getElementById("btnSaveClose").innerText = strings.saveClose;
//     document.getElementById("lblGroove").innerText = strings.groove;
//     document.getElementById("lblBpm").innerText = strings.bpm;
//     document.getElementById("lblMasterVol").innerText = strings.volume;
//     document.getElementById("lblInstrument").innerText = strings.instrument;
//     document.getElementById("lblNotation").innerText = strings.notation;
//     document.getElementById("lblOctave").innerText = strings.qwertyOctave;
//     document.getElementById("optRock").innerText = strings.rock;
//     document.getElementById("optFunk").innerText = strings.funk;
//     document.getElementById("optMetronome").innerText = strings.metronome;
//     document.getElementById("pad0").innerText = strings.pad0;
//     document.getElementById("pad1").innerText = strings.pad1;
//     document.getElementById("pad2").innerText = strings.pad2;
//     document.getElementById("pad6").innerText = strings.pad6;
//     document.getElementById("shutdownBtn").innerText = strings.quit;

//     // Equalizer Preset Dropdown Translations
//     const lblEqPreset = document.getElementById("lblEqPreset");
//     if (lblEqPreset && strings.lblEqPreset) lblEqPreset.innerText = strings.lblEqPreset;
//     if (document.getElementById("optPresetFlat"))   document.getElementById("optPresetFlat").innerText   = strings.presetFlat;
//     if (document.getElementById("optPresetRock"))   document.getElementById("optPresetRock").innerText   = strings.presetRock;
//     if (document.getElementById("optPresetDisco"))  document.getElementById("optPresetDisco").innerText  = strings.presetDisco;
//     if (document.getElementById("optPresetIndian")) document.getElementById("optPresetIndian").innerText = strings.presetIndian;
//     if (document.getElementById("optPresetBass"))   document.getElementById("optPresetBass").innerText   = strings.presetBass;
//     if (document.getElementById("optPresetVocal"))  document.getElementById("optPresetVocal").innerText  = strings.presetVocal;
//     if (document.getElementById("optPresetCustom")) document.getElementById("optPresetCustom").innerText = strings.presetCustom;

//     loopBtn.innerText = isLooping ? strings.stopLoop : strings.startLoop;

//     const isSargam = (notationModeSelect.value === "Sargam");
//     document.getElementById("rootLabel").innerText = isSargam ? strings.saRoot : strings.westernRoot;
//     document.getElementById("scaleLabel").innerText = isSargam ? strings.scaleThaat : strings.scaleWestern;

//     rebuildKeyboard();
// }

// // =============================================================================
// // Equalizer & Spectrum Visualizer (Classic Media Player Style)
// // =============================================================================

// function setBandValue(freq, val) {
//     eqGains[freq] = parseFloat(val);
//     const sliderMap = { 60: "eq60", 250: "eq250", 1000: "eq1k", 4000: "eq4k", 12000: "eq12k" };
//     const labelMap  = { 60: "val60", 250: "val250", 1000: "val1k", 4000: "val4k", 12000: "val12k" };

//     const slider = document.getElementById(sliderMap[freq]);
//     if (slider) slider.value = val;

//     const label = document.getElementById(labelMap[freq]);
//     if (label) label.innerText = (val > 0 ? "+" : "") + val + "dB";
// }

// export function updateEq(freq, val) {
//     setBandValue(freq, val);
//     if (eqPresetSelect) {
//         eqPresetSelect.value = "Custom";
//     }
// }

// export function applyEqPreset(presetName) {
//     const preset = EQ_PRESETS[presetName];
//     if (!preset) return;

//     for (const [freq, gain] of Object.entries(preset)) {
//         setBandValue(parseInt(freq, 10), gain);
//     }
// }

// function stimulateSpectrum(freqRatio) {
//     if (!enableVisualizerCheck.checked) return;
//     const targetBin = Math.min(spectrumBins.length - 1, Math.floor(freqRatio * spectrumBins.length));
//     spectrumBins[targetBin] = Math.min(48, spectrumBins[targetBin] + 40);
//     if (targetBin > 0) spectrumBins[targetBin - 1] += 18;
//     if (targetBin < spectrumBins.length - 1) spectrumBins[targetBin + 1] += 18;
// }

// function drawSpectrum() {
//     ctx.fillStyle = "#040608";
//     ctx.fillRect(0, 0, canvas.width, canvas.height);

//     const numSegments = 16;
//     const segmentHeight = 3.5;
//     const segmentSpacing = 1.5;
//     const barWidth = Math.floor((canvas.width - (spectrumBins.length * 2)) / spectrumBins.length);

//     for (let i = 0; i < spectrumBins.length; ++i) {
//         spectrumBins[i] = Math.max(2, spectrumBins[i] * 0.90);

//         const activeSegments = Math.floor((spectrumBins[i] / 50.0) * numSegments);
//         const x = i * (barWidth + 2) + 2;

//         for (let s = 0; s < numSegments; ++s) {
//             const y = canvas.height - ((s + 1) * (segmentHeight + segmentSpacing));

//             if (s < activeSegments) {
//                 if (s >= 13) {
//                     ctx.fillStyle = "#ff2a2a";
//                 } else if (s >= 9) {
//                     ctx.fillStyle = "#ffb000";
//                 } else {
//                     ctx.fillStyle = "#00e676";
//                 }
//             } else {
//                 ctx.fillStyle = "rgba(255, 255, 255, 0.035)";
//             }
//             ctx.fillRect(x, y, barWidth, segmentHeight);
//         }
//     }
//     requestAnimationFrame(drawSpectrum);
// }
// drawSpectrum();

// // =============================================================================
// // Audio Dispatch & Keyboard Core
// // =============================================================================

// function midiToFreq(midi) {
//     return 440.0 * Math.pow(2.0, (midi - 69.0) / 12.0);
// }

// export function sendPlay(freq) {
//     const inst = instSelect.value;
//     const vol = volSlider.value;
//     stimulateSpectrum(Math.min(1.0, freq / 2000.0));
//     fetch(`/play?inst=${inst}&freq=${freq}&vol=${vol}&dur=1.2`, {
//         method: "GET",
//         keepalive: true,
//         cache: "no-store",
//         priority: "high"
//     }).catch(() => {});
// }

// export function sendDrumHit(pieceId) {
//     stimulateSpectrum(pieceId === 0 ? 0.08 : 0.45);
//     fetch(`/drumHit?piece=${pieceId}`, {
//         method: "GET",
//         keepalive: true,
//         cache: "no-store",
//         priority: "high"
//     }).catch(() => {});
// }

// function clearAllActiveKeys() {
//     activeKeyElements.forEach(el => el.classList.remove("active"));
//     activeKeyElements.clear();
// }

// function activateKeyElement(el, freq) {
//     if (!el || activeKeyElements.has(el)) return;
//     el.classList.add("active");
//     activeKeyElements.add(el);
//     sendPlay(freq);
// }

// function deactivateKeyElement(el) {
//     if (!el) return;
//     el.classList.remove("active");
//     activeKeyElements.delete(el);
// }

// function rebuildKeyboard() {
//     clearAllActiveKeys();
//     keyboard.innerHTML = "";
//     keyElements = [];

//     const currentLocale = localeSelect.value || "en-US";
//     const swaraList = SWARA_SCRIPTS[currentLocale] || SWARA_SCRIPTS["en"];

//     const isSargam = (notationModeSelect.value === "Sargam");
//     const rootOffset = parseInt(rootKeySelect.value, 10);
//     const scaleIntervals = SCALE_INTERVALS[scaleModeSelect.value] || SCALE_INTERVALS["Bilawal"];
//     const qwertyBase = parseInt(qwertyOctaveSelect.value, 10);

//     let whiteOffset = 0;
//     const whiteKeyWidth = 32;

//     for (let midi = 36; midi <= 96; ++midi) {
//         const pitchClass = (midi % 12);
//         const isBlack = BLACK_KEY_PITCHES.includes(pitchClass);
//         const freq = midiToFreq(midi);

//         const swaraDegree = (pitchClass - rootOffset + 12) % 12;
//         const inScale = scaleIntervals.includes(swaraDegree);
//         const westernName = NOTE_NAMES[pitchClass] + Math.floor(midi / 12 - 1);

//         let swaraText = swaraList[swaraDegree];
//         if (midi >= 72) {
//             swaraText += "̇";
//         } else if (midi < 60) {
//             swaraText += "̣";
//         }

//         const qwertyItem = QWERTY_OFFSETS.find(q => (qwertyBase + q.offset) === midi);
//         const qwertyKey = (qwertyItem && showQwertyLabels) ? qwertyItem.key : "";

//         const el = document.createElement("div");
//         el.dataset.freq = freq.toFixed(2);
//         el.dataset.midi = midi;

//         const modeClass = isSargam ? "sargam-mode" : "";
//         const qwertyClass = qwertyKey ? "qwerty-mapped" : "";

//         if (!isBlack) {
//             el.className = `white-key ${modeClass} ${qwertyClass} ${inScale ? 'in-scale' : 'out-scale'}`;
//             const topLabel = qwertyKey ? qwertyKey.toUpperCase() : "";
//             if (isSargam) {
//                 el.innerHTML = `<span>${topLabel}</span><span class="swara-label">${swaraText}</span><span class="note-label">${westernName}</span>`;
//             } else {
//                 el.innerHTML = `<span>${topLabel}</span><span class="note-label">${westernName}</span>`;
//             }
//             keyboard.appendChild(el);
//             whiteOffset += whiteKeyWidth;
//         } else {
//             el.className = `black-key ${modeClass} ${qwertyClass} ${inScale ? 'in-scale' : 'out-scale'}`;
//             el.style.left = (whiteOffset - 10) + "px";
//             const topLabel = qwertyKey ? qwertyKey.toUpperCase() : "";
//             if (isSargam) {
//                 el.innerHTML = `<span>${topLabel}</span><span class="swara-label">${swaraText}</span>`;
//             } else {
//                 el.innerHTML = `<span>${topLabel}</span>`;
//             }
//             keyboard.appendChild(el);
//         }

//         el.addEventListener("mousedown", (e) => {
//             e.preventDefault();
//             activateKeyElement(el, freq);
//         });

//         el.addEventListener("mouseenter", () => {
//             if (isMouseDown) activateKeyElement(el, freq);
//         });

//         el.addEventListener("mouseleave", () => deactivateKeyElement(el));
//         el.addEventListener("mouseup", () => deactivateKeyElement(el));

//         keyElements.push({ el, freq, midi, qwertyKey });
//     }
// }

// export function closeApplication() {
//     if (confirm("Are you sure you want to stop the music engine and exit?")) {
//         navigator.sendBeacon("/shutdown");
//         document.body.innerHTML = `
//             <div style="text-align:center; margin-top:20vh;">
//                 <h2 style="color:#28a745;">Application Terminated</h2>
//                 <p style="color:#aaa;">Audio subsystem and backend server have exited cleanly. You can now close this tab.</p>
//             </div>`;
//         setTimeout(() => window.close(), 1500);
//     }
// }

// // =============================================================================
// // Event Listeners & Bootstrapping
// // =============================================================================

// localeSelect.addEventListener("change", (e) => {
//     applyLocale(e.target.value);
//     saveUserSettings();
// });

// notationModeSelect.addEventListener("change", () => {
//     const loc = localeSelect.value;
//     const strings = I18N[loc] || I18N["en-US"];
//     const isSargam = (notationModeSelect.value === "Sargam");
//     document.getElementById("rootLabel").innerText = isSargam ? strings.saRoot : strings.westernRoot;
//     document.getElementById("scaleLabel").innerText = isSargam ? strings.scaleThaat : strings.scaleWestern;
//     rebuildKeyboard();
// });

// rootKeySelect.addEventListener("change", rebuildKeyboard);
// scaleModeSelect.addEventListener("change", rebuildKeyboard);
// qwertyOctaveSelect.addEventListener("change", () => {
//     rebuildKeyboard();
//     const baseMidi = parseInt(qwertyOctaveSelect.value, 10);
//     const target = keyElements.find(k => k.midi === baseMidi);
//     if (target && target.el) {
//         target.el.scrollIntoView({ behavior: 'smooth', inline: 'center' });
//     }
// });

// loopBtn.addEventListener("click", () => {
//     isLooping = !isLooping;
//     const loc = localeSelect.value;
//     const strings = I18N[loc] || I18N["en-US"];
//     if (isLooping) {
//         loopBtn.className = "stop";
//         loopBtn.innerText = strings.stopLoop;
//         fetch(`/loop?action=start&pattern=${encodeURIComponent(loopPattern.value)}&bpm=${encodeURIComponent(bpmInput.value)}`, {
//             keepalive: true, cache: "no-store"
//         });
//     } else {
//         loopBtn.className = "start";
//         loopBtn.innerText = strings.startLoop;
//         fetch(`/loop?action=stop`, { keepalive: true, cache: "no-store" });
//     }
// });

// window.addEventListener("beforeunload", () => {
//     navigator.sendBeacon("/shutdown");
// });

// window.addEventListener("mousedown", () => { isMouseDown = true; });
// window.addEventListener("mouseup", () => {
//     isMouseDown = false;
//     clearAllActiveKeys();
// });
// window.addEventListener("blur", clearAllActiveKeys);

// window.addEventListener("keydown", (e) => {
//     if (document.activeElement && document.activeElement.tagName === "INPUT" && document.activeElement.type === "number") {
//         return;
//     }

//     if (document.activeElement && document.activeElement.tagName === "SELECT") {
//         document.activeElement.blur();
//     }

//     const k = e.key.toLowerCase();
//     const isDrumKey = ['1', '2', '3', '4'].includes(k);
//     const keyObj = keyElements.find(item => item.qwertyKey === k);

//     if (isDrumKey || keyObj) {
//         e.preventDefault();
//     }

//     if (e.repeat) return;

//     if (k === '1') { sendDrumHit(0); return; }
//     if (k === '2') { sendDrumHit(1); return; }
//     if (k === '3') { sendDrumHit(2); return; }
//     if (k === '4') { sendDrumHit(6); return; }

//     if (keyObj) {
//         activateKeyElement(keyObj.el, keyObj.freq);
//     }
// });

// window.addEventListener("keyup", (e) => {
//     const k = e.key.toLowerCase();
//     const keyObj = keyElements.find(item => item.qwertyKey === k);
//     if (keyObj) {
//         deactivateKeyElement(keyObj.el);
//     }
// });

// // Attach global methods needed for inline HTML events
// window.openSettingsModal = openSettingsModal;
// window.closeSettingsModal = closeSettingsModal;
// window.toggleQwertyOverlay = toggleQwertyOverlay;
// window.closeApplication = closeApplication;
// window.updateEq = updateEq;
// window.applyEqPreset = applyEqPreset;
// window.sendDrumHit = sendDrumHit;

// // Boot
// loadUserSettings();

// setTimeout(() => {
//     const midKey = keyElements.find(k => k.midi === 60);
//     if (midKey && midKey.el) {
//         midKey.el.scrollIntoView({ inline: 'center' });
//     }
// }, 100);

import { I18N, SWARA_SCRIPTS } from './locales.js';

// Scale interval definitions
const SCALE_INTERVALS = {
    "Bilawal":          [0, 2, 4, 5, 7, 9, 11],
    "Kafi":             [0, 2, 3, 5, 7, 9, 10],
    "Bhairav":          [0, 1, 4, 5, 7, 8, 11],
    "Yaman":            [0, 2, 4, 6, 7, 9, 11],
    "Asavari":          [0, 2, 3, 5, 7, 8, 10],
    "Khamaj":           [0, 2, 4, 5, 7, 9, 10],
    "Bhairavi":         [0, 1, 3, 5, 7, 8, 10],
    "Todi":             [0, 1, 3, 6, 7, 8, 11],
    "Chromatic":        [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11],
    "PentatonicMajor":  [0, 2, 4, 7, 9],
    "PentatonicMinor":  [0, 3, 5, 7, 10],
    "Blues":            [0, 3, 5, 6, 7, 10]
};

const NOTE_NAMES = ["C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"];
const BLACK_KEY_PITCHES = [1, 3, 6, 8, 10];

const QWERTY_OFFSETS = [
    { offset: 0,  key: "a" },
    { offset: 1,  key: "w" },
    { offset: 2,  key: "s" },
    { offset: 3,  key: "e" },
    { offset: 4,  key: "d" },
    { offset: 5,  key: "f" },
    { offset: 6,  key: "t" },
    { offset: 7,  key: "g" },
    { offset: 8,  key: "y" },
    { offset: 9,  key: "h" },
    { offset: 10, key: "u" },
    { offset: 11, key: "j" },
    { offset: 12, key: "k" },
    { offset: 13, key: "o" },
    { offset: 14, key: "l" },
    { offset: 15, key: "p" },
    { offset: 16, key: ";" }
];

// Equalizer Presets
const EQ_PRESETS = {
    "Flat":      { 60: 0,  250: 0,  1000: 0,  4000: 0,  12000: 0 },
    "Rock":      { 60: 5,  250: 2,  1000: -1, 4000: 3,  12000: 6 },
    "Disco":     { 60: 6,  250: 3,  1000: 0,  4000: 2,  12000: 5 },
    "Indian":    { 60: 2,  250: 4,  1000: 1,  4000: 5,  12000: 3 },
    "BassBoost": { 60: 8,  250: 5,  1000: 0,  4000: -2, 12000: -4 },
    "Vocal":     { 60: -3, 250: 1,  1000: 5,  4000: 4,  12000: 1 }
};

// DOM references
const keyboard = document.getElementById("keyboard");
const instSelect = document.getElementById("melodicInstrument");
const notationModeSelect = document.getElementById("notationMode");
const rootKeySelect = document.getElementById("rootKey");
const scaleModeSelect = document.getElementById("scaleMode");
const qwertyOctaveSelect = document.getElementById("qwertyOctave");
const volSlider = document.getElementById("vol");
const loopBtn = document.getElementById("toggleLoopBtn");
const loopPattern = document.getElementById("loopPattern");
const bpmInput = document.getElementById("bpm");
const localeSelect = document.getElementById("localeSelect");
const settingsModal = document.getElementById("settingsModal");
const helpModal = document.getElementById("helpModal");
const aboutModal = document.getElementById("aboutModal");
const showQwertyCheck = document.getElementById("showQwertyCheck");
const enableVisualizerCheck = document.getElementById("enableVisualizerCheck");
const eqPresetSelect = document.getElementById("eqPreset");

let isLooping = false;
let isMouseDown = false;
let showQwertyLabels = true;
const activeKeyElements = new Set();
let keyElements = [];

// Equalizer state (10 bins tailored for the 80x88px canvas)
const eqGains = { 60: 0, 250: 0, 1000: 0, 4000: 0, 12000: 0 };
const canvas = document.getElementById("eqCanvas");
const ctx = canvas.getContext("2d");
const spectrumBins = new Array(10).fill(2);

// =============================================================================
// Modals & User Preferences
// =============================================================================

export function openSettingsModal() {
    if (settingsModal) settingsModal.classList.add("open");
}

export function closeSettingsModal() {
    if (settingsModal) settingsModal.classList.remove("open");
    saveUserSettings();
}

export function openHelpModal() {
    if (helpModal) helpModal.classList.add("open");
}

export function closeHelpModal() {
    if (helpModal) helpModal.classList.remove("open");
}

export function openAboutModal() {
    if (aboutModal) aboutModal.classList.add("open");
}

export function closeAboutModal() {
    if (aboutModal) aboutModal.classList.remove("open");
}

export function toggleQwertyOverlay(show) {
    showQwertyLabels = show;
    rebuildKeyboard();
}

function saveUserSettings() {
    const settings = {
        locale: localeSelect.value,
        showQwerty: showQwertyCheck.checked,
        enableVisualizer: enableVisualizerCheck.checked
    };
    localStorage.setItem("user_synthesizer_settings", JSON.stringify(settings));
}

function loadUserSettings() {
    const raw = localStorage.getItem("user_synthesizer_settings");
    let settings = { locale: "en-US", showQwerty: true, enableVisualizer: true };
    if (raw) {
        try { settings = Object.assign(settings, JSON.parse(raw)); } catch (e) {}
    }
    localeSelect.value = settings.locale || "en-US";
    showQwertyCheck.checked = settings.showQwerty;
    enableVisualizerCheck.checked = settings.enableVisualizer;
    showQwertyLabels = settings.showQwerty;
    applyLocale(settings.locale || "en-US");
}

// =============================================================================
// I18N Locale Applicator
// =============================================================================

function setElementText(id, val) {
    const el = document.getElementById(id);
    if (el && val !== undefined) {
        el.innerText = val;
    }
}

function setOptgroupLabel(id, val) {
    const el = document.getElementById(id);
    if (el && val !== undefined) {
        el.label = val;
    }
}

function applyLocale(locale) {
    const strings = I18N[locale] || I18N["en-US"];

    // Main Layout Headers & Controls
    setElementText("lblAppTitle", strings.appTitle);
    setElementText("lblEqHeader", strings.eqHeader);
    setElementText("lblRhythmHeader", strings.rhythmHeader);
    setElementText("lblKeyboardHeader", strings.keyboardHeader);
    setElementText("lblGroove", strings.groove);
    setElementText("lblBpm", strings.bpm);
    setElementText("lblMasterVol", strings.volume);
    setElementText("lblInstrument", strings.instrument);
    setElementText("lblNotation", strings.notation);
    setElementText("lblOctave", strings.qwertyOctave);
    setElementText("shutdownBtn", strings.quit);

    // Header Actions
    setElementText("btnHelpLabel", strings.btnHelpLabel);
    setElementText("btnAboutLabel", strings.btnAboutLabel);
    setElementText("btnSettingsLabel", strings.btnSettingsLabel);

    // Settings Modal
    setElementText("modalTitle", strings.modalTitle);
    setElementText("setLocaleTitle", strings.setLocaleTitle);
    setElementText("setLocaleSub", strings.setLocaleSub);
    setElementText("setKeybTitle", strings.setKeybTitle);
    setElementText("setKeybSub", strings.setKeybSub);
    setElementText("setEqualizerTitle", strings.setEqualizerTitle);
    setElementText("setEqualizerSub", strings.setEqualizerSub);
    setElementText("btnSaveClose", strings.saveClose);

    // Help Modal
    setElementText("helpModalTitle", strings.helpModalTitle);
    setElementText("helpKbTitle", strings.helpKbTitle);
    setElementText("helpKbDesc", strings.helpKbDesc);
    setElementText("helpDrumTitle", strings.helpDrumTitle);
    setElementText("helpEqTitle", strings.helpEqTitle);
    setElementText("helpEqDesc", strings.helpEqDesc);
    setElementText("helpSargamDesc", strings.helpSargamDesc);
    setElementText("btnCloseHelp", strings.btnCloseHelp);

    // About Modal
    setElementText("aboutModalTitle", strings.aboutModalTitle);
    setElementText("aboutAuthorLabel", strings.aboutAuthorLabel);
    setElementText("aboutEngineLabel", strings.aboutEngineLabel);
    setElementText("aboutLicenseLabel", strings.aboutLicenseLabel);
    setElementText("aboutCopyrightText", strings.aboutCopyrightText);
    setElementText("btnCloseAbout", strings.btnCloseAbout);

    // Equalizer Preset Selector
    setElementText("lblEqPreset", strings.lblEqPreset);
    setElementText("optPresetFlat", strings.presetFlat);
    setElementText("optPresetRock", strings.presetRock);
    setElementText("optPresetDisco", strings.presetDisco);
    setElementText("optPresetIndian", strings.presetIndian);
    setElementText("optPresetBass", strings.presetBass);
    setElementText("optPresetVocal", strings.presetVocal);
    setElementText("optPresetCustom", strings.presetCustom);

    // Rhythm Grooves & Percussion Pads
    setElementText("optRock", strings.rock);
    setElementText("optFunk", strings.funk);
    setElementText("optMetronome", strings.metronome);
    setElementText("pad0", strings.pad0);
    setElementText("pad1", strings.pad1);
    setElementText("pad2", strings.pad2);
    setElementText("pad6", strings.pad6);

    // Instrument Group & Option Labels
    setOptgroupLabel("grpKeys", strings.grpKeys);
    setOptgroupLabel("grpPlucked", strings.grpPlucked);
    setOptgroupLabel("grpWinds", strings.grpWinds);

    // Dynamic Loop Button
    loopBtn.innerText = isLooping ? strings.stopLoop : strings.startLoop;

    // Sargam vs Western Notation Root and Scale labels
    const isSargam = (notationModeSelect.value === "Sargam");
    setElementText("rootLabel", isSargam ? strings.saRoot : strings.westernRoot);
    setElementText("scaleLabel", isSargam ? strings.scaleThaat : strings.scaleWestern);

    rebuildKeyboard();
}

// =============================================================================
// Equalizer & Spectrum Visualizer (Classic Media Player Style)
// =============================================================================

function setBandValue(freq, val) {
    eqGains[freq] = parseFloat(val);
    const sliderMap = { 60: "eq60", 250: "eq250", 1000: "eq1k", 4000: "eq4k", 12000: "eq12k" };
    const labelMap  = { 60: "val60", 250: "val250", 1000: "val1k", 4000: "val4k", 12000: "val12k" };

    const slider = document.getElementById(sliderMap[freq]);
    if (slider) slider.value = val;

    const label = document.getElementById(labelMap[freq]);
    if (label) label.innerText = (val > 0 ? "+" : "") + val + "dB";
}

export function updateEq(freq, val) {
    setBandValue(freq, val);
    if (eqPresetSelect) {
        eqPresetSelect.value = "Custom";
    }
}

export function applyEqPreset(presetName) {
    const preset = EQ_PRESETS[presetName];
    if (!preset) return;

    for (const [freq, gain] of Object.entries(preset)) {
        setBandValue(parseInt(freq, 10), gain);
    }
}

function stimulateSpectrum(freqRatio) {
    if (!enableVisualizerCheck.checked) return;
    const targetBin = Math.min(spectrumBins.length - 1, Math.floor(freqRatio * spectrumBins.length));
    spectrumBins[targetBin] = Math.min(48, spectrumBins[targetBin] + 40);
    if (targetBin > 0) spectrumBins[targetBin - 1] += 18;
    if (targetBin < spectrumBins.length - 1) spectrumBins[targetBin + 1] += 18;
}

function drawSpectrum() {
    ctx.fillStyle = "#040608";
    ctx.fillRect(0, 0, canvas.width, canvas.height);

    const numSegments = 16;
    const segmentHeight = 3.5;
    const segmentSpacing = 1.5;
    const barWidth = Math.floor((canvas.width - (spectrumBins.length * 2)) / spectrumBins.length);

    for (let i = 0; i < spectrumBins.length; ++i) {
        spectrumBins[i] = Math.max(2, spectrumBins[i] * 0.90);

        const activeSegments = Math.floor((spectrumBins[i] / 50.0) * numSegments);
        const x = i * (barWidth + 2) + 2;

        for (let s = 0; s < numSegments; ++s) {
            const y = canvas.height - ((s + 1) * (segmentHeight + segmentSpacing));

            if (s < activeSegments) {
                if (s >= 13) {
                    ctx.fillStyle = "#ff2a2a";
                } else if (s >= 9) {
                    ctx.fillStyle = "#ffb000";
                } else {
                    ctx.fillStyle = "#00e676";
                }
            } else {
                ctx.fillStyle = "rgba(255, 255, 255, 0.035)";
            }
            ctx.fillRect(x, y, barWidth, segmentHeight);
        }
    }
    requestAnimationFrame(drawSpectrum);
}
drawSpectrum();

// =============================================================================
// Audio Dispatch & Keyboard Core
// =============================================================================

function midiToFreq(midi) {
    return 440.0 * Math.pow(2.0, (midi - 69.0) / 12.0);
}

export function sendPlay(freq) {
    const inst = instSelect.value;
    const vol = volSlider.value;
    stimulateSpectrum(Math.min(1.0, freq / 2000.0));
    fetch(`/play?inst=${inst}&freq=${freq}&vol=${vol}&dur=1.2`, {
        method: "GET",
        keepalive: true,
        cache: "no-store",
        priority: "high"
    }).catch(() => {});
}

export function sendDrumHit(pieceId) {
    stimulateSpectrum(pieceId === 0 ? 0.08 : 0.45);
    fetch(`/drumHit?piece=${pieceId}`, {
        method: "GET",
        keepalive: true,
        cache: "no-store",
        priority: "high"
    }).catch(() => {});
}

function clearAllActiveKeys() {
    activeKeyElements.forEach(el => el.classList.remove("active"));
    activeKeyElements.clear();
}

function activateKeyElement(el, freq) {
    if (!el || activeKeyElements.has(el)) return;
    el.classList.add("active");
    activeKeyElements.add(el);
    sendPlay(freq);
}

function deactivateKeyElement(el) {
    if (!el) return;
    el.classList.remove("active");
    activeKeyElements.delete(el);
}

function rebuildKeyboard() {
    clearAllActiveKeys();
    keyboard.innerHTML = "";
    keyElements = [];

    const currentLocale = localeSelect.value || "en-US";
    const swaraList = SWARA_SCRIPTS[currentLocale] || SWARA_SCRIPTS["en"];

    const isSargam = (notationModeSelect.value === "Sargam");
    const rootOffset = parseInt(rootKeySelect.value, 10);
    const scaleIntervals = SCALE_INTERVALS[scaleModeSelect.value] || SCALE_INTERVALS["Bilawal"];
    const qwertyBase = parseInt(qwertyOctaveSelect.value, 10);

    let whiteOffset = 0;
    const whiteKeyWidth = 32;

    for (let midi = 36; midi <= 96; ++midi) {
        const pitchClass = (midi % 12);
        const isBlack = BLACK_KEY_PITCHES.includes(pitchClass);
        const freq = midiToFreq(midi);

        const swaraDegree = (pitchClass - rootOffset + 12) % 12;
        const inScale = scaleIntervals.includes(swaraDegree);
        const westernName = NOTE_NAMES[pitchClass] + Math.floor(midi / 12 - 1);

        let swaraText = swaraList[swaraDegree];
        if (midi >= 72) {
            swaraText += "̇";
        } else if (midi < 60) {
            swaraText += "̣";
        }

        const qwertyItem = QWERTY_OFFSETS.find(q => (qwertyBase + q.offset) === midi);
        const qwertyKey = (qwertyItem && showQwertyLabels) ? qwertyItem.key : "";

        const el = document.createElement("div");
        el.dataset.freq = freq.toFixed(2);
        el.dataset.midi = midi;

        const modeClass = isSargam ? "sargam-mode" : "";
        const qwertyClass = qwertyKey ? "qwerty-mapped" : "";

        if (!isBlack) {
            el.className = `white-key ${modeClass} ${qwertyClass} ${inScale ? 'in-scale' : 'out-scale'}`;
            const topLabel = qwertyKey ? qwertyKey.toUpperCase() : "";
            if (isSargam) {
                el.innerHTML = `<span>${topLabel}</span><span class="swara-label">${swaraText}</span><span class="note-label">${westernName}</span>`;
            } else {
                el.innerHTML = `<span>${topLabel}</span><span class="note-label">${westernName}</span>`;
            }
            keyboard.appendChild(el);
            whiteOffset += whiteKeyWidth;
        } else {
            el.className = `black-key ${modeClass} ${qwertyClass} ${inScale ? 'in-scale' : 'out-scale'}`;
            el.style.left = (whiteOffset - 10) + "px";
            const topLabel = qwertyKey ? qwertyKey.toUpperCase() : "";
            if (isSargam) {
                el.innerHTML = `<span>${topLabel}</span><span class="swara-label">${swaraText}</span>`;
            } else {
                el.innerHTML = `<span>${topLabel}</span>`;
            }
            keyboard.appendChild(el);
        }

        el.addEventListener("mousedown", (e) => {
            e.preventDefault();
            activateKeyElement(el, freq);
        });

        el.addEventListener("mouseenter", () => {
            if (isMouseDown) activateKeyElement(el, freq);
        });

        el.addEventListener("mouseleave", () => deactivateKeyElement(el));
        el.addEventListener("mouseup", () => deactivateKeyElement(el));

        keyElements.push({ el, freq, midi, qwertyKey });
    }
}

export function closeApplication() {
    if (confirm("Are you sure you want to stop the music engine and exit?")) {
        navigator.sendBeacon("/shutdown");
        document.body.innerHTML = `
            <div style="text-align:center; margin-top:20vh;">
                <h2 style="color:#28a745;">Application Terminated</h2>
                <p style="color:#aaa;">Audio subsystem and backend server have exited cleanly. You can now close this tab.</p>
            </div>`;
        setTimeout(() => window.close(), 1500);
    }
}

// =============================================================================
// Event Listeners & Bootstrapping
// =============================================================================

localeSelect.addEventListener("change", (e) => {
    applyLocale(e.target.value);
    saveUserSettings();
});

notationModeSelect.addEventListener("change", () => {
    const loc = localeSelect.value;
    const strings = I18N[loc] || I18N["en-US"];
    const isSargam = (notationModeSelect.value === "Sargam");
    setElementText("rootLabel", isSargam ? strings.saRoot : strings.westernRoot);
    setElementText("scaleLabel", isSargam ? strings.scaleThaat : strings.scaleWestern);
    rebuildKeyboard();
});

rootKeySelect.addEventListener("change", rebuildKeyboard);
scaleModeSelect.addEventListener("change", rebuildKeyboard);
qwertyOctaveSelect.addEventListener("change", () => {
    rebuildKeyboard();
    const baseMidi = parseInt(qwertyOctaveSelect.value, 10);
    const target = keyElements.find(k => k.midi === baseMidi);
    if (target && target.el) {
        target.el.scrollIntoView({ behavior: 'smooth', inline: 'center' });
    }
});

loopBtn.addEventListener("click", () => {
    isLooping = !isLooping;
    const loc = localeSelect.value;
    const strings = I18N[loc] || I18N["en-US"];
    if (isLooping) {
        loopBtn.className = "stop";
        loopBtn.innerText = strings.stopLoop;
        fetch(`/loop?action=start&pattern=${encodeURIComponent(loopPattern.value)}&bpm=${encodeURIComponent(bpmInput.value)}`, {
            keepalive: true, cache: "no-store"
        });
    } else {
        loopBtn.className = "start";
        loopBtn.innerText = strings.startLoop;
        fetch(`/loop?action=stop`, { keepalive: true, cache: "no-store" });
    }
});

window.addEventListener("beforeunload", () => {
    navigator.sendBeacon("/shutdown");
});

window.addEventListener("mousedown", () => { isMouseDown = true; });
window.addEventListener("mouseup", () => {
    isMouseDown = false;
    clearAllActiveKeys();
});
window.addEventListener("blur", clearAllActiveKeys);

window.addEventListener("keydown", (e) => {
    if (document.activeElement && document.activeElement.tagName === "INPUT" && document.activeElement.type === "number") {
        return;
    }

    if (document.activeElement && document.activeElement.tagName === "SELECT") {
        document.activeElement.blur();
    }

    const k = e.key.toLowerCase();
    const isDrumKey = ['1', '2', '3', '4'].includes(k);
    const keyObj = keyElements.find(item => item.qwertyKey === k);

    if (isDrumKey || keyObj) {
        e.preventDefault();
    }

    if (e.repeat) return;

    if (k === '1') { sendDrumHit(0); return; }
    if (k === '2') { sendDrumHit(1); return; }
    if (k === '3') { sendDrumHit(2); return; }
    if (k === '4') { sendDrumHit(6); return; }

    if (keyObj) {
        activateKeyElement(keyObj.el, keyObj.freq);
    }
});

window.addEventListener("keyup", (e) => {
    const k = e.key.toLowerCase();
    const keyObj = keyElements.find(item => item.qwertyKey === k);
    if (keyObj) {
        deactivateKeyElement(keyObj.el);
    }
});

// Attach global methods needed for inline HTML events
window.openSettingsModal = openSettingsModal;
window.closeSettingsModal = closeSettingsModal;
window.openHelpModal = openHelpModal;
window.closeHelpModal = closeHelpModal;
window.openAboutModal = openAboutModal;
window.closeAboutModal = closeAboutModal;
window.toggleQwertyOverlay = toggleQwertyOverlay;
window.closeApplication = closeApplication;
window.updateEq = updateEq;
window.applyEqPreset = applyEqPreset;
window.sendDrumHit = sendDrumHit;

// Boot
loadUserSettings();

setTimeout(() => {
    const midKey = keyElements.find(k => k.midi === 60);
    if (midKey && midKey.el) {
        midKey.el.scrollIntoView({ inline: 'center' });
    }
}, 100);