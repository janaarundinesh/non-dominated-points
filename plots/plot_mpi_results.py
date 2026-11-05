"""Speedup and scaling of the MPI marry-go-round.

Two single-panel figures from results/mpi_marry_go_round_benchmark.csv, whose
times are in seconds:

* speedup.png -- speedup over the one-process run, one line per dimension, at a
  fixed input size.
* scaling.png -- wall time against N at a fixed process count, one line per
  dimension.

One panel each, rather than a 2x3 grid of panels with a dozen lines apiece.
"""

import matplotlib.pyplot as plt
import pandas as pd

from benchmark_data import FIGURES_DIR, MARRY_GO_ROUND_CSV, require


# One representative input size for speedup and one process count for scaling;
# both fall back to the largest available value when the CSV lacks them.
SPEEDUP_POINTS = 10_000
SCALING_PROCESSES = 8

require(MARRY_GO_ROUND_CSV)

FIGURES_DIR.mkdir(exist_ok=True)

frame = pd.read_csv(MARRY_GO_ROUND_CSV)

frame = frame[
    (frame["num_points"] > 0)
    & (frame["num_processes"] > 0)
    & (frame["execution_time"] > 0)
].copy()

median = frame.groupby(
    ["dimensions", "num_points", "num_processes"], as_index=False
).agg(median_time=("execution_time", "median"))

if median.empty:
    raise SystemExit(f"No usable rows in {MARRY_GO_ROUND_CSV}")

dimensions = sorted(int(d) for d in median["dimensions"].unique())

available_points = sorted(int(p) for p in median["num_points"].unique())
speedup_points = (
    SPEEDUP_POINTS if SPEEDUP_POINTS in available_points else available_points[-1]
)

baseline = (
    median[median["num_processes"] == 1][
        ["dimensions", "num_points", "median_time"]
    ]
    .rename(columns={"median_time": "serial_time"})
)

speedup = median.merge(baseline, on=["dimensions", "num_points"], how="left")
speedup = speedup[speedup["serial_time"] > 0].copy()
speedup["observed_speedup"] = speedup["serial_time"] / speedup["median_time"]


def save(figure, filename):
    figure.tight_layout()
    figure.savefig(
        FIGURES_DIR / filename,
        dpi=300,
        bbox_inches="tight",
        metadata={"Software": None},
    )
    plt.close(figure)
    print(f"wrote {filename}")


# ---------------------------------------------------------------------------
# Speedup at one input size.
# ---------------------------------------------------------------------------

figure, panel = plt.subplots(figsize=(8, 5))

for dim in dimensions:
    data = speedup[
        (speedup["dimensions"] == dim) & (speedup["num_points"] == speedup_points)
    ].sort_values("num_processes")

    if data.empty:
        continue

    panel.plot(
        data["num_processes"],
        data["observed_speedup"],
        marker="o",
        markersize=5,
        linewidth=1.8,
        label=f"{dim}D",
    )

panel.axhline(1.0, color="grey", linestyle=":", linewidth=1)
panel.set_xlabel("MPI processes")
panel.set_ylabel("Speedup over one process")
panel.set_title(f"Speedup on the marry-go-round, N = {speedup_points:,}")
panel.grid(True, which="both", linestyle=":", alpha=0.5)
panel.legend(title="Dimension", ncol=2)

save(figure, "speedup.png")


# ---------------------------------------------------------------------------
# Wall time against N at one process count.
# ---------------------------------------------------------------------------

available_processes = sorted(int(p) for p in median["num_processes"].unique())
scaling_processes = (
    SCALING_PROCESSES
    if SCALING_PROCESSES in available_processes
    else available_processes[-1]
)

figure, panel = plt.subplots(figsize=(8, 5))

for dim in dimensions:
    data = median[
        (median["dimensions"] == dim)
        & (median["num_processes"] == scaling_processes)
    ].sort_values("num_points")

    if data.empty:
        continue

    panel.plot(
        data["num_points"],
        data["median_time"],
        marker="o",
        markersize=5,
        linewidth=1.8,
        label=f"{dim}D",
    )

panel.set_xscale("log")
panel.set_yscale("log")
panel.set_xlabel("Number of Points")
panel.set_ylabel(f"Execution Time (s), P = {scaling_processes}")
panel.set_title("Marry-go-round runtime against input size")
panel.grid(True, which="both", linestyle=":", alpha=0.5)
panel.legend(title="Dimension", ncol=2)

save(figure, "scaling.png")
