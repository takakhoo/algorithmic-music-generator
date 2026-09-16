# Algorithmic Music Generator in C and Csound

A command-line composition system that generates a Csound score from musical
parameters, then renders or plays the result. The C program combines controlled
randomness with scale, tempo, section, percussion, and time-stretching rules.

## Highlights

- Written in C with Csound as the synthesis engine
- Seven modal scale choices (Ionian through Locrian)
- Configurable tempo, section length, section count, and repetitions
- Generated melody, hi-hat, kick, and reverb parts
- Optional AIFF rendering and time-stretched output

## Prerequisites

- A C11-compatible compiler (`cc`, GCC, or Clang)
- [Csound](https://csound.com/download.html) available on `PATH`
- `make` (optional)

## Build and run

```bash
git clone https://github.com/takakhoo/algorithmic-music-generator.git
cd algorithmic-music-generator
make
./exe
```

If your platform uses a different output name:

```bash
cc -std=c11 -O2 -o music-generator algo_comp_v3.c
./music-generator
```

The program prompts for mode, BPM, section duration, number of sections, loop
count, and render mode. It writes `algo_comp_v3.csd`; Csound then synthesizes
the score.

## Timing model

For tempo \(B\) beats per minute and rhythmic division \(D\), note duration is

\[
t=\frac{60}{B D}.
\]

This keeps every generated layer synchronized when the tempo changes.

## Repository map

- `algo_comp_v3.c` — composition logic and command-line interface
- `algo_comp_v3.csd` — generated/reference Csound document
- `algo_comp_v3_stretched.csd` — time-stretched render variant
- `composed.aif` — example rendered output
- `Makefile` — build target and cleanup rules

## Verification

Verified on macOS with Clang on September 16, 2026: `make` completes without
warnings and the executable writes a valid Csound document. Audio rendering is
then delegated to `csound`; when it is not installed, the program exits with a
clear installation message instead of reporting a successful render.

## Scope

This is an experimental generator, not a digital audio workstation. Generated
files in the repository are examples; running the program may replace local
copies.
