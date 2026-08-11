from pathlib import Path

import pandas as pd
import matplotlib.pyplot as plt


BASE_DIR = Path(__file__).resolve().parent
PROJECT_DIR = BASE_DIR.parent

RESULTS_DIR = PROJECT_DIR / "results"
FIGURES_DIR = BASE_DIR / "figures"

FIGURES_DIR.mkdir(exist_ok=True)


# Read benchmark data
maxima = pd.read_csv(
    RESULTS_DIR / "maxima_benchmark.csv"
)

brute_force = pd.read_csv(
    RESULTS_DIR / "brute_force_benchmark.csv"
)


# Convert execution time from microseconds to seconds
maxima["execution_time_s"] = (
    maxima["execution_time"] / 1_000_000
)

brute_force["execution_time_s"] = (
    brute_force["execution_time"] / 1_000_000
)


# Create one plot for each dimension
for dim in sorted(maxima["dimensions"].unique()):

    maxima_subset = maxima[
        maxima["dimensions"] == dim
    ].sort_values("num_points")

    brute_force_subset = brute_force[
        brute_force["dimensions"] == dim
    ].sort_values("num_points")


    plt.figure(figsize=(8, 5))


    plt.plot(
        maxima_subset["num_points"],
        maxima_subset["execution_time_s"],
        marker="o",
        linewidth=2,
        markersize=6,
        label="MAXIMA2"
    )


    plt.plot(
        brute_force_subset["num_points"],
        brute_force_subset["execution_time_s"],
        marker="o",
        linewidth=2,
        markersize=6,
        label="Brute Force"
    )


    plt.xscale("log")
    plt.yscale("log")

    plt.xlabel("Number of Points")
    plt.ylabel("Execution Time (s)")
    plt.title(
        f"Runtime Comparison: Brute Force vs MAXIMA2 ({dim}D)"
    )

    plt.legend()
    plt.grid(True)

    plt.tight_layout()


    plt.savefig(
        FIGURES_DIR / f"runtime_comparison_{dim}D.png",
        dpi=300
    )

    plt.close()


print("Plots saved successfully!")