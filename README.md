# Gexex Synth

A 3-oscillator, FM-capable, DAW-automatable **VST3** synthesizer (also AU on
macOS, and Standalone) — the instrument at the heart of gexex's
[browser synth toy](https://github.com/berkleyhoran/gexex), rebuilt native
and CPU-optimized, without the sequencer/drum-machine/beat-mangling features
a real DAW already covers.

Every knob is a proper host-automatable parameter — draw automation curves
for the filter, the LFO, any effect, exactly like you would for a stock
plugin.

## Features

- **3 oscillators** — saw/square/sine/triangle/pulse (band-limited via
  PolyBLEP), pulse-width, wavefolding, octave/semitone/fine detune, per-osc
  level and mute, plus **FM** (osc2/osc3 → osc1).
- A resonant multi-mode **filter** (lowpass/highpass/bandpass, velocity → cutoff).
- One routable **LFO** (tempo-syncable) targeting almost any other knob.
- A full-ADSR **envelope** plus glide/portamento, mono or poly voicing, and
  an **arpeggiator**.
- An effects rack: bitcrusher, drive, tempo-synced stereo delay, algorithmic
  reverb, chorus, phaser/flanger, and a 4-curve master saturator.
- Factory + user **presets**, randomize/init/panic utilities.

## Building

```
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

JUCE 8.0.15 is fetched automatically via `FetchContent` — no manual JUCE
setup required. See `CMakeLists.txt` for the `GEXEXSYNTH_VERSION` override
used by CI/the installer, and `.github/workflows/release.yml` for how
tagged releases build Windows/macOS/Linux installers.

## Status

Early scaffolding (Phase 0 of the build plan) — builds and loads as a
silent instrument. DSP, GUI, presets, and the installer land in later
phases.

---

gexex © 2026
