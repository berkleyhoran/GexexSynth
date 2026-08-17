# Synthex

A gexex synthesizer: a 3-oscillator, FM-capable, DAW-automatable **VST3**
(also AU on macOS, and Standalone) — the instrument at the heart of gexex's
[browser synth toy](https://github.com/berkleyhoran/gexex), rebuilt native
and CPU-optimized, without the sequencer/drum-machine/beat-mangling features
a real DAW already covers.

Every knob is a proper host-automatable parameter — draw automation curves
for the filter, the LFOs, any effect, exactly like you would for a stock
plugin.

## Download

Grab the latest installer for your platform from the
**[Releases page](https://github.com/berkleyhoran/GexexSynth/releases)**:

- **Windows** — run the installer; it drops the VST3 into
  `Program Files\Common Files\VST3` (rescan plugins in your DAW if it
  doesn't show up right away).
- **macOS** — unzip, drag the `.vst3`/`.component` into your usual plugin
  folders. Unsigned/un-notarized for now, so Gatekeeper will ask you to
  right-click → Open the first time.
- **Linux** — untar and copy the `.vst3` into `~/.vst3`.

## Features

- **3 main oscillators** — saw/square/sine/triangle/pulse (band-limited via
  PolyBLEP), pulse-width, wavefolding, octave/semitone/fine detune, per-osc
  level and mute, plus **FM** (osc2/osc3 → osc1).
- A **sub-oscillator** (1 or 2 octaves down) and a **noise generator**
  (white/pink/brown), both mixed in as additional voice sources.
- **Dual filters** — lowpass/highpass/bandpass each, velocity → cutoff,
  routable in series or parallel.
- **2 independent LFOs** (tempo-syncable) plus a **mod envelope**, each
  routable to almost any other parameter.
- A full-ADSR **amp envelope** plus glide/portamento, mono or poly voicing,
  and an **arpeggiator**.
- A **reorderable effects chain** — drag to rearrange Drive, Bitcrusher,
  Chorus, Phaser/Flanger, Saturator, a Hilbert-transform **Frequency
  Shifter**, and a **3-band Multiband Compressor** — plus tempo-synced
  stereo Delay and an algorithmic Reverb as parallel sends.
- **42 factory presets** across six categories (Pads, Leads, Keys, Bass,
  Ambient, Textures), plus user presets, randomize/init/panic utilities.
- A custom "fruity aero" GUI — draggable-node envelope editors, animated
  reverb/delay visuals, a live LFO shape preview, per-oscillator
  oscilloscopes, a drag-to-reorder signal chain, and a parallax sky/cloud/
  grass background with a glowing master-output trace.

## Building

```
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

JUCE 8.0.15 is fetched automatically via `FetchContent` — no manual JUCE
setup required. See `CMakeLists.txt` for the `GEXEXSYNTH_VERSION` override
used by CI/the installer, and `.github/workflows/release.yml` for how
tagged releases build Windows/macOS/Linux installers.

### Building the Windows installer locally

Requires [Inno Setup 6](https://jrsoftware.org/isinfo.php):

```
ISCC.exe Installer\Synthex.iss
```

Produces `Installer\Output\SynthexSetup.exe`.

## Releasing a new version

Push a `v*` tag (e.g. `v1.0.0`) — `.github/workflows/release.yml` builds
Windows/macOS/Linux and publishes a GitHub Release with every installer
attached automatically. Use the workflow's manual "Run workflow" button to
test the build pipeline without publishing anything.

---

gexex © 2026
