"""Runtime comparison of brute force, MAXIMA2 and the MPI marry-go-round.

One 2x3 panel figure covers all six dimensions. Every panel carries the same
three curves, so the figure replaces the earlier one-file-per-dimension set
without making any single panel busier.
"""

import matplotlib.pyplot as plt

from benchmark_data import (
    BRUTE_FORCE_CSV,
    FIGURES_DIR,
    MARRY_GO_ROUND_CSV,
    MAXIMA_CSV,
    load_benchmark,
    load_parallel_benchmark,
    require,
)


# Process count drawn for the MPI curve; the benchmark CSV holds every count
# from 1 to 20, and one curve per panel keeps the comparison readable.
MARRY_GO_ROUND_PROCESSES = 8

require(MAXIMA_CSV, BRUTE_FORCE_CSV)

FIGURES_DIR.mkdir(exist_ok=True)

maxima = load_benchmark(MAXIMA_CSV)
brute_force = load_benchmark(BRUTE_FORCE_CSV)

# The MPI results come from a local run and are not tracked, so the third curve
# is optional: the two sequential series still plot without it.
if MARRY_GO_ROUND_CSV.exists():
    marry_go_round = load_parallel_benchmark(
        MARRY_GO_ROUND_CSV, MARRY_GO_ROUND_PROCESSES
    )
else:
    marry_go_round = None
    print(
        f"No MPI results at {MARRY_GO_ROUND_CSV}; "
        "plotting MAXIMA2 and brute force only."
    )

dimensions = sorted(int(d) for d in maxima["dimensions"].unique())

figure, axes = plt.subplots(
    2,
    3,
    figsize=(15, 8),
    squeeze=False,
    sharex=True,
    sharey=True,
)

panels = axes.flatten()

for panel, dim in zip(panels, dimensions):
    maxima_subset = maxima[maxima["dimensions"] == dim].sort_values("num_points")
    brute_force_subset = brute_force[
        brute_force["dimensions"] == dim
    ].sort_values("num_points")

    panel.plot(
        maxima_subset["num_points"],
        maxima_subset["execution_time_s"],
        marker="o",
        linewidth=2,
        color="tab:blue",
        label="MAXIMA2",
    )

    panel.plot(
        brute_force_subset["num_points"],
        brute_force_subset["execution_time_s"],
        marker="o",
        linewidth=2,
        color="tab:orange",
        label="Brute Force",
    )

    if marry_go_round is not None:
        marry_subset = marry_go_round[
            marry_go_round["dimensions"] == dim
        ].sort_values("num_points")

        if not marry_subset.empty:
            panel.plot(
                marry_subset["num_points"],
                marry_subset["execution_time_s"],
                marker="s",
                linestyle="--",
                linewidth=2,
                color="tab:green",
                label=f"MPI (P={MARRY_GO_ROUND_PROCESSES})",
            )

    panel.set_title(f"{dim}D")
    panel.set_xscale("log")
    panel.set_yscale("log")
    panel.grid(True, which="both", linestyle="--", alpha=0.4)

for panel in panels[len(dimensions):]:
    panel.set_visible(False)

# Axis labels go on the outer panels only; the grid shares its axes.
for panel in panels[-3:]:
    panel.set_xlabel("Number of Points")

for panel in panels[::3]:
    panel.set_ylabel("Execution Time (s)")

handles, labels = panels[0].get_legend_handles_labels()

figure.legend(
    handles,
    labels,
    loc="upper center",
    ncol=len(labels),
    frameon=False,
    bbox_to_anchor=(0.5, 1.0),
)

figure.suptitle("Runtime comparison over all dimensions", y=1.03)

figure.tight_layout()
figure.savefig(
    FIGURES_DIR / "runtime_comparison.png",
    dpi=300,
    bbox_inches="tight",
    metadata={"Software": None},
)
plt.close(figure)

print("wrote runtime_comparison.png")
