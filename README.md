# SUNDRIFT — psychedelic guitar tone suite

**Version 0.1.2 · JUCE VST3 project · optional NAM A2 playback**

Sundrift is an original guitar effects plugin inspired by the broad sound palettes of **BALTHVS**, **Khruangbin**, **Glass Beams** and **Yuuf**. It's made to complement Murmur: the same dark mechanical, Flip Fella-style look, tactile clock dials and split-flap preset readout, but a fresh sound engine and four instantly distinguishable band modes. No band has endorsed or participated in this project.

## What's new in v0.1.2: more bite without losing the clean shimmer

The real DI listening comparison highlighted that some lead sounds were a little too polished. This update adds a separate **EDGE** control immediately after the amplifier stage. It combines a restrained 2.1 kHz presence peak with blended, asymmetric soft clipping for more defined pick attack and touch-responsive upper-mid grit. **DRIVE** retains its existing purpose as the main preamp saturation control.

The artist bank presets have been rebalanced: Sunday Forecast and Golden Cumbia now have noticeable low-gain breakup, Glass Beams has the strongest push, Khruangbin retains controlled single-note articulation, and Yuuf stays relatively clean. EDGE works with optional NAM too, even though DRIVE is bypassed in neural amp mode. At EDGE=0 the new stage is neutral.

An extra audio-engine regression test checks that EDGE creates new harmonic content, rather than merely increasing the overall level. The same 12 presets were rerendered from the clean guitar DI, with no clipping or non-finite output.

**Compatibility:** The original 17 parameter IDs remain unchanged and an additional `edge` parameter is added. The project has not yet been compiled as a Windows VST3 or checked in Ableton; the C++ DSP engine and DI rendering tools have been compiled and exercised.

## What works in the source

- Four coloured band banks with **3 presets each** (12 presets total).
- A live, adjustable signal chain with input gain, simple compression, midrange focus, warm saturation, **a separate EDGE control for pick attack and slightly overdriven upper mids**, body and tone shaping, four-stage phaser, modulated chorus/wow, stereo echoes and a spacious reverb.
- Per-band last preset remembered for the current UI session. Preset and all 18 parameters are saved in the host's plugin state.
- JUCE VST3 plugin entry point and stereo/mono support, with a custom dark GUI.
- Optional **NeuralAmpModelerCore v0.5.2** (A2-capable fast path) to load a local `.nam` amp capture. The amp slot defaults to a built-in clean model, so Sundrift also works without downloads or captures.
- No allocation in the native effects engine's audio-processing function. The optional NAM model is loaded and prewarmed outside the audio callback; switching models keeps old state alive until plugin teardown.
- Offline engine tests and four synthetic audio demos (one for each band bank).
- Offline DI renderer for testing all 12 presets with the same recorded guitar phrase. The private DI and its renders are not packaged with the distributable source.

**Build status:** the dependency-free audio engine was compiled and tested on Linux, including 44.1, 48 and 96 kHz. The JUCE-based VST3 and end-to-end NAM loading **have not been compiled and tested here** because this environment cannot download the third-party build dependencies. The included Windows/GitHub build scripts are provided to create the plugin and surface any platform-specific errors. Do not mistake the project ZIP for a Windows installer.

## Get a Windows VST3 via GitHub Actions

1. Create a **new empty GitHub repository**, ideally private, named `Sundrift`.
2. Extract this archive, then upload its contents **including the hidden `.github` folder** to the repository root, or push with Git. Avoid uploading `tests/build` (the package excludes generated build files).
3. Open **Actions**, enable workflows if GitHub asks, and run **Build Windows VST3**. It also runs on push.
4. When the workflow succeeds, open the run and download the **Sundrift-Windows-x64-VST3** artefact.
5. Extract it, then copy `Sundrift.vst3` to `C:\Program Files\Common Files\VST3\`.
6. Open Ableton Live, choose **Settings → Plug-Ins**, enable system VST3 folders and rescan. Add Sundrift to an audio track with your guitar connected through your audio interface.

If GitHub Actions fails, copy the error log for the failing step; the project includes all editable source. This build downloads JUCE and NAM sources, so internet access is required.

### Building locally on Windows

Requires Visual Studio 2022 with **Desktop development with C++**, recent CMake and Git. Run from PowerShell:

```powershell
.\build-windows.ps1
```

For Visual Studio 2026, use `build-windows-2026.ps1`. Or build without NAM:

```powershell
cmake -S . -B build-win -G "Visual Studio 17 2022" -A x64 -DSUNDRIFT_WITH_NAM=OFF
cmake --build build-win --config Release --target Sundrift_VST3
```

## Sound banks

| Band | Accent | Preset | Intended use |
| --- | --- | --- | --- |
| BALTHVS | warm amber | **Sunday Forecast** | bright, hazy sustained lead; the project's primary reference |
| BALTHVS | warm amber | Golden Cumbia | buoyant clean guitar, warm picking |
| BALTHVS | warm amber | Lunar Coast | wide, slower psychedelic phrases |
| Khruangbin | sea green | Silk Lines | compressed and expressive melodic phrases |
| Khruangbin | sea green | Desert Dub | lively picking with restrained echo |
| Khruangbin | sea green | Texas Velvet | sweet, intimate near-dry clean tone |
| Glass Beams | violet | Mirage Pulse | dark, rhythmic phaser-forward sound |
| Glass Beams | violet | Temple Steps | hypnotic rhythmic phase and bite |
| Glass Beams | violet | Copper Serpent | singing darker sustain |
| Yuuf | sky blue | Afterglow | airy chorus and soft ambient trails |
| Yuuf | sky blue | Coastal Float | gentle modulated, glassy clean sound |
| Yuuf | sky blue | Late Drift | moodier echoes and long spatial tail |

Presets are **creative interpretations**, not recordings, cloned guitars, isolated stems or exact reproductions of artists' chains. Their response will depend strongly on your guitar pickups, DI level and playing style.

## Quick start for guitar

1. Put Sundrift **on a guitar audio track** in your DAW. Monitor through your audio interface at a safe level.
2. Select one of the four big **band** tabs. The accent strip, selected tab and split-flap preset will change colour.
3. Pick one of the three presets within that band. Start with **BALTHVS → Sunday Forecast**.
4. Turn **SQUEEZE** and **FOCUS** to shape the attack. **DRIVE** controls saturation of the built-in amp; **EDGE** adds upper-mid definition and a separate lightly asymmetric grit layer, including when a NAM model is loaded. **PHASE**, **CHORUS** and **WOBBLE** create movement; **TIME**, **REPEATS** and **LEVEL** control echo; **SPACE**, **DECAY** and **WIDTH** control the tail.
5. Use the output control to level-match when comparing presets, especially with higher input gain.

### NAM A2 captures

Select **NAM A2 model** under *Amp Engine*, press **LOAD .NAM**, and choose an A2-compatible `.nam` file. The plugin uses NeuralAmpModelerCore v0.5.2 with its A2 fast-path compile option. Because captures may represent **amp only, pedal only, or amp with speaker**, choose a capture suited to a clean amp or a clean amp+cab setup for these presets. No third-party captures are bundled.

**Important limitations in this release:**

- Only **mono-input, mono-output `.nam` captures** are supported. They run independently on left and right.
- Match the DAW sample rate to the capture's expected sample rate, typically 48 kHz. This first version does **not** resample NAM captures internally.
- NAM mode replaces built-in saturation, but compressor/focus/tone, EDGE and all spatial effects still apply. The INPUT knob drives the neural model, DRIVE is bypassed and EDGE remains usable after the capture.
- Loading a model happens on the interface thread and may briefly pause the UI; it never happens inside the audio callback.
- If no compatible NAM is loaded, built-in amplification continues instead. The GUI states this explicitly.
- No dedicated cabinet IR loader, preset import/export or tempo-synchronised delay yet.

## Offline tests (no JUCE required)

```sh
cmake -S tests -B tests/build -DCMAKE_BUILD_TYPE=Release
cmake --build tests/build -j
ctest --test-dir tests/build --output-on-failure
```

To render a made-up plucked-string test pattern through preset 0:

```sh
./tests/build/sundrift_demo 0 sunday-forecast-preview.wav
```

The `AudioPreviews` folder includes four synthetic WAVs, one from each band bank. They are engineering smoke tests, not guitar recordings or artist references.

### Render your own DI (44.1 kHz or above)

The `tools/render_user_di.py` helper applies **the same `ToneEngine.h` code as the VST3** to your own mono/stereo WAV file. It is deliberately separate from the plugin and requires Python with `numpy` and `soundfile` plus a C++17 compiler:

```sh
c++ -O3 -std=c++17 -fPIC -shared tools/render_di_bridge.cpp -o libsundrift_render.so
python tools/render_user_di.py clean-guitar.wav output_folder --all
```

The script makes 12 named, 24-bit WAV renders with a 4-second tail. These are preview-only **level-matched to -20 dBFS RMS**, with a -1 dBFS peak ceiling for easier comparison. It does not alter the actual VST3 preset settings or input file. This bridge is Linux-oriented; the Windows VST3 is built through the separate CMake workflow.

**11 November 2026 DI validation:** a 24-second real electric-guitar DI, 44.1 kHz, was successfully processed through **all 12 bank presets**. This also caught and prompted a fix for a rare wraparound bug in the fractional delay buffer. All preset renders were finite with peaks below clipping; the engine regression tests pass at 44.1/48/96 kHz. A short address-sanitised sequence including the previously problematic Glass Beams preset passed too. The supplied original DI and processed audio remain outside the distributable source archive.

## Technical architecture

```
Audio input
  ├─ Default: input gain → compressor → mid focus → built-in soft amp
  └─ NAM: input gain → NAM A2/A1 capture → compressor → mid focus
                                     ↓
                        EDGE upper-mid boost + grit → body/tone
           ↓
4-stage phaser → modulated chorus/tape wobble
           ↓
filtered stereo echoes → diffuser / multi-comb reverb → stereo width → output
```

Uses C++20 for the plugin and C++17 for the self-contained native DSP engine. The interface is custom-drawn using JUCE 8. The A2-accelerated model path comes from [NeuralAmpModelerCore](https://github.com/sdatkinson/NeuralAmpModelerCore) (MIT). JUCE requires adherence to [JUCE 8 licensing terms](https://juce.com/legal/juce-8-licence/) when building/distributing.

## What's next

The real DI tests and the v0.1.2 bite/edge revision are complete. The next step is to compile the Windows VST3, test it inside Ableton, and fine-tune the Sunday Forecast bank from listening comparisons to the intended reference. An actual A2 neural amp capture can be integrated after DAW validation; none is bundled.
