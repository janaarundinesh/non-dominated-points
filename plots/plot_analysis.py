"""Analyses over the benchmark results that go beyond "which is faster".

plot_results.py answers the runtime comparison. This script answers the
questions a report has to defend:

* scaling_exponent.png      measured growth versus the worst-case bound
* normalized_runtime.png    runtime divided by N log2^(d-2) N
* algorithmic_advantage.png brute-force time divided by MAXIMA2 time
* per_point_cost.png        microseconds per point versus N
* maxima_fraction.png       share of the input that is actually maximal
* sequential_vs_parallel.png local MPI brute force versus sequential MAXIMA2
* memory_and_large_n.png    peak memory and the million-point measurements

The last two need the optional measurement files in plots/data/; the others
only need the benchmark CSVs.
"""

import math

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

from benchmark_data import (
    BRUTE_FORCE_CSV,
    DATA_DIR,
    FIGURES_DIR,
    MAXIMA_CSV,
    load_benchmark,
    require,
)


require(MAXIMA_CSV, BRUTE_FORCE_CSV)

FIGURES_DIR.mkdir(exist_ok=True)

maxima = load_benchmark(MAXIMA_CSV)
brute_force = load_benchmark(BRUTE_FORCE_CSV)

dimensions = sorted(int(d) for d in maxima["dimensions"].unique())

colours = {
    dim: plt.get_cmap("viridis")(index / max(1, len(dimensions) - 1))
    for index, dim in enumerate(dimensions)
}


def save(figure, filename):
    figure.tight_layout()
    figure.savefig(FIGURES_DIR / filename, dpi=300, bbox_inches="tight")
    plt.close(figure)
    print(f"wrote {filename}")


def subset(frame, dimension):
    return frame[frame["dimensions"] == dimension].sort_values("num_points")


# ---------------------------------------------------------------------------
# 1. Algorithmic advantage: how much slower the quadratic baseline is.
# ---------------------------------------------------------------------------

advantage = maxima.merge(
    brute_force,
    on=["dimensions", "num_points"],
    suffixes=("_maxima", "_brute"),
)

advantage["ratio"] = (
    advantage["execution_time_s_brute"] / advantage["execution_time_s_maxima"]
)

figure, axes = plt.subplots(figsize=(8, 5))

for dim in dimensions:
    data = advantage[advantage["dimensions"] == dim].sort_values("num_points")
    axes.plot(
        data["num_points"],
        data["ratio"],
        marker="o",
        linewidth=1.8,
        color=colours[dim],
        label=f"{dim}D",
    )

axes.axhline(1.0, color="grey", linestyle="--", linewidth=1)
axes.set_xscale("log")
axes.set_yscale("log")
axes.set_xlabel("Number of Points")
axes.set_ylabel("Time (brute force) / Time (MAXIMA2)")
axes.set_title("Algorithmic advantage of MAXIMA2 over the quadratic baseline")
axes.grid(True, which="both", linestyle=":", alpha=0.5)
axes.legend(title="Dimension", ncol=2)

save(figure, "algorithmic_advantage.png")


# ---------------------------------------------------------------------------
# 2. Cost per point and the growth rate.
# ---------------------------------------------------------------------------

maxima["us_per_point"] = maxima["execution_time_s"] * 1e6 / maxima["num_points"]

figure, axes = plt.subplots(figsize=(8, 5))

for dim in dimensions:
    data = subset(maxima, dim)
    axes.plot(
        data["num_points"],
        data["us_per_point"],
        marker="o",
        linewidth=1.8,
        color=colours[dim],
        label=f"{dim}D",
    )

axes.set_xscale("log")
axes.set_yscale("log")
axes.set_xlabel("Number of Points")
axes.set_ylabel("Microseconds per point")
axes.set_title("Cost per point: flat in low dimensions, rising with d")
axes.grid(True, which="both", linestyle=":", alpha=0.5)
axes.legend(title="Dimension", ncol=2)

save(figure, "per_point_cost.png")


# ---------------------------------------------------------------------------
# 3. Normalised runtime: flat means the measured cost follows N log^(d-2) N.
# ---------------------------------------------------------------------------

figure, axes = plt.subplots(figsize=(8, 5))

for dim in dimensions:
    data = subset(maxima, dim).copy()
    data["normalised"] = data["execution_time_s"] / (
        data["num_points"] * np.log2(data["num_points"]) ** (dim - 2)
    )

    axes.plot(
        data["num_points"],
        data["normalised"],
        marker="o",
        linewidth=1.8,
        color=colours[dim],
        label=f"{dim}D",
    )

axes.set_xscale("log")
axes.set_yscale("log")
axes.set_xlabel("Number of Points")
axes.set_ylabel(r"$t \,/\, (N \log_2^{d-2} N)$   [s]")
axes.set_title("Runtime normalised by the worst-case bound")
axes.grid(True, which="both", linestyle=":", alpha=0.5)
axes.legend(title="Dimension", ncol=2)

save(figure, "normalized_runtime.png")


# ---------------------------------------------------------------------------
# 4. Fitted growth exponent versus the worst-case exponent.
# ---------------------------------------------------------------------------

fits = []

for dim in dimensions:
    data = subset(maxima, dim)
    data = data[data["num_points"] >= 1000]

    slope = float(
        np.polyfit(np.log(data["num_points"]), np.log(data["execution_time_s"]), 1)[0]
    )

    low, high = data["num_points"].min(), data["num_points"].max()

    # Exponent implied by N log2^(d-2) N between the fitted endpoints.
    theoretical = 1 + (dim - 2) * math.log(
        math.log(high) / math.log(low)
    ) / math.log(high / low)

    fits.append((dim, slope, theoretical))

    print(
        f"d={dim:2d}  measured slope={slope:.2f}  "
        f"worst-case slope={theoretical:.2f}  (N={low}..{high})"
    )

figure, axes = plt.subplots(figsize=(8, 5))

positions = np.arange(len(fits))
width = 0.38

axes.bar(
    positions - width / 2,
    [slope for _, slope, _ in fits],
    width,
    label="measured",
    color="tab:blue",
)

axes.bar(
    positions + width / 2,
    [theoretical for _, _, theoretical in fits],
    width,
    label="worst case $N \\log_2^{d-2} N$",
    color="tab:grey",
    alpha=0.7,
)

axes.set_xticks(positions)
axes.set_xticklabels([f"{dim}D" for dim, _, _ in fits])
axes.set_ylabel("Exponent p in  t ~ N^p")
axes.set_title("Measured growth versus the worst-case bound (N = 1 000 … 100 000)")
axes.grid(True, axis="y", linestyle=":", alpha=0.5)
axes.legend()

save(figure, "scaling_exponent.png")


# ---------------------------------------------------------------------------
# 5. How much of the input is actually maximal.
# ---------------------------------------------------------------------------

maxima["maxima_fraction"] = (
    maxima["num_non_dominated_points"] / maxima["num_points"]
)

figure, axes = plt.subplots(figsize=(8, 5))

for dim in dimensions:
    data = subset(maxima, dim)

    axes.plot(
        data["num_points"],
        data["maxima_fraction"],
        marker="o",
        linewidth=1.8,
        color=colours[dim],
        label=f"{dim}D",
    )

axes.set_xscale("log")
axes.set_xlabel("Number of Points")
axes.set_ylabel("Maxima / N")
axes.set_title("Share of the input that is non-dominated")
axes.grid(True, which="both", linestyle=":", alpha=0.5)
axes.legend(title="Dimension", ncol=2)

save(figure, "maxima_fraction.png")

print("\nCrossover point of the algorithmic advantage (first N with ratio > 1):")

for dim in dimensions:
    data = advantage[advantage["dimensions"] == dim].sort_values("num_points")
    above = data[data["ratio"] > 1]

    if not above.empty:
        row = above.iloc[0]
        print(
            f"  d={dim:2d}: N={int(row['num_points'])}, "
            f"{float(row['ratio']):.2f}x"
        )


# ---------------------------------------------------------------------------
# 6. Sequential MAXIMA2 versus parallel brute force on one machine, and memory.
# ---------------------------------------------------------------------------

local_csv = DATA_DIR / "local_mpi_vs_sequential.csv"

if local_csv.exists():
    local = pd.read_csv(local_csv)
    datasets = (
        local[["dataset", "dimensions", "num_points"]]
        .drop_duplicates()
        .sort_values(["dimensions", "num_points"])["dataset"]
        .tolist()
    )

    figure, axes_list = plt.subplots(
        1,
        len(datasets),
        figsize=(5.0 * len(datasets), 4.6),
        squeeze=False,
    )

    for axes, name in zip(axes_list[0], datasets):
        data = local[local["dataset"] == name]
        medians = (
            data.groupby("processes", as_index=False)["time_s"]
            .median()
            .sort_values("processes")
        )

        axes.plot(
            medians["processes"],
            medians["time_s"],
            marker="o",
            linewidth=1.8,
            color="tab:orange",
            label="MPI brute force",
        )

        dimensions_of_data = int(data["dimensions"].iloc[0])
        points = int(data["num_points"].iloc[0])

        reference = maxima[
            (maxima["dimensions"] == dimensions_of_data)
            & (maxima["num_points"] == points)
        ]

        if not reference.empty:
            axes.axhline(
                reference["execution_time_s"].iloc[0],
                color="tab:blue",
                linestyle="--",
                linewidth=1.6,
                label="sequential MAXIMA2",
            )

        axes.set_yscale("log")
        axes.set_xticks([1, 2, 4, 8])
        axes.set_xlabel("MPI processes")
        axes.set_ylabel("Time (s)")
        axes.set_title(f"{name} ({dimensions_of_data}D, N={points:,})")
        axes.grid(True, which="both", linestyle=":", alpha=0.5)
        axes.legend(fontsize=8)

    save(figure, "sequential_vs_parallel.png")
else:
    print(f"skipped sequential_vs_parallel.png (no {local_csv})")


memory_csv = DATA_DIR / "memory_and_large_n.csv"

if memory_csv.exists():
    memory = pd.read_csv(memory_csv)

    figure, panels = plt.subplots(1, 2, figsize=(13, 4.8))

    for dim in sorted(int(d) for d in memory["dimensions"].unique()):
        data = memory[memory["dimensions"] == dim].sort_values("num_points")

        panels[0].plot(
            data["num_points"],
            data["peak_rss_mb"],
            marker="o",
            linewidth=1.8,
            color=colours.get(dim),
            label=f"{dim}D",
        )

        panels[1].plot(
            data["num_points"],
            data["extra_rss_mb"],
            marker="o",
            linewidth=1.8,
            color=colours.get(dim),
            label=f"{dim}D",
        )

    panels[0].set_title("Peak memory (input included)")
    panels[1].set_title("Memory added by the run")

    for panel in panels:
        panel.set_xscale("log")
        panel.set_xlabel("Number of Points")
        panel.set_ylabel("Megabytes")
        panel.grid(True, which="both", linestyle=":", alpha=0.5)
        panel.legend(title="Dimension")

    save(figure, "memory_and_large_n.png")
else:
    print(f"skipped memory_and_large_n.png (no {memory_csv})")
