# Sundrift development notes

**v0.1.2, 11 November 2026: EDGE and drive refinement**

- Added `edge` as an independent stateful host parameter and an eighth processing step for upper-mid attack.
- Updated all 12 recipes, processor mappings, offline bridge, test and demo renderers, and the Murmur-inspired front panel.
- Preserved existing parameter IDs; the only new ID is `edge`.
- New stage is after neural/built-in amplification, before body/tone and movement effects.
- Tests: dependency-free DSP suite at 44.1/48/96 kHz, distinct bank fingerprints, external NAM bypass, fractional ring wrap, and EDGE harmonic-generation test.
- Rerendered the 12 real-guitar DI previews. Source package excludes the user recording and processed user audio.
- Compiled VST3 and NAM end-to-end tests still outstanding.

# Sundrift developer handover

**v0.1.1 initial scaffold and real DI validation, 11 November 2026**

## Product direction

Murmur-style Flip Fella dark front panel, rotary clock dials and split-flap display, four visually distinct **band banks** with three presets each. Sonically prioritise expressive clean leads, cumbia grooves, phase-heavy psych and ambient guitar. Everything here is original DSP; no artist samples or captures bundled.

## File map

- `Source/ToneEngine.h`: self-contained allocation-free-in-process DSP engine, no JUCE includes.
- `Source/Presets.h`: stable band and preset order, 18 parameter recipes as of v0.1.2.
- `Source/PluginProcessor.*`: JUCE wrapper, APVTS automation/state, host programs, NAM model loading and optional playback.
- `Source/PluginEditor.*`: hand-drawn GUI, band switching, split-flap preset selection, rotary knobs and NAM chooser.
- `tools/render_demo.cpp`: synthetic guitar-ish signal generator / WAV renderer.
- `tests/test_engine.cpp`: 12-preset render smoke tests at three sample rates, finite/bounded output, distinct first presets per band.
- `.github/workflows/build-windows.yml`: Windows 2022 cloud build job for VST3.

## Real DI validation / fixed issue

- 24-second dual-mono-like stereo 44.1 kHz PCM16 guitar recording rendered through **every one of the 12 presets**. These audio samples remain private to the working conversation and **should not be committed to a public repository** without permission.
- This revealed an out-of-bounds read in `Ring::tap` when a float interpolation index wrapped just below zero, was shifted by the buffer length and rounded up to `samples.size()`. The fix uses integer circular indices and separately interpolates the fractional delay. Important for Glass Beams phaser/chorus presets.
- Added direct Ring wraparound regression checks in Release builds, which now use `CHECK` instead of `assert` (which compiles away with `NDEBUG`). Re-ran tests at 44.1 / 48 / 96 kHz and short AddressSanitizer+UBSan stress through preset 7.
- `tools/render_di_bridge.cpp` and `tools/render_user_di.py` use the exact core DSP for repeatable guitar DI previews. The preview utility rejects abnormal runaway peaks and level-matches only for demo export. It does not change live plugin output.

## Known technical issues / extensions

1. JUCE VST3 binary not yet compiled or DAW-tested; fix any action logs when first pushed. Avoid calling the source archive a prebuilt plugin.
2. NAM `NeuralAmpModelerCore` v0.5.2 included only at build time via FetchContent with `NAM_ENABLE_A2_FAST` and `NAM_SAMPLE_FLOAT`; load model on message thread, prewarm before use. Keep sample rate equal to capture's training rate until proper SRC is implemented.
3. Neural models are fed the input gain first; the tone engine then applies compression/EQ and the wet chain. If future sound tests show that guitar EQ should precede model, split pre and post stages rather than silently double gain.
4. Retired NAM models are retained through plugin lifetime to avoid destructing a neural model on the audio callback when swapping. Once safe deferred cleanup is implemented, reclaim them off the audio thread.
5. Parameters update per audio block with no smoothing of EQ coefficients, so add de-zippering before a public release.
6. Phaser and reverb are intentionally lightweight approximations. For higher fidelity, add oversampling for drive, proper preamp/cab responses and tempo-synchronised echoes, then A/B on dry DI.
7. Disabling the NAM build with `-DSUNDRIFT_WITH_NAM=OFF` builds the original DSP-only mode; the NAM loader button is disabled and its status explains why.
8. Need more testing for stereo host layouts and automation, UI layout at Windows scaling, host preset restore and file paths with non-ASCII characters.
9. Do not copy Murmur code wholesale. This GUI is a new implementation with similar design language; some Murmur elements such as the reactive character and MIDI pedal controls are not part of v0.1.

## Consistency rules for continuing development

- Preserve the four named band banks and 3 preset slots (unless explicitly asked to expand).
- Keep `Presets.h` value-array order in lockstep with `PluginProcessor::knobIDs` and `parameterLayout()`.
- Keep user-facing labels and interface consistent with Murmur's compact black/cream split-flap style.
- No bundled commercial guitar samples, song recordings or non-permissively licensed `.nam` captures.
- Use versioned, pinned NAM/JUCE sources and retain licensing notes.
- After changes, build offline tests and run at least the 44.1/48/96kHz smoke test.
