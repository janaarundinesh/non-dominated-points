"""The other end of the input range: when no point is dominated.

MAXIMA2 is fastest when almost every point is pruned; this script looks at the
opposite input, an antichain on which every one of the N points is maximal, so
neither algorithm can discard anything. Produces
figures/worst_case_all_maxima.png:

* runtime on the antichain for both algorithms, against N,
* the fitted exponent next to the uniform-input measurement and the
  worst-case bound N log2^(d-2) N,
* how much slower each algorithm is on the antichain than on the uniform
  dataset of the same size.

Data comes from scripts/worst_case_sweep.sh (plots/data/worst_case_all_maxima.csv)
plus the ordinary benchmark CSVs for the uniform comparison.
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


WORST_CASE_CSV = DATA_DIR / "worst_case_all_maxima.csv"

require(WORST_CASE_CSV, MAXIMA_CSV, BRUTE_FORCE_CSV)

FIGURES_DIR.mkdir(exist_ok=True)

worst = pd.read_csv(WORST_CASE_CSV)
maxima = load_benchmark(MAXIMA_CSV)
brute_force = load_benchmark(BRUTE_FORCE_CSV)

worst["time_s"] = worst["median_ms"] / 1000.0

# Same name as in the worst-case table, so the two can be merged.
for frame in (maxima, brute_force):
    frame["time_s"] = frame["execution_time_s"]

dimensions = sorted(int(d) for d in worst["dimensions"].unique())

colours = {
    dim: plt.get_cmap("viridis")(index / max(1, len(dimensions) - 1))
    for index, dim in enumerate(dimensions)
}

maxima2 = worst[worst["algorithm"] == "maxima2"]
brute = worst[worst["algorithm"] == "brute_force"]


def save(figure, filename):
    figure.tight_layout()
    figure.savefig(FIGURES_DIR / filename, dpi=300, bbox_inches="tight")
    plt.close(figure)
    print(f"wrote {filename}")


# ---------------------------------------------------------------------------
# 1. Runtime on the antichain.
# ---------------------------------------------------------------------------

figure, panels = plt.subplots(1, 3, figsize=(16, 4.8))

curves = panels[0]

for dim in dimensions:
    data = maxima2[maxima2["dimensions"] == dim].sort_values("num_points")

    curves.errorbar(
        data["num_points"],
        data["time_s"],
        yerr=[
            (data["median_ms"] - data["min_ms"]) / 1000.0,
            (data["max_ms"] - data["median_ms"]) / 1000.0,
        ],
        marker="o",
        markersize=4,
        linewidth=1.8,
        color=colours[dim],
        capsize=2,
        label=f"{dim}D MAXIMA2",
    )

for dim in dimensions:
    data = brute[brute["dimensions"] == dim].sort_values("num_points")

    curves.plot(
        data["num_points"],
        data["time_s"],
        marker="s",
        markersize=4,
        linewidth=1.2,
        linestyle="--",
        color=colours[dim],
    )

curves.set_xscale("log")
curves.set_yscale("log")
curves.set_xlabel("Number of Points")
curves.set_ylabel("Seconds")
curves.set_title("Antichain: every point is maximal")
curves.grid(True, which="both", linestyle=":", alpha=0.5)
curves.legend(fontsize=7, ncol=2)

curves.text(
    0.03,
    0.04,
    "solid: MAXIMA2    dashed: brute force",
    transform=curves.transAxes,
    fontsize=8,
    bbox={"facecolor": "white", "edgecolor": "none", "alpha": 0.8},
)


# ---------------------------------------------------------------------------
# 2. Growth on the antichain, on uniform data, and the worst-case bound.
# ---------------------------------------------------------------------------

def fitted_exponent(frame, dimension, minimum):
    data = frame[
        (frame["dimensions"] == dimension) & (frame["num_points"] >= minimum)
    ].sort_values("num_points")

    if len(data) < 2:
        return float("nan"), (None, None)

    slope = float(
        np.polyfit(np.log(data["num_points"]), np.log(data["time_s"]), 1)[0]
    )

    return slope, (data["num_points"].min(), data["num_points"].max())


def bound_exponent(dimension, low, high):
    return 1 + (dimension - 2) * math.log(
        math.log(high) / math.log(low)
    ) / math.log(high / low)


uniform = maxima[maxima["dimensions"].isin(dimensions)]
uniform_brute = brute_force[brute_force["dimensions"].isin(dimensions)]

uniform_slopes = []
worst_slopes = []
bounds = []

for dim in dimensions:
    slope, (low, high) = fitted_exponent(uniform, dim, 1000)
    uniform_slopes.append(slope)
    worst_slope, _ = fitted_exponent(maxima2, dim, 1000)
    worst_slopes.append(worst_slope)
    bounds.append(bound_exponent(dim, low, high) if low else float("nan"))

    print(
        f"d={dim:2d}  MAXIMA2 slope uniform={slope:.2f}  "
        f"antichain={worst_slope:.2f}  bound={bounds[-1]:.2f}"
    )

brute_slopes = []

for dim in dimensions:
    slope, _ = fitted_exponent(brute, dim, 500)
    brute_slopes.append(slope)

slopes = panels[1]

positions = np.arange(len(dimensions))
width = 0.27

slopes.bar(
    positions - width,
    uniform_slopes,
    width,
    label="MAXIMA2, uniform",
    color="tab:blue",
)
slopes.bar(
    positions,
    worst_slopes,
    width,
    label="MAXIMA2, antichain",
    color="tab:orange",
)
slopes.bar(
    positions + width,
    bounds,
    width,
    label="bound $N\\log_2^{d-2}N$",
    color="tab:grey",
    alpha=0.7,
)

slopes.set_xticks(positions)
slopes.set_xticklabels([f"{dim}D" for dim in dimensions])
slopes.set_ylabel("Fitted exponent p in  t ~ N^p")
slopes.set_title("Growth with N = 1 000 … 100 000")
slopes.grid(True, axis="y", linestyle=":", alpha=0.5)
slopes.legend(fontsize=8, loc="upper left")


# ---------------------------------------------------------------------------
# 3. Penalty of the antichain against the uniform dataset of the same size.
# ---------------------------------------------------------------------------

penalty = panels[2]

for label, frame, uniform_frame, marker, linestyle in [
    ("MAXIMA2", maxima2, maxima, "o", "-"),
    ("brute force", brute, uniform_brute, "s", "--"),
]:
    for dim in dimensions:
        data = frame[frame["dimensions"] == dim]

        reference = uniform_frame[uniform_frame["dimensions"] == dim]

        merged = data.merge(
            reference[["num_points", "time_s"]],
            on="num_points",
            suffixes=("", "_uniform"),
        )

        if merged.empty:
            continue

        merged["ratio"] = merged["time_s"] / merged["time_s_uniform"]

        penalty.plot(
            merged["num_points"],
            merged["ratio"],
            marker=marker,
            markersize=4,
            linewidth=1.6,
            linestyle=linestyle,
            color=colours[dim],
        )

    penalty.plot([], [], color="black", marker=marker, linestyle=linestyle,
                 label=label)

penalty.axhline(1.0, color="grey", linestyle="--", linewidth=1)
penalty.set_xscale("log")
penalty.set_xlabel("Number of Points")
penalty.set_ylabel("Antichain time / uniform time")
penalty.set_title("Antichain cost, colour = dimension (5D … 10D)")
penalty.grid(True, which="both", linestyle=":", alpha=0.5)
penalty.legend(fontsize=8)

save(figure, "worst_case_all_maxima.png")


# ---------------------------------------------------------------------------
# Numbers for the report.
# ---------------------------------------------------------------------------

print("\nSlowdown of MAXIMA2 on the antichain at the largest shared N:")

for dim in dimensions:
    data = maxima2[maxima2["dimensions"] == dim]
    reference = maxima[maxima["dimensions"] == dim]

    merged = data.merge(
        reference[["num_points", "time_s"]],
        on="num_points",
        suffixes=("", "_uniform"),
    )

    if merged.empty:
        continue

    row = merged.sort_values("num_points").iloc[-1]

    print(
        f"  d={dim:2d}: N={int(row['num_points'])}, "
        f"{row['time_s'] / row['time_s_uniform']:.2f}x "
        f"({row['time_s'] * 1000:.1f} ms vs {row['time_s_uniform'] * 1000:.1f} ms)"
    )

print("\nBrute-force exponent on the antichain (N = 500 … 10 000):")

for dim, slope in zip(dimensions, brute_slopes):
    print(f"  d={dim:2d}: p={slope:.2f}")
