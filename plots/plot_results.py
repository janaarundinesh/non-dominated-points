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


# Process count drawn for the MPI marry-go-round curve; the benchmark CSV holds
# every count from 1 to 20, and one curve keeps the comparison readable.
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


def draw(frame, label, color):
    plt.errorbar(
        frame["num_points"],
        frame["execution_time_s"],
        yerr=[frame["lower_error"], frame["upper_error"]],
        marker="o",
        linewidth=2,
        markersize=6,
        capsize=3,
        color=color,
        label=label,
    )


# Create one plot for each dimension
for dim in sorted(maxima["dimensions"].unique()):

    maxima_subset = maxima[maxima["dimensions"] == dim].sort_values("num_points")
    brute_force_subset = brute_force[
        brute_force["dimensions"] == dim
    ].sort_values("num_points")

    plt.figure(figsize=(8, 5))

    draw(maxima_subset, "MAXIMA2", "tab:blue")
    draw(brute_force_subset, "Brute Force", "tab:orange")

    if marry_go_round is not None:
        marry_subset = marry_go_round[
            marry_go_round["dimensions"] == dim
        ].sort_values("num_points")

        if not marry_subset.empty:
            plt.plot(
                marry_subset["num_points"],
                marry_subset["execution_time_s"],
                marker="s",
                markersize=6,
                linewidth=2,
                linestyle="--",
                color="tab:green",
                label=f"MPI Marry-go-round (P={MARRY_GO_ROUND_PROCESSES})",
            )

    plt.xscale("log")
    plt.yscale("log")

    plt.xlabel("Number of Points")
    plt.ylabel("Execution Time (s)")
    plt.title(f"Runtime Comparison: Brute Force vs MAXIMA2 vs MPI ({dim}D)")

    plt.legend()
    plt.grid(True, which="both", linestyle="--", alpha=0.4)

    plt.tight_layout()

    # metadata={"Software": None} keeps the plotting library's name out of the
    # PNG chunk.
    plt.savefig(
        FIGURES_DIR / f"runtime_comparison_{dim}D.png",
        dpi=300,
        metadata={"Software": None},
    )

    plt.close()


print("Plots saved successfully!")
