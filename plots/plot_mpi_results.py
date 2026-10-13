
from pathlib import Path
import math

import pandas as pd
import matplotlib.pyplot as plt


BASE_DIR = Path(__file__).resolve().parent.parent

INPUT_FILE = BASE_DIR / "results" / "mpi_marry_go_round_benchmark.csv"
OUTPUT_DIR = Path(__file__).resolve().parent / "figures"

OUTPUT_DIR.mkdir(parents=True, exist_ok=True)

REQUIRED_COLUMNS = [
    "dimensions",
    "num_points",
    "num_processes",
    "execution_time",
    "num_non_dominated_points",
]

# Must match the unit stored in the CSV.
TIME_UNIT = "seconds"
TIME_LABEL = f"Median Execution Time ({TIME_UNIT})"


if not INPUT_FILE.exists():
    raise FileNotFoundError(
        f"CSV file not found: {INPUT_FILE}\n"
        "Check that the CSV is inside the results directory."
    )

df = pd.read_csv(INPUT_FILE)

missing_columns = set(REQUIRED_COLUMNS) - set(df.columns)

if missing_columns:
    raise ValueError(
        f"Missing CSV columns: {sorted(missing_columns)}"
    )

for column in REQUIRED_COLUMNS:
    df[column] = pd.to_numeric(df[column], errors="coerce")

df = df.dropna(subset=REQUIRED_COLUMNS).copy()

df = df[
    (df["dimensions"] > 0)
    & (df["num_points"] > 0)
    & (df["num_processes"] > 0)
    & (df["execution_time"] > 0)
].copy()

if df.empty:
    raise ValueError("No valid benchmark data found.")

for column in [
    "dimensions",
    "num_points",
    "num_processes",
    "num_non_dominated_points",
]:
    df[column] = df[column].astype(int)


group_columns = [
    "dimensions",
    "num_points",
    "num_processes",
]

median_df = (
    df.groupby(group_columns, as_index=False)
    .agg(
        median_time=("execution_time", "median"),
        repetitions=("execution_time", "count"),
        median_non_dominated_points=(
            "num_non_dominated_points",
            "median",
        ),
    )
)

dimensions = sorted(median_df["dimensions"].unique().tolist())

input_sizes = sorted(
    median_df["num_points"].unique().tolist()
)

processors_available = sorted(
    median_df["num_processes"].unique().tolist()
)


baseline = median_df[
    median_df["num_processes"] == 1
][
    ["dimensions", "num_points", "median_time"]
].rename(
    columns={"median_time": "serial_time"}
)

summary_df = median_df.merge(
    baseline,
    on=["dimensions", "num_points"],
    how="left",
)

summary_df["observed_speedup"] = (
    summary_df["serial_time"] / summary_df["median_time"]
)

summary_df["parallel_efficiency_percent"] = (
    summary_df["observed_speedup"]
    / summary_df["num_processes"]
    * 100
)

summary_file = OUTPUT_DIR / "median_performance_results.csv"
summary_df.to_csv(summary_file, index=False)

def create_axes():
    """Create a two-column subplot layout."""

    count = len(dimensions)
    rows = math.ceil(count / 2)

    fig, axes = plt.subplots(
        rows,
        2,
        figsize=(16, 5.5 * rows),
        squeeze=False,
    )

    flat_axes = axes.flatten()

    for ax in flat_axes[count:]:
        ax.set_visible(False)

    return fig, flat_axes[:count]


def save_figure(fig, filename):
    """Save a figure in the figures directory."""

    output_path = OUTPUT_DIR / filename

    fig.tight_layout()
    fig.savefig(
        output_path,
        dpi=300,
        bbox_inches="tight",
    )

    plt.close(fig)


fig, axes = create_axes()

for ax, dimension in zip(axes, dimensions):

    dimension_df = summary_df[
        summary_df["dimensions"] == dimension
    ]

    for n_points in sorted(
        dimension_df["num_points"].unique()
    ):

        subset = dimension_df[
            dimension_df["num_points"] == n_points
        ].sort_values("num_processes")

        subset = subset.dropna(
            subset=["observed_speedup"]
        )

        if subset.empty:
            continue

        ax.plot(
            subset["num_processes"],
            subset["observed_speedup"],
            marker="o",
            markersize=4,
            linewidth=1.6,
            label=f"N = {n_points:,}",
        )

    ax.set_title(f"{dimension}D")
    ax.set_xlabel("Number of Processors (P)")
    ax.set_ylabel("Observed Speedup: T(1) / T(P)")

    ax.set_xticks(processors_available)

    ax.grid(
        True,
        which="both",
        linestyle="--",
        alpha=0.4,
    )

    ax.legend(
        title="Input size",
        fontsize=7,
        ncol=2,
    )

save_figure(
    fig,
    "speedup_all_dimensions.png",
)

fig, axes = create_axes()

for ax, dimension in zip(axes, dimensions):

    dimension_df = median_df[
        median_df["dimensions"] == dimension
    ]

    for process_count in processors_available:

        subset = dimension_df[
            dimension_df["num_processes"] == process_count
        ].sort_values("num_points")

        if subset.empty:
            continue

        ax.plot(
            subset["num_points"],
            subset["median_time"],
            marker="o",
            markersize=4,
            linewidth=1.6,
            label=f"P = {process_count}",
        )

    ax.set_title(f"{dimension}D")
    ax.set_xlabel("Number of Points (N)")
    ax.set_ylabel(TIME_LABEL)

    ax.set_xscale("log")
    ax.set_yscale("log")

    ax.grid(
        True,
        which="both",
        linestyle="--",
        alpha=0.4,
    )

    ax.legend(
        title="Processors",
        fontsize=8,
        ncol=2,
    )

save_figure(
    fig,
    "problem_size_scaling_all_processors.png",
)

fig, axes = create_axes()

for ax, dimension in zip(axes, dimensions):

    dimension_df = median_df[
        median_df["dimensions"] == dimension
    ]

    for n_points in sorted(
        dimension_df["num_points"].unique()
    ):

        subset = dimension_df[
            dimension_df["num_points"] == n_points
        ].sort_values("num_processes")

        if subset.empty:
            continue

        ax.plot(
            subset["num_processes"],
            subset["median_time"],
            marker="o",
            markersize=4,
            linewidth=1.5,
            label=f"N = {n_points:,}",
        )

    ax.set_title(f"{dimension}D")
    ax.set_xlabel("Number of Processors (P)")
    ax.set_ylabel(TIME_LABEL)

    ax.set_xticks(processors_available)
    ax.set_yscale("log")

    ax.grid(
        True,
        which="both",
        linestyle="--",
        alpha=0.4,
    )

    ax.legend(
        title="Input size",
        fontsize=7,
        ncol=2,
    )

save_figure(
    fig,
    "execution_time_vs_processors_all_dimensions.png",
)