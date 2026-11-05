import matplotlib.pyplot as plt

from benchmark_data import (
    BRUTE_FORCE_CSV,
    FIGURES_DIR,
    MAXIMA_CSV,
    load_benchmark,
    require,
)


require(MAXIMA_CSV, BRUTE_FORCE_CSV)

FIGURES_DIR.mkdir(exist_ok=True)

maxima = load_benchmark(MAXIMA_CSV)
brute_force = load_benchmark(BRUTE_FORCE_CSV)


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

    plt.xscale("log")
    plt.yscale("log")

    plt.xlabel("Number of Points")
    plt.ylabel("Execution Time (s)")
    plt.title(f"Runtime Comparison: Brute Force vs MAXIMA2 ({dim}D)")

    plt.legend()
    plt.grid(True, which="both", linestyle="--", alpha=0.4)

    plt.tight_layout()

    plt.savefig(FIGURES_DIR / f"runtime_comparison_{dim}D.png", dpi=300)

    plt.close()


print("Plots saved successfully!")
