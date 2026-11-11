# Sundrift | Claude development handover

Read this before modifying the project. The source of truth is **this repository**: https://github.com/kevinwkaye/sundrift on `main`, not `kevinwkaye/murmur`.

## What Sundrift is

Sundrift v0.1.2 is a new, artist-inspired psychedelic guitar multi-effect VST3 for Windows, designed for an Ableton Live guitarist. Its sonic touchstones are BALTHVS (with *Sunday Forecast* as the primary reference), Khruangbin, Glass Beams and Yuuf. The owner is interested in warm clean guitar with movement, not huge rock distortion. Latest feedback: some tones needed **more bite and slight overdrive**, but distortion must remain fully user-adjustable. Preserve musicality, clarity, dynamic picking response and note definition.

Inspired tones are creative interpretations, not licensed recreations. No artist audio stems, captured gear models, proprietary samples or commercial presets are bundled.

This is a **separate project** from Murmur. Use Murmur as a visual style reference only, if accessible; **do not import Murmur classes or overwrite its repo**.

## Visual design brief

Carry over the Flip Fella / Murmur design vocabulary: almost-black metal face, cream/white labels, tactile clock-inspired knobs, small pill controls, mechanical split-flap preset readout and restrained motion. It should not resemble a generic stack of guitar pedal photographs.

There must be four instantly distinguishable artist banks, with band name displayed clearly and a distinct accent colour that updates the selection and preset display:

- BALTHVS: amber (#E9AD54), presets Sunday Forecast, Golden Cumbia, Lunar Coast
- Khruangbin: sea green (#72BCA9), presets Silk Lines, Desert Dub, Texas Velvet
- Glass Beams: violet (#AB99DC), presets Mirage Pulse, Temple Steps, Copper Serpent
- Yuuf: sky blue (#85BEE0), presets Afterglow, Coastal Float, Late Drift

Keep three presets per bank and preserve host recall. Make the controls readable, consistent and responsive at normal Windows DPI scaling. Maintain clear on-screen labels and knob values. `DRIVE` and `EDGE` must be independently adjustable regardless of which preset is selected.

## Current implementation (inspect the code, do not assume it is perfect)

- `CMakeLists.txt`: JUCE 8.0.17-security fetched at configure time. Builds an x64 VST3. Optional `SUNDRIFT_WITH_NAM=ON` fetches NeuralAmpModelerCore v0.5.2 with `NAM_SAMPLE_FLOAT` and `NAM_ENABLE_A2_FAST`. This is the optional A2-capable amp **playback** path, not an A2 model trainer and not proof that every model receives the A2 fast path.
- `Source/ToneEngine.h`: self-contained, JUCE-free, C++17 stereo engine. It allocates its ring buffers/reverb state in `prepare()`, then processes without heap allocation in `process()`. Stages: input gain -> shared peak compression -> resonant mid focus -> soft asymmetric built-in preamp (or external NAM amp) -> independent EDGE peak/pick-grit stage -> body/tone high/low shaping -> 4-stage phaser -> chorus and tape wobble -> stereo delay with crossfeed -> comb/allpass algorithmic reverb -> stereo width -> output.
- `Source/Presets.h`: 4 banks, 12 presets and 18-element float parameter arrays, ordered to match the processor.
- `Source/PluginProcessor.*`: JUCE AudioProcessor, APVTS parameters, host program indices, state save/load, mono/stereo layout support and NAM model file loading. `NAM A2` mode loads a `.nam` capture using two mono model instances for stereo; model and scratch buffers are prepared off the audio thread. A capture's sample rate must match the DAW session currently. If no valid model is present, the built-in amp remains usable.
- `Source/PluginEditor.*`: custom painted GUI with band selector, split-flap display, fixed physical knob positions, preset chooser and file picker for a NAM capture.
- `tests/test_engine.cpp`: core smoke/regression checks at 44.1/48/96 kHz, including finiteness, boundedness, differing preset fingerprints, fractional delay wrap safety and EDGE harmonic generation.
- `tools/render_demo.cpp`, `tools/render_di_bridge.cpp`, `tools/render_user_di.py`: synthetic sounds and repeatable audio rendering from a private guitar DI when supplied locally. The user's DI recording is NOT in this public repository.
- `.github/workflows/build-windows.yml`: Linux DSP test job and Windows 2022 VST3 build/upload job. Check Actions results, address compiler or test failures and then run a DAW compatibility test. A successful GitHub compile does not automatically prove Ableton compatibility.

### Important audio controls

The processor holds 18 automatable floats in this exact order, plus a separate `ampMode` choice:

`input` dB, `compress`, `focus`, `drive`, `edge`, `body`, `tone`, `phaseRate` Hz, `phase`, `chorus`, `wobble`, `echoTime` ms, `feedback`, `echoMix`, `space`, `decay`, `width`, `output` dB.

`DRIVE` controls the built-in preamp's nonlinear gain. When NAM is active the built-in DRIVE stage is intentionally bypassed to avoid stacking amp saturation. `EDGE` adds an upper-mid (2.1 kHz) peak with blended asymmetric clipping AFTER whichever amp engine is active, so it still works in NAM mode. At EDGE=0 its special processing should be neutral. Do not conflate these controls or silently make the presets fully distorted. If you want to improve this behaviour, propose and test an approach.

`Source/Presets.h` entries map to `SundriftAudioProcessor::knobIDs` and `parameterLayout()`. Update all three together, and preserve parameter IDs for DAW automation/project compatibility.

## History and validation

v0.1.1 established the baseline sound banks and GUI. A real clean 24 s guitar DI exposed an interpolation wraparound out-of-bounds bug in the fractional delay ring; v0.1.1 corrected this using integer circular indices, and added regression testing. v0.1.2 added EDGE and pushed some BALTHVS and Glass Beams presets slightly further into breakup. Core tests and private DI renders were exercised at 44.1/48/96 kHz. A preceding GitHub Windows build in the temporary Murmur branch completed successfully, but treat the **new Sundrift workflow** as authoritative for future releases.

Do not commit the user's clean DI, audio renders, credentials or third-party `.nam` capture files to the public repository without explicit permission.

## Areas to polish, highest priority

1. **Audit the complete Windows VST3 build** in the new repository, confirm the downloaded .vst3 loads into Ableton Live, and check automation, presets, saving/reopening, audio routing and knob values. Run pluginval or equivalent host compatibility testing; log the real outcome. Do not claim builds/tests pass without running them.
2. **Audition the tonal response** using a clean DI, especially BALTHVS Sunday Forecast and Glass Beams Mirage Pulse. Preserve warmth but improve dynamic response, pick attack and low-gain overdrive. Level-match all A/B tests so greater loudness is not mistaken for better tone.
3. **Oversample nonlinear stages** to reduce high-frequency aliasing at higher drive/edge settings. Ensure acceptable CPU on realistic Ableton buffer sizes; do not introduce audio-callback allocations.
4. **Add parameter smoothing** to avoid zipper noise while automating drive, filters, mixes, delay, width and output; deal with changes that could cause discontinuities.
5. **NAM model management:** audit UTF-8 file paths, sample rate support, error handling, stereo/mono models, loading/prewarming on non-real-time thread, cancellation/replacement and retirement without unbounded memory accumulation. User-loaded models should never be parsed or deallocated on the audio thread.
6. **Mix quality:** check wet/dry response, gain staging, stereo image, mono compatibility, tail decay, reverb/delay limits and phase alignment. Consider more convincing cabinet filtering/IR for built-in amps and tasteful tempo sync.
7. **User experience:** prevent accidental preset overwrites, implement A/B comparison or simple undo, make bank/preset selection intuitive, improve readout/tooltips, check accessibility and DPI scaling. Preserve the Murmur-inspired visual identity and per-band accent colours.
8. **Release hygiene:** publish repeatable Windows VST3 builds with pinned dependency versions; update README for real installation steps; add clear third-party licence acknowledgements and project licence after owner chooses one.

## Hard constraints and working style

- Always work in `kevinwkaye/sundrift`, never `kevinwkaye/murmur`.
- Preserve existing preset names and four separate artist banks unless the owner requests changes.
- Preserve (or carefully migrate) existing automation IDs and saved presets.
- `DRIVE` and `EDGE` must remain available to the user, not locked to a recipe.
- Keep built-in tones usable with no NAM capture. NAM A2 acceleration is an optional improvement, not a requirement for clean tones.
- Prefer simple, efficient, maintainable C++ and real-time-safe audio. No network access on the audio thread.
- Use British English in copy and docs. Avoid em dashes.
- Make small commits, run tests, and explain actual changes and remaining issues. Do not redesign the sound palette into modern metal.
- Before major changes, inspect and reference the actual source, existing issues and current Actions results. For next session, start by fixing any new build/host bugs, then focus on tonal polish.

## Commands

Standalone core smoke tests without JUCE:

```bash
cmake -S tests -B build-dsp -DCMAKE_BUILD_TYPE=Release
cmake --build build-dsp --parallel
ctest --test-dir build-dsp --output-on-failure
```

Windows VS2022 VST3, with NAM acceleration:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DSUNDRIFT_WITH_NAM=ON -DSUNDRIFT_BUILD_TESTS=ON
cmake --build build --config Release --target Sundrift_VST3 sundrift_tests --parallel 4
ctest --test-dir build -C Release --output-on-failure
```

Without NAM: substitute `-DSUNDRIFT_WITH_NAM=OFF`. Copy extracted `Sundrift.vst3` to `C:\Program Files\Common Files\VST3\`, then rescan VST3 in Ableton.

## First response to the owner

Confirm you have inspected this repo, summarise what actually builds/tests, highlight the biggest code-quality/sound risks, and propose your first small polish pass. Ask for audio reference or DI only if a specific listening test genuinely needs it. Be proactive about code improvements, but avoid claiming completion without verifying.
