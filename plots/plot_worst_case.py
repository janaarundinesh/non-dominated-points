"""The other end of the input range: when no point is dominated.

MAXIMA2 is fastest when almost every point is pruned; this script looks at the
opposite input, an antichain on which every one of the N points is maximal, so
neither algorithm can discard anything.

Two panels instead of three:

* the antichain runtime of all three programs in the highest dimension,
* how much slower MAXIMA2 is on the antichain than on uniform data of the same
  size, one line per dimension,
* where the MPI time goes as the process count rises, which is where the gather
  takes over.

Data comes from scripts/worst_case_sweep.sh (plots/data/worst_case_all_maxima.csv),
scripts/worst_case_mpi_sweep.py (plots/data/worst_case_mpi_scaling.csv) and the
ordinary benchmark CSVs for the uniform comparison.
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
SCALING_CSV = DATA_DIR / "worst_case_mpi_scaling.csv"

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
# Optional: written by scripts/worst_case_mpi_sweep.py.
mpi = worst[worst["algorithm"] == "mpi_marry_go_round"]

MPI_PROCESSES = 8

figure, (curves, penalty, scaling) = plt.subplots(1, 3, figsize=(19, 5))

# ---------------------------------------------------------------------------
# 1. Runtime on the antichain in the highest dimension.
# ---------------------------------------------------------------------------

shown = dimensions[-1]

for frame, label, marker, linestyle, colour in [
    (maxima2, "MAXIMA2", "o", "-", "tab:blue"),
    (brute, "brute force", "s", "--", "tab:orange"),
    (mpi, f"MPI (P={MPI_PROCESSES})", "D", "-.", "tab:green"),
]:
    data = frame[frame["dimensions"] == shown].sort_values("num_points")

    if data.empty:
        continue

    curves.plot(
        data["num_points"],
        data["time_s"],
        marker=marker,
        markersize=5,
        linewidth=1.8,
        linestyle=linestyle,
        label=label,
        color=colour,
    )

curves.set_xscale("log")
curves.set_yscale("log")
curves.set_xlabel("Number of Points")
curves.set_ylabel("Seconds")
curves.set_title(f"Antichain ({shown}D): every point is maximal")
curves.grid(True, which="both", linestyle=":", alpha=0.5)
curves.legend()

# ---------------------------------------------------------------------------
# 2. Antichain cost against uniform cost, one line per dimension.
# ---------------------------------------------------------------------------

ratios = []

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

    merged["ratio"] = merged["time_s"] / merged["time_s_uniform"]
    ratios.append((dim, merged))

    penalty.plot(
        merged["num_points"],
        merged["ratio"],
        marker="o",
        markersize=5,
        linewidth=1.8,
        color=colours[dim],
        label=f"{dim}D",
    )

penalty.axhline(1.0, color="grey", linestyle="--", linewidth=1)
penalty.set_xscale("log")
penalty.set_xlabel("Number of Points")
penalty.set_ylabel("Antichain time / uniform time")
penalty.set_title("MAXIMA2 cost on the antichain")
penalty.grid(True, which="both", linestyle=":", alpha=0.5)
penalty.legend(title="Dimension", ncol=2)

# ---------------------------------------------------------------------------
# 3. Where the MPI time goes as the process count rises.
# ---------------------------------------------------------------------------

if SCALING_CSV.exists():
    scaling_data = pd.read_csv(SCALING_CSV)

    # Pick the largest dataset that has the full process sweep, so the panel
    # never collapses to a single point when the biggest input was measured at
    # one process count only.
    counts = (
        scaling_data.groupby(["dimensions", "num_points"])["num_processes"]
        .nunique()
        .reset_index(name="process_counts")
    )
    best = counts[counts["process_counts"] == counts["process_counts"].max()]
    largest = best.sort_values(["dimensions", "num_points"]).iloc[-1]

    shown_scaling = scaling_data[
        (scaling_data["dimensions"] == largest["dimensions"])
        & (scaling_data["num_points"] == largest["num_points"])
    ].sort_values("num_processes")

    for column, label, colour, marker in [
        ("median_total_ms", "Total", "black", "s"),
        ("median_gather_ms", "Gather", "tab:red", "o"),
        ("median_communication_ms", "Communication", "tab:purple", "^"),
        ("median_local_ms", "Local brute force", "tab:blue", "D"),
    ]:
        scaling.plot(
            shown_scaling["num_processes"],
            shown_scaling[column].clip(lower=1e-3) / 1000.0,
            marker=marker,
            markersize=5,
            linewidth=1.8,
            color=colour,
            label=label,
        )

    scaling.set_xscale("log", base=2)
    scaling.set_yscale("log")
    scaling.set_xticks(shown_scaling["num_processes"].tolist())
    scaling.set_xticklabels(
        [int(p) for p in shown_scaling["num_processes"]]
    )
    scaling.set_xlabel("MPI processes")
    scaling.set_ylabel("Seconds")
    scaling.set_title(
        f"Where the time goes ({int(largest['dimensions'])}D, "
        f"N = {int(largest['num_points']):,})"
    )
    scaling.grid(True, which="both", linestyle=":", alpha=0.5)
    scaling.legend(fontsize=8)
else:
    scaling.set_visible(False)
    print(f"no {SCALING_CSV.name}; the MPI scaling panel stays empty")

figure.tight_layout()
figure.savefig(
    FIGURES_DIR / "worst_case.png",
    dpi=300,
    bbox_inches="tight",
    metadata={"Software": None},
)
plt.close(figure)

print("wrote worst_case.png")

# ---------------------------------------------------------------------------
# Numbers for the report.
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


print("\nFitted exponents, N = 1 000 … 100 000:")

for dim in dimensions:
    slope, (low, high) = fitted_exponent(maxima, dim, 1000)
    worst_slope, _ = fitted_exponent(maxima2, dim, 1000)

    bound = (
        1
        + (dim - 2) * math.log(math.log(high) / math.log(low)) / math.log(high / low)
        if low
        else float("nan")
    )

    print(
        f"  d={dim:2d}  uniform={slope:.2f}  antichain={worst_slope:.2f}  "
        f"bound={bound:.2f}"
    )

print("\nAntichain slowdown of MAXIMA2 at the largest shared N:")

for dim, merged in ratios:
    row = merged.sort_values("num_points").iloc[-1]

    print(
        f"  d={dim:2d}: N={int(row['num_points'])}, {row['ratio']:.2f}x "
        f"({row['time_s'] * 1000:.1f} ms vs {row['time_s_uniform'] * 1000:.1f} ms)"
    )
