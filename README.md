# Algorithmic composition in C and Csound

A deterministic modal melody generator with seven scales, tempo-aware rhythms,
repeatable sections, percussion, stereo synthesis and reverb. C writes the score;
Csound renders it. Generating a score never starts audio playback or invokes a shell.

## Build and hear it

```bash
make
./exe --mode 2 --bpm 90 --seconds 4 --sections 2 --loops 2 --seed 7 --output generated.csd
csound -W -s -o demo.wav generated.csd
```

Open `demo.wav` in an audio player. To play through Csound directly, explicitly
run `csound -odac generated.csd`. Only rendering/playback requires
[Csound](https://csound.com/download.html); score generation requires a C11 compiler
and the standard math library. No Python packages are needed for the tests.

### Controls

| Flag | Meaning | Default |
|---|---|---:|
| `--mode` | 1 Ionian, 2 Dorian, 3 Phrygian, 4 Lydian, 5 Mixolydian, 6 Aeolian, 7 Locrian | 1 |
| `--bpm` | Tempo, 20–200 | 90 |
| `--seconds` | Length of one section before stretching, 0.1–60 | 4 |
| `--sections` | Newly generated sections, 1–20 | 2 |
| `--loops` | Exact phrase repeats per section, 1–10 | 1 |
| `--stretch` | Multiply score times/durations, 0.25–4; leave pitches unchanged | 1 |
| `--seed` | Fixed nonzero integer random seed | 7 |
| `--output` | Destination score (overwritten if it already exists) | generated.csd |

Total musical duration is `seconds × sections × loops × stretch`, followed by
a three-second reverb tail. Percussion enters after the first section. Stretch
is **symbolic score-time scaling**, not a waveform time-stretching algorithm.

## Reproduce the examples

```bash
python3 -m unittest -v test_generator
python3 reproduce.py
csound -W -s -o results/dorian.wav results/dorian.csd
```

Committed scores: [Ionian](results/ionian.csd), [Dorian](results/dorian.csd),
[Aeolian](results/aeolian.csd). The default seed produces 14 melody events and
18 percussion events across eight seconds, plus the reverb tail. Each mode uses
the same random rhythm/scale degrees for a controlled harmonic comparison.
[Metrics and score hashes](results/metrics.json) record two byte-identical runs.

CI compiles with address/undefined-behavior sanitizers, runs the tests, renders
real audio with Csound, and uploads `rendered-music` containing a WAV and scores.
The audio test checks stereo format, duration, non-silence and absence of clipping.
Without local Csound, that one integration test is explicitly skipped; the five
score/CLI tests still run.

## What changed

- Removed fixed 256-element event buffers that could overrun on long sections.
- Replaced unchecked interactive input with validated, scriptable arguments.
- Added a specified xorshift32 RNG so score sequences do not depend on the
  platform's `rand()` implementation. Numeric text may still differ at the last
  decimal across math libraries.
- Implemented the previously inactive loop control; final events are clipped
  to exact section boundaries, without accumulating timing drift.
- Put orchestra globals inside `CsInstruments`, use stereo output consistently,
  and separate generation from rendering. Rendering errors can no longer be
  hidden behind a successful generator exit code.
- Preserved the exploratory source in [`legacy/`](legacy/algo_comp_v3.c).
  It is historical, has known defects, and is not built or executed by the tests.
  Existing root-level scores and `composed.aif` are historical examples too.

The composition rules are deliberately simple, not a learned music model.
Sound quality is subjective; reproducible score structure is not proof of
musical quality. Reference: [Csound unified document format](https://csound.com/docs/manual/CommandUnifile.html).
