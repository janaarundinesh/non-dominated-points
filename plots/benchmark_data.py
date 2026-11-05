"""Shared paths and loading for the benchmark CSVs produced by the drivers."""

from pathlib import Path

import pandas as pd


BASE_DIR = Path(__file__).resolve().parent
PROJECT_DIR = BASE_DIR.parent

RESULTS_DIR = PROJECT_DIR / "results"
FIGURES_DIR = BASE_DIR / "figures"
DATA_DIR = BASE_DIR / "data"

MAXIMA_CSV = RESULTS_DIR / "maxima_benchmark.csv"
BRUTE_FORCE_CSV = RESULTS_DIR / "brute_force_benchmark.csv"
MARRY_GO_ROUND_CSV = RESULTS_DIR / "mpi_marry_go_round_benchmark.csv"


def require(*paths):
    """Exit with an actionable message when a benchmark run is missing."""
    missing = [str(path) for path in paths if not path.exists()]

    if missing:
        raise SystemExit(
            "Missing benchmark data: " + ", ".join(missing) + "\n"
            "Run the Maxima_method and brute_force drivers first "
            "(see README, section 7)."
        )


def load_parallel_benchmark(path, processes):
    """Read the MPI benchmark CSV for one process count.

    That CSV stores execution times in seconds, not microseconds, so it is
    loaded separately from load_benchmark. One row per (dimension, N) holds the
    median over the repetitions of that configuration.
    """
    frame = pd.read_csv(path)
    frame = frame[frame["num_processes"] == processes]

    return (
        frame.groupby(["dimensions", "num_points"], as_index=False)
        .agg(execution_time_s=("execution_time", "median"))
        .sort_values("num_points")
    )


def load_benchmark(path):
    """Read one benchmark CSV and normalise its columns.

    Older files have no dispersion columns; those fall back to the median so
    that the plotting code does not need two branches.
    """
    frame = pd.read_csv(path)

    for column in ("min_execution_time", "max_execution_time"):
        if column not in frame:
            frame[column] = frame["execution_time"]

    frame["execution_time_s"] = frame["execution_time"] / 1_000_000
    frame["min_s"] = frame["min_execution_time"] / 1_000_000
    frame["max_s"] = frame["max_execution_time"] / 1_000_000

    # Asymmetric error bars around the median, clipped at zero.
    frame["lower_error"] = (frame["execution_time_s"] - frame["min_s"]).clip(lower=0)
    frame["upper_error"] = (frame["max_s"] - frame["execution_time_s"]).clip(lower=0)

    return frame
