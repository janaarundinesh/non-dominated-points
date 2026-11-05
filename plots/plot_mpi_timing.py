"""Where the time goes inside the marry-go-round MPI program.

Reads the timing breakdown written by MPI_Marrygomethod and produces
figures/mpi_time_breakdown.png: for each measured dataset, the wall time of the
critical rank split into local brute force, cross-filter, message exchange and
barrier wait (top row, absolute), and the same split as a share of the wall
time (bottom row).

The components come from the rank that determined the wall time, so they add up
to the total; the busiest rank's communication is drawn as a dotted line for
comparison. Scatter and gather are outside that region: they are reported by
the program but are too small to show here (under 3 ms even at 100 000 points).

Run scripts/mpi_timing_sweep.sh first; the CSV is the one that program writes.
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

median = (
    frame.groupby(["dimensions", "num_points", "num_processes"], as_index=False)
    .median(numeric_only=True)
    .sort_values(["dimensions", "num_points", "num_processes"])
)

datasets = (
    median[["dimensions", "num_points"]]
    .drop_duplicates()
    .sort_values(["dimensions", "num_points"])
    .values
    .tolist()
)

fig, panels = plt.subplots(
    2,
    len(datasets),
    figsize=(5.0 * len(datasets), 8.4),
    squeeze=False,
)

for column, (dimension, points) in enumerate(datasets):
    data = median[
        (median["dimensions"] == dimension) & (median["num_points"] == points)
    ].sort_values("num_processes")

    absolute = panels[0][column]
    shares = panels[1][column]

    for field, label, colour in COMPONENTS:
        absolute.plot(
            data["num_processes"],
            data[field],
            marker="o",
            linewidth=1.8,
            color=colour,
            label=label,
        )

    # The busiest rank usually talks more than the critical one, whose
    # decomposition is plotted above; showing both avoids the question of why
    # the communication line can fall while the process count rises.
    if "max_communication_s" in data:
        absolute.plot(
            data["num_processes"],
            data["max_communication_s"],
            marker="^",
            markersize=5,
            linewidth=1.2,
            linestyle=":",
            color="tab:red",
            label="Communication (busiest rank)",
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
    absolute.set_ylim(top=absolute.get_ylim()[1] * 2)
    absolute.set_xlabel("MPI processes")
    absolute.set_ylabel("Seconds")
    absolute.set_title(f"{int(dimension)}D, N = {int(points):,}")
    absolute.grid(True, which="both", linestyle=":", alpha=0.5)
    absolute.legend(fontsize=7, loc="lower left")

    bottom = np.zeros(len(data))
    positions = np.arange(len(data))

    # Normalised by the sum of the components rather than by the total: the
    # medians of the parts need not add up to the median of the whole, and a
    # stacked bar that overshoots 100 % would only raise the question.
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
    shares.set_title(f"{int(dimension)}D, N = {int(points):,}")
    shares.grid(True, axis="y", linestyle=":", alpha=0.5)

fig.suptitle("Marry-go-round breakdown: the critical rank's wall time", y=1.0)

fig.tight_layout()
fig.savefig(FIGURES_DIR / "mpi_time_breakdown.png", dpi=300, bbox_inches="tight")
plt.close(fig)

print(f"read {source}")
print("wrote mpi_time_breakdown.png")

print("\nBreakdown at the largest process count:")
print(
    median[median["num_processes"] == median["num_processes"].max()][
        [
            "dimensions",
            "num_points",
            "num_processes",
            "total_s",
            "local_s",
            "crossfilter_s",
            "communication_s",
            "barrier_s",
            "bytes_exchanged",
            "local_imbalance",
        ]
    ].to_string(index=False)
)
