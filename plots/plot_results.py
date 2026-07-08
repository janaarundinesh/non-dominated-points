from pathlib import Path
import pandas as pd
import matplotlib.pyplot as plt

BASE_DIR = Path(__file__).resolve().parent
PROJECT_DIR = BASE_DIR.parent

RESULTS_DIR = PROJECT_DIR / "results"
FIGURES_DIR = BASE_DIR / "figures"

FIGURES_DIR.mkdir(exist_ok=True)

# Read benchmark data
df = pd.read_csv(RESULTS_DIR / "benchmark_results.csv")

# Create one plot for each dimension
for dim in sorted(df["Dimension"].unique()):

    subset = df[df["Dimension"] == dim].sort_values("NPoints")

    plt.figure(figsize=(8, 5))

    plt.plot(
        subset["NPoints"],
        subset["ExecutionTime_us"] / 1000, # Convert to milliseconds
        marker="o",
        linewidth=2,
        markersize=6
    )

    plt.xscale("log")
    plt.yscale("log")
    plt.xlabel("Number of Points")
    plt.ylabel("Execution Time (ms)")
    plt.title(f"Runtime vs Number of Points for ({dim}D)")
    plt.grid(True)

    plt.tight_layout()

    plt.savefig(
        FIGURES_DIR / f"runtime_{dim}D.png",
        dpi=300
    )

    plt.close()

print("Plots saved successfully!")