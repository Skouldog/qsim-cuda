"""Plot Google Benchmark results for the single-qubit gate sweep."""

# ./.venv/bin/python scripts/plot_benchmarks.py
import json
import math
import re

import matplotlib.pyplot as plt

RESULTS = "benchmarks/results/latest.json"
OUTPUT = "docs/single-qubit-sweep.png"
BYTES_PER_AMPLITUDE = 16  # std::complex<double>

with open(RESULTS) as f:
    data = json.load(f)

# Keep only the aggregated means; skip raw per-repetition rows.
points = []
for b in data["benchmarks"]:
    if b.get("aggregate_name") != "mean":
        continue
    match = re.search(r"qubits:(\d+)", b["run_name"])
    if match:
        points.append(
            (int(match.group(1)), b["real_time"], b["bytes_per_second"] / 1e9)
        )
points.sort()

qubits = [p[0] for p in points]
times = [p[1] for p in points]
bandwidth = [p[2] for p in points]

# Qubit count at which the statevector outgrows each cache level.
boundaries = [
    (c["level"], math.log2(c["size"] / BYTES_PER_AMPLITUDE))
    for c in data["context"]["caches"]
    if c["type"] in ("Data", "Unified")
]

fig, (ax_time, ax_bw) = plt.subplots(1, 2, figsize=(11, 4))

ax_time.semilogy(qubits, times, marker="o")
ax_time.set_xlabel("qubits")
ax_time.set_ylabel("time per gate (ns)")
ax_time.set_title("Time")

ax_bw.plot(qubits, bandwidth, marker="o")
ax_bw.set_xlabel("qubits")
ax_bw.set_ylabel("GB/s")
ax_bw.set_ylim(bottom=0)
ax_bw.set_title("Achieved bandwidth")

for ax in (ax_time, ax_bw):
    ax.grid(True, which="both", alpha=0.3)
    for level, edge in boundaries:
        ax.axvline(edge, color="grey", linestyle="--", alpha=0.5)
        ax.text(edge, ax.get_ylim()[1], f"L{level}", ha="center", va="bottom",
                fontsize=8, color="grey")

fig.suptitle(f"Single-qubit gate — {data['context']['host_name']}")
fig.tight_layout()
fig.savefig(OUTPUT, dpi=150)
print(f"wrote {OUTPUT}")
