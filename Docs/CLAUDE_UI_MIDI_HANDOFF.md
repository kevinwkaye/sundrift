# Sundrift: Claude handoff for UI unification and MIDI foot control
**Prepared:** 11 November 2026  
**Current known release:** v0.1.3 Windows VST3  
**Repository:** https://github.com/kevinwkaye/sundrift  
**Priority:** Interface and live performance. **The user likes the current sounds. Do not redesign the DSP or retune presets without an explicit listening reason.**

## Brief from the owner

> To be fair, the sounds are good. I want the UI to be more in line with the other VSTs I've made, and I want Sundrift to work with my existing MIDI pedals.

Take over Sundrift as a separately maintained JUCE guitar VST3. Use the actual existing Flip Fella / Murmur interfaces and pedal implementation as references, not generic VST mock-ups. The main goal is for the plugin to look and behave like a member of the owner's other plugins, and work hands-free with their already configured MIDI footswitches and expression pedal in Ableton Live.

**Do not change Murmur or other plugins' repositories.** Work on a new Sundrift feature branch, e.g. `claude/ui-midi-polish`, and submit a PR after real tests.

## Reference repositories and files to inspect first

1. **Murmur:** `kevinwkaye/murmur`, branch `MurmurGPT`. Study `Source/PluginEditor.h/.cpp`, `Source/PedalControl.h/.cpp`, `Source/Keeper.h/.cpp` and especially `docs/foot-control-plan.md`. Murmur is private: if you cannot access it, say so and ask the owner to grant repo access or supply the specific files. Do not silently invent how it works.
2. **Jetstream:** `kevinwkaye/Jetstream`, `Source/UI/Family.h`. An existing implementation of the Flip Fella family visual system, with mechanical split-flap cards, clock-like dials, pills, readable condensed typography and the near-black/cream palette.
3. **Speckle:** `kevinwkaye/Speckle`, `Source/PedalControl.*`, and its editor. Also check `kevinwkaye/prism--patch` for shared pedal defaults and Learn conventions.
4. **Sundrift:** `Source/PluginEditor.*`, `Source/PluginProcessor.*`, `Source/ToneEngine.h`, `Source/Presets.h`, `tests/host_smoke.cpp`, `tests/test_engine.cpp`, and `.github/workflows/build-windows.yml`.
5. Other existing plugins may be useful for proportions and layout, but prefer the actual shared components rather than merging codebases.

The owner has **not** supplied a confirmed MIDI pedal brand/model or hardware-specific settings in this conversation. Existing code recognises some device-name patterns, but that does **not** establish the owner's hardware model. Support generic MIDI CC and learnable mappings.

## Non-negotiable goals

- **Sound:** protect the existing 4 artist banks and 12 presets; keep all current parameter IDs, ranges, host automation and state recall. DRIVE and EDGE must remain independently adjustable. Do not shift preset levels, harmonics or modulation just for a UI revision.
- **UI:** recognisably the Flip Fella family, not a generic pedalboard or a wholly new visual language. Make band and preset selection easy to distinguish from footswitch assignment. Keep Sundrift's band colour accents as secondary highlights.
- **MIDI:** the current Sundrift `acceptsMidi()` returns `false`, and `processBlock()` ignores its `juce::MidiBuffer`. Implement genuine MIDI support, not merely buttons that look assignable. It must work in a realistic Ableton Live audio-effects workflow, with a documented path for MIDI input.
- **Reliability:** Sundrift v0.1.2 crashed Ableton because the GUI called `setSize()` before its band buttons existed; fixed in v0.1.3. **Do not regress this fix.** GUI components must all exist before layout. Run lifecycle, DSP and pluginval GUI/audio/automation tests on the new Windows build. Confirm Ableton testing separately; an Actions compile does not prove Ableton compatibility.
- **Other plugins:** do not steal, overwrite or conflict with Murmur's shared MIDI mapping or live pedal device. Document and test multi-plugin/device contention.

## Shared Flip Fella pedal configuration already implemented elsewhere

From Murmur's `Source/PedalControl.h/.cpp` and `docs/foot-control-plan.md`:

- Default footswitches **A, B, C, D:** momentary MIDI CC **20, 21, 22, 23**, respectively, all on **channel 16**.
- Default expression pedal: **CC 11, channel 16**.
- Actual mappings are configurable using MIDI Learn, so **read the persisted settings**, do not assume the defaults describe the current physical hardware.
- Shared Windows settings location, via `PedalSettings::file()`: `%APPDATA%\Kevin Kaye\Flip Fella Pedals.xml` (resolved with JUCE's `userApplicationDataDirectory`).
- Persisted settings include footswitch channel/CC, expression channel/CC, heel/toe calibration (including reversed and short travel), expression curve, selected input device, optional original-pedal map channel and optional Keeper switch.
- Gesture logic: **hold 0.45 s**, **combo window 0.12 s**, **combo hold 1.0 s**. Murmur's updated `FootswitchLogic` recognises A+B, B+C and C+D combinations.
- Murmur foot conventions include C tap for tap tempo, D tap for next preset and D hold for previous preset. C+D tap has been used to hand control to the next plugin.
- The existing `DirectMidiInput` opens a device and routes input to one active sink, including MIDI capture without manual DAW MIDI routing. However, **separate Windows VST3 modules may not both open the same device simultaneously**; Murmur's own plan documents this limitation. Copying `DirectMidiInput` blindly into Sundrift could break Murmur/Prism Patch. Verify coexistence and offer a reliable Ableton MIDI-routing fallback.

Treat the shared settings format as a stable contract. Avoid rewriting unrelated attributes, racing to save it, or silently resetting values. If introducing additional Sundrift-only settings, store them separately or in a backward-compatible namespace without corrupting the shared configuration.

## UI redesign direction

Build a visual audit based on actual screenshots or the rendered editors of Murmur, Jetstream and at least one other Flip Fella VST. Describe the existing design tokens and reusable patterns before coding.

Use the family's near-black background, cream ink, tactile clock-shaped rotary knobs with readable labels and values, subtle grooves and divisions, mechanical split-flap readouts, restrained flip animations and compact pill buttons. Borrow properly licensed typefaces/assets already in the owner's code where feasible. Avoid excessive gradients, neon, faux wood, photorealistic pedals or large featureless knob grids.

**Suggested layout:**
- **PERFORM** page: clear band and preset display; four colour-coded artist bank choices; a small number of expressive hero controls such as DRIVE, EDGE, FOCUS, MODULATION, ECHO and SPACE; live status such as selected preset, expression amount, pedal ownership and tempo. All 18 existing controls must remain reachable.
- **DEEP / TONE** page: remaining parameters and NAM capture selection, with good grouping and exact value readouts.
- **PEDALS** page: four labelled flip-card representations of A/B/C/D, each showing *MIDI source, tap action, hold action, MIDI Learn* and live press feedback; combinations; expression target/calibration/curve; MIDI device routing/ownership status; instructions in ordinary words. On-screen pedal buttons must run the **same action dispatch** as real MIDI events.
- Make the layout responsive to Windows scaling and DAW window resizing. No clipped controls. Support mouse-only operation and keyboard-accessible navigation where feasible. Consider high contrast, tooltips and readable focus indicators.
- Keep band accents: BALTHVS amber `#E9AD54`, Khruangbin sea green `#72BCA9`, Glass Beams violet `#AB99DC`, Yuuf sky blue `#85BEE0`.

## MIDI behaviour and suggested Sundrift defaults

Design one **source-independent action dispatcher** called by MIDI, on-screen pedal controls and host automation where appropriate. Actions should be configurable, with clear tap, hold and combo semantics. A **proposed starting set** for the user to audition, not a hard-coded irreversible hardware map:

| Gesture | Proposed action | Rationale |
| --- | --- | --- |
| A tap | Effect wet/bypass toggle, preserving tails where sensible | Hands-free effect on/off |
| B tap | Toggle echo send or delay effect | Dub-style guitar performance |
| C tap | Tap tempo (two or more presses) | Familiar Murmur convention |
| D tap | Next preset within current band | Familiar Murmur convention |
| D hold | Previous preset | Familiar Murmur convention |
| B+C hold | Next artist band, preserving the last selected scene per band | Similar to Murmur mode change |
| C+D tap | Hand pedal focus to the next Flip Fella plugin when supported | Avoid two loaded plugins reacting together |
| Expression | Learnable per-preset target, heel/toe calibration and curve | FOCUS / PHASE / DELAY mix / SPACE / DRIVE / EDGE possibilities |

Any new wet/bypass or tempo parameters should be implemented without breaking the old audio settings, and should preserve delay/reverb tails and avoid clicks. Offer configurable footswitch actions and a safe no-action option for unused holds/combos. Avoid clashing with another plugin's C+D, Keeper, or live control role. Keep simultaneous MIDI and GUI control consistent.

Implement MIDI Learn for switches and expression; recognise momentary press/release CC values (including low/high device-specific thresholds), debounce press noise, avoid interpreting an expression sweep as a switch, and support channel filtering. Test Program Change messages only if explicitly implemented and documented. Do not indiscriminately consume all MIDI; retain unrelated messages as appropriate for the host.

### Ableton and multiple-plugin routing

Explicitly test **both** paths, and document limitations:
1. **Host-supplied MIDI:** correct VST3 input support, with workable Ableton MIDI track routing to an audio effect or supported controller configuration. Provide exact routing steps, not just `acceptsMidi = true`.
2. **Direct device MIDI:** only if it can coexist safely with Murmur/Prism and their shared MIDI settings. One exclusive Windows device handle per plugin is not acceptable as an undocumented default. Consider a shared external bridge/controller owner or a safe 'Use Ableton routing' mode. Do not silently change the user's other plugins.

If multi-plugin direct control cannot be delivered reliably without a shared service, ship the reliable DAW-routed approach, expose the limitation, and propose a tested shared bridge as a separate phase. Never silently intercept MIDI from another plugin.

## Real-time and state safety

- Never mutate JUCE GUI components from audio/MIDI callback threads. Queue or atomically publish events to a safe dispatcher; keep the audio callback allocation-free, lock-free and free of filesystem work.
- Don't call `setValueNotifyingHost` from an unsafe callback thread. Respect host automation gestures and parameter normalisation.
- MIDI continuous controls must be smoothed to avoid zipper noise and sudden echo/drive jumps.
- Persist per-preset expression target/range plus custom foot actions without losing the original `ampMode`, NAM path or saved patch parameters. Separate global pedal settings from Ableton project state.
- Verify multiple instances, live focus ownership, instances opening/closing, MIDI hotplug/disconnect, and loaded NAM captures.

## Tests and definition of done

1. Build **Windows x64 VST3** with optional NAM A2 engine enabled and publish the real ZIP from `kevinwkaye/sundrift`.
2. Run standalone DSP regression tests; JUCE processor **and editor startup** smoke tests; pluginval with the GUI tests enabled; and a MIDI test suite for CC press/release, holds, combo precedence, expression calibration, Learn, channel filtering, preset/bank switching, state save/reload and no double triggers.
3. Verify the old v0.1.3 user presets and host parameter automation still load, and that simply opening/closing the interface cannot crash the host.
4. Provide a screenshot comparison with existing Flip Fella plugins, and show a working PEDALS configuration page with live status and mapped actions.
5. Test in Ableton Live on Windows, or explicitly mark that real DAW test pending; include a reproducible manual test plan for a fresh empty session, with a copy of a project before testing older sets.
6. Test two loaded Sundrift instances and Sundrift plus Murmur, including the shared MIDI source and who is currently in charge.
7. Document installation, rollback to v0.1.3, CC Learn, Ableton MIDI setup, and a concise user-facing reference for the A/B/C/D actions.

**Do not deliver a 'stable' installer merely because it compiles.** The v0.1.2 Ableton crash makes host validation essential. Show proof of what passed and what remains untested.

## Immediate task sequence for Claude

1. Read this handoff, `CLAUDE.md`, the latest Actions logs and the reference source files.
2. Summarise the specific differences between Sundrift's current GUI and the family's implemented visual system, and identify MIDI integration risks.
3. Create a `claude/ui-midi-polish` feature branch in Sundrift.
4. Implement the *smallest safe vertical slice*: host MIDI reception + reusable pedal event/action mapping + expression Learn + GUI foot status, covered by unit tests.
5. Polish the UI using the real shared family patterns while retaining the current sound engine and band/preset definitions.
6. Build, test, provide screenshots and a checked Windows VST3. Create a PR with a clear 'tested / not tested' table.
7. If the exact pedal hardware behaviour matters, ask the owner for a MIDI monitor capture, device name or actual settings XML, rather than making one up.

**The user's main ask is not 'make it sound different'. It is 'make Sundrift feel like the rest of my VSTs and make my current foot pedals work'.**
