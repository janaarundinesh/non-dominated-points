"""Where the time goes inside the marry-go-round MPI program.

One dataset is enough to show the shape of the breakdown: the critical rank's
wall time split into local brute force, cross-filter, communication and barrier
wait, in seconds on the left and as a share of the wall time on the right.

The components come from the rank that determined the wall time, so they add up
to the total. Run scripts/mpi_timing_sweep.sh first; it writes the CSV read
here.
"""

from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

BASE_DIR = Path(__file__).resolve().parent
PROJECT_DIR = BASE_DIR.parent
FIGURES_DIR = BASE_DIR / "figures"

CANDIDATES = [
    BASE_DIR / "data" / "mpi_marry_go_round_timing.csv",
    PROJECT_DIR / "results" / "mpi_marry_go_round_timing.csv",
]

COMPONENTS = [
    ("local_s", "Local brute force", "tab:blue"),
    ("crossfilter_s", "Cross-filter", "tab:orange"),
    ("communication_s", "Communication", "tab:red"),
    ("barrier_s", "Barrier wait", "tab:purple"),
]


def load():
    for path in CANDIDATES:
        if path.exists():
            return pd.read_csv(path), path

    raise SystemExit(
        "Missing mpi_marry_go_round_timing.csv: run "
        "scripts/mpi_timing_sweep.sh from a build directory first."
    )


frame, source = load()

FIGURES_DIR.mkdir(exist_ok=True)

median = frame.groupby(
    ["dimensions", "num_points", "num_processes"], as_index=False
).median(numeric_only=True)

# The largest measured dataset carries every effect the breakdown is meant to
# show; the smaller ones told the same story in more panels.
datasets = median[["dimensions", "num_points"]].drop_duplicates()
largest = datasets.sort_values(["dimensions", "num_points"]).iloc[-1]

data = median[
    (median["dimensions"] == largest["dimensions"])
    & (median["num_points"] == largest["num_points"])
].sort_values("num_processes")

dimension = int(largest["dimensions"])
points = int(largest["num_points"])

figure, (absolute, shares) = plt.subplots(1, 2, figsize=(12, 4.8))

for field, label, colour in COMPONENTS:
    absolute.plot(
        data["num_processes"],
        data[field],
        marker="o",
        markersize=5,
        linewidth=1.8,
        color=colour,
        label=label,
    )

absolute.plot(
    data["num_processes"],
    data["total_s"],
    marker="s",
    linewidth=1.4,
    linestyle="--",
    color="black",
    label="Total (critical rank)",
)

absolute.set_xscale("log", base=2)
absolute.set_xticks(data["num_processes"].tolist())
absolute.set_xticklabels([int(p) for p in data["num_processes"]])
absolute.set_yscale("log")
absolute.set_xlabel("MPI processes")
absolute.set_ylabel("Seconds")
absolute.set_title(f"{dimension}D, N = {points:,}")
absolute.grid(True, which="both", linestyle=":", alpha=0.5)
absolute.legend(fontsize=8)

positions = np.arange(len(data))
bottom = np.zeros(len(data))

# Normalised by the sum of the components rather than by the total: medians of
# the parts need not add up to the median of the whole.
component_sum = data[[field for field, _, _ in COMPONENTS]].sum(axis=1)

for field, label, colour in COMPONENTS:
    values = (data[field] / component_sum).to_numpy()
    shares.bar(
        positions,
        values,
        bottom=bottom,
        color=colour,
        label=label,
        width=0.65,
    )
    bottom += values

shares.set_xticks(positions)
shares.set_xticklabels([int(p) for p in data["num_processes"]])
shares.set_xlabel("MPI processes")
shares.set_ylabel("Share of wall time")
shares.set_ylim(0, 1.05)
shares.set_title(f"{dimension}D, N = {points:,}")
shares.grid(True, axis="y", linestyle=":", alpha=0.5)

figure.suptitle("Marry-go-round breakdown: the critical rank's wall time")

figure.tight_layout()
figure.savefig(
    FIGURES_DIR / "mpi_time_breakdown.png",
    dpi=300,
    bbox_inches="tight",
    metadata={"Software": None},
)
plt.close(figure)

print(f"read {source}")
print("wrote mpi_time_breakdown.png")
