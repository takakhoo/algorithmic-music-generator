/* Taka Khoo: deterministic modal composition for Csound.
 * Score generation is separate from rendering: never invoke a shell or audio
 * device implicitly. The original exploratory implementation is in legacy/.
 */
#include <errno.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int mode, bpm, sections, loops;
    double seconds, stretch;
    uint32_t seed;
    const char *output;
} Config;

static uint32_t next_random(uint32_t *state) {
    /* Fixed xorshift32 algorithm, unlike platform-dependent C rand(). */
    uint32_t x = *state;
    x ^= x << 13; x ^= x >> 17; x ^= x << 5;
    return *state = x;
}

static double number(const char *text) {
    char *end;
    errno = 0;
    double value = strtod(text, &end);
    if (errno || end == text || *end || !isfinite(value)) {
        fprintf(stderr, "Invalid numeric value: %s\n", text);
        exit(2);
    }
    return value;
}

static int integer(const char *text, int low, int high) {
    double value = number(text);
    if (value < low || value > high || floor(value) != value) {
        fprintf(stderr, "Expected integer in [%d,%d]: %s\n", low, high, text);
        exit(2);
    }
    return (int)value;
}

static double frequency(int mode, int degree) {
    static const int modes[7][8] = {
        {0,2,4,5,7,9,11,12}, {0,2,3,5,7,9,10,12},
        {0,1,3,5,7,8,10,12}, {0,2,4,6,7,9,11,12},
        {0,2,4,5,7,9,10,12}, {0,2,3,5,7,8,10,12},
        {0,1,3,5,6,8,10,12}
    };
    return 440.0 * pow(2.0, (60 + modes[mode - 1][degree] - 69) / 12.0);
}

static void usage(void) {
    puts("music-generator [--mode 1..7] [--bpm 20..200] [--seconds 0.1..60]\n"
         "  [--sections 1..20] [--loops 1..10] [--stretch 0.25..4]\n"
         "  [--seed 1..2147483647] [--output generated.csd]\n"
         "Defaults: Ionian, 90 BPM, 4 seconds, 2 sections, 1 loop, seed 7.\n"
         "Writes a score only. Render separately with csound -W -o demo.wav generated.csd.");
}

static int write_score(const Config *c) {
    FILE *out = fopen(c->output, "w");
    if (!out) { perror(c->output); return 1; }
    fprintf(out, "; seed=%u mode=%d bpm=%d sections=%d loops=%d stretch=%.6f\n",
            c->seed, c->mode, c->bpm, c->sections, c->loops, c->stretch);
    fputs("<CsoundSynthesizer>\n<CsOptions>\n-d -m0\n</CsOptions>\n<CsInstruments>\n"
          "sr = 44100\nksmps = 32\nnchnls = 2\n0dbfs = 1\n", out);
    fprintf(out, "seed %u\n", c->seed);
    fputs("gaL init 0\ngaR init 0\n"
          "instr 1\n"
          "aenv linen 0.12, 0.005, p3, 0.02\n"
          "asig oscili aenv, p4, 1\n"
          "outs asig, asig\n"
          "gaL = gaL + asig * 0.25\ngaR = gaR + asig * 0.25\nendin\n"
          "instr 2\n"
          "anoise random -0.07, 0.07\n"
          "afilt butterhp anoise, 6000\n"
          "aenv expon 1, p3, 0.001\n"
          "outs afilt * aenv, afilt * aenv\nendin\n"
          "instr 3\n"
          "kfreq expon 130, p3, 45\n"
          "aenv expon 0.2, p3, 0.001\n"
          "asig oscili aenv, kfreq, 2\nouts asig, asig\nendin\n"
          "instr 99\n"
          "aL, aR reverbsc gaL, gaR, 0.75, 10000\n"
          "outs aL, aR\nclear gaL, gaR\nendin\n"
          "</CsInstruments>\n<CsScore>\n"
          "f 1 0 8192 10 1 0.25 0.1\nf 2 0 8192 10 1\n", out);
    uint32_t state = c->seed;
    const double rhythms[] = {1, 1, 0.5, 0.5, 0.25, 0.25, 2};
    unsigned melody_events = 0, drum_events = 0;
    for (int section = 0; section < c->sections; ++section) {
        uint32_t section_seed = state;
        for (int loop = 0; loop < c->loops; ++loop) {
            /* Repeat the same rhythm and pitches within a section. */
            uint32_t local = section_seed;
            double start = (section * c->loops + loop) * c->seconds;
            double t = 0;
            int previous = 0;
            while (t < c->seconds - 1e-9) {
                int degree;
                do { degree = (int)(next_random(&local) % 8); }
                while (abs(degree - previous) > 5);
                double duration = (60.0 / c->bpm) * rhythms[next_random(&local) % 7];
                if (t == 0 || t + duration >= c->seconds) degree = 0;
                duration = fmin(duration, c->seconds - t);
                fprintf(out, "i1 %.9f %.9f %.9f\n", (start + t) * c->stretch,
                        duration * c->stretch, frequency(c->mode, degree));
                ++melody_events;
                t += duration;
                previous = degree;
            }
            if (section > 0) {
                for (t = 0; t < c->seconds - 1e-9; t += 30.0 / c->bpm) {
                    fprintf(out, "i2 %.9f %.9f\n", (start + t) * c->stretch,
                            fmin(0.08, c->seconds - t) * c->stretch);
                    ++drum_events;
                }
                for (t = 0; t < c->seconds - 1e-9; t += 60.0 / c->bpm) {
                    fprintf(out, "i3 %.9f %.9f\n", (start + t) * c->stretch,
                            fmin(0.2, c->seconds - t) * c->stretch);
                    ++drum_events;
                }
            }
            state = local;
        }
    }
    double duration = c->seconds * c->sections * c->loops * c->stretch;
    fprintf(out, "i99 0 %.9f\ne\n</CsScore>\n", duration + 3.0);
    int failed = ferror(out);
    if (fclose(out) != 0) failed = 1;
    if (failed) { fprintf(stderr, "Could not finish writing score\n"); return 1; }
    printf("Wrote %s: %u melody events, %u drum events, %.3f seconds + 3-second reverb tail\n",
           c->output, melody_events, drum_events, duration);
    return 0;
}

int main(int argc, char **argv) {
    Config c = {1, 90, 2, 1, 4.0, 1.0, 7, "generated.csd"};
    for (int i = 1; i < argc; ++i) {
        const char *option = argv[i];
        if (!strcmp(option, "--help")) { usage(); return 0; }
        if (++i == argc) { fprintf(stderr, "Missing value for %s\n", option); return 2; }
        const char *value = argv[i];
        if (!strcmp(option, "--mode")) c.mode = integer(value, 1, 7);
        else if (!strcmp(option, "--bpm")) c.bpm = integer(value, 20, 200);
        else if (!strcmp(option, "--sections")) c.sections = integer(value, 1, 20);
        else if (!strcmp(option, "--loops")) c.loops = integer(value, 1, 10);
        else if (!strcmp(option, "--seed")) c.seed = (uint32_t)integer(value, 1, 2147483647);
        else if (!strcmp(option, "--seconds")) c.seconds = number(value);
        else if (!strcmp(option, "--stretch")) c.stretch = number(value);
        else if (!strcmp(option, "--output")) c.output = value;
        else { fprintf(stderr, "Unknown option: %s\n", option); return 2; }
    }
    if (c.seconds < 0.1 || c.seconds > 60 || c.stretch < 0.25 || c.stretch > 4) {
        fputs("Seconds must be in [0.1,60]; stretch in [0.25,4]\n", stderr);
        return 2;
    }
    return write_score(&c);
}
