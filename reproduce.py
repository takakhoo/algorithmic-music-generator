"""Generate deterministic modal score examples and machine-readable evidence."""
import hashlib
import json
from pathlib import Path
import subprocess


def run():
    root = Path(__file__).resolve().parent
    out = root / "results"
    out.mkdir(exist_ok=True)
    rows = []
    for mode, name in [(1, "ionian"), (2, "dorian"), (6, "aeolian")]:
        path = out / f"{name}.csd"
        args = [str(root / "exe"), "--mode", str(mode), "--seed", "7", "--output", str(path)]
        subprocess.run(args, check=True, capture_output=True, timeout=10)
        first = path.read_bytes()
        subprocess.run(args, check=True, capture_output=True, timeout=10)
        if first != path.read_bytes():
            raise AssertionError("Same seed changed score bytes")
        lines = first.decode().splitlines()
        events = [line.split() for line in lines if line.startswith(("i1 ", "i2 ", "i3 "))]
        rows.append(dict(mode=name, seed=7, bpm=90, section_seconds=4, sections=2, loops=1,
                         melody_events=sum(event[0] == "i1" for event in events),
                         drum_events=sum(event[0] != "i1" for event in events),
                         score_duration_seconds=8, reverb_tail_seconds=3,
                         sha256=hashlib.sha256(first).hexdigest()))
    (out / "metrics.json").write_text(json.dumps(rows, indent=2) + "\n")
    print(json.dumps(rows, indent=2))


if __name__ == "__main__":
    run()
