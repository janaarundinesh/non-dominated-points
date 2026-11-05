"""Peak memory of the large-input measurements.

Reads plots/data/memory_and_large_n.csv (written by tools/large_benchmark) and
plots peak resident memory against N, one line per dimension. The million-point
runs live here because the quadratic baseline cannot produce them at all.
"""

import matplotlib.pyplot as plt
import pandas as pd

from benchmark_data import DATA_DIR, FIGURES_DIR


MEMORY_CSV = DATA_DIR / "memory_and_large_n.csv"

if not MEMORY_CSV.exists():
    raise SystemExit(
        f"Missing {MEMORY_CSV}: run tools/large_benchmark first "
        "(see README, section on the million-point measurements)."
    )

FIGURES_DIR.mkdir(exist_ok=True)

memory = pd.read_csv(MEMORY_CSV)

figure, panel = plt.subplots(figsize=(8, 5))

for dim in sorted(int(d) for d in memory["dimensions"].unique()):
    data = memory[memory["dimensions"] == dim].sort_values("num_points")

    panel.plot(
        data["num_points"],
        data["peak_rss_mb"],
        marker="o",
        markersize=5,
        linewidth=1.8,
        label=f"{dim}D",
    )

panel.set_xscale("log")
panel.set_xlabel("Number of Points")
panel.set_ylabel("Peak resident memory (MB)")
panel.set_title("Peak memory, input included")
panel.grid(True, which="both", linestyle=":", alpha=0.5)
panel.legend(title="Dimension", ncol=2)

figure.tight_layout()
figure.savefig(
    FIGURES_DIR / "memory.png",
    dpi=300,
    bbox_inches="tight",
    metadata={"Software": None},
)
plt.close(figure)

print("wrote memory.png")
