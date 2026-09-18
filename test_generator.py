"""Black-box CLI and score invariants, with optional real Csound rendering."""
import math
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest
import wave
import array

ROOT = Path(__file__).resolve().parent
EXE = ROOT / "exe"


def generate(directory, *arguments):
    path = Path(directory) / "score.csd"
    subprocess.run([str(EXE), "--output", str(path), *arguments], check=True, capture_output=True, timeout=30)
    text = path.read_text()
    events = [[float(value) for value in line.split()[1:]]
              for line in text.splitlines() if line.startswith("i1 ")]
    return text, events


class GeneratorTests(unittest.TestCase):
    def test_reproducible_seed_and_no_implicit_audio(self):
        with tempfile.TemporaryDirectory() as tmp:
            first, _ = generate(tmp, "--seed", "19")
            second, _ = generate(tmp, "--seed", "19")
            other, _ = generate(tmp, "--seed", "41")
            self.assertEqual(first, second)
            self.assertNotEqual(first, other)
            self.assertEqual(set(p.name for p in Path(tmp).iterdir()), {"score.csd"})

    def test_modes_and_boundaries(self):
        modes = [{0,2,4,5,7,9,11}, {0,2,3,5,7,9,10}, {0,1,3,5,7,8,10},
                 {0,2,4,6,7,9,11}, {0,2,4,5,7,9,10}, {0,2,3,5,7,8,10}, {0,1,3,5,6,8,10}]
        with tempfile.TemporaryDirectory() as tmp:
            for mode, pitches in enumerate(modes, 1):
                _, events = generate(tmp, "--mode", str(mode), "--seconds", "9", "--sections", "3")
                for start, duration, hz in events:
                    self.assertGreater(duration, 0)
                    self.assertLessEqual(start + duration, 27 + 1e-8)
                    midi = 69 + 12 * math.log2(hz / 440)
                    self.assertIn(round(midi) % 12, pitches)
                    self.assertAlmostEqual(midi, round(midi), places=7)
                self.assertAlmostEqual(events[-1][0] + events[-1][1], 27)

    def test_loops_repeat_and_stretch_preserves_pitch(self):
        with tempfile.TemporaryDirectory() as tmp:
            _, plain = generate(tmp, "--sections", "1", "--loops", "2")
            half = len(plain) // 2
            self.assertEqual([e[1:] for e in plain[:half]], [e[1:] for e in plain[half:]])
            _, stretched = generate(tmp, "--sections", "1", "--loops", "2", "--stretch", "2")
            for a, b in zip(plain, stretched):
                self.assertAlmostEqual(a[0] * 2, b[0], places=7)
                self.assertAlmostEqual(a[1] * 2, b[1], places=7)
                self.assertEqual(a[2], b[2])

    def test_long_score_exceeds_old_fixed_buffer(self):
        with tempfile.TemporaryDirectory() as tmp:
            _, events = generate(tmp, "--seconds", "60", "--bpm", "200", "--sections", "2")
            self.assertGreater(len(events), 256)

    def test_invalid_inputs_fail_without_writing(self):
        cases = [("--bpm", "nan"), ("--bpm", "1.5"), ("--seconds", "-1"),
                 ("--seed", "0"), ("--mode", "8"), ("--loops", "9999999999999"),
                 ("--stretch", "inf"), ("--unknown", "7"), ("--mode",)]
        with tempfile.TemporaryDirectory() as tmp:
            for args in cases:
                result = subprocess.run([str(EXE), *args], cwd=tmp, capture_output=True, timeout=5)
                self.assertNotEqual(result.returncode, 0)
            self.assertEqual(list(Path(tmp).iterdir()), [])

    @unittest.skipUnless(shutil.which("csound"), "Csound unavailable; CI renders actual audio")
    def test_actual_csound_render(self):
        with tempfile.TemporaryDirectory() as tmp:
            generate(tmp)
            output = Path(tmp) / "render.wav"
            subprocess.run(["csound", "-W", "-s", "-o", str(output), str(Path(tmp) / "score.csd")],
                           check=True, capture_output=True, timeout=30)
            with wave.open(str(output)) as wav:
                self.assertEqual(wav.getnchannels(), 2)
                self.assertEqual(wav.getframerate(), 44100)
                self.assertAlmostEqual(wav.getnframes() / wav.getframerate(), 11, delta=0.01)
                samples = array.array("h", wav.readframes(wav.getnframes()))
            self.assertGreater(max(map(abs, samples)), 100)
            self.assertLess(max(map(abs, samples)), 32767)


if __name__ == "__main__":
    unittest.main()
