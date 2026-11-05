"""Measures the MPI marry-go-round on the worst-case antichain.

The antichain input is written by `antichain_dataset`, so the MPI rows describe
exactly the same points the serial worst-case rows were measured on. Each
configuration is repeated and reduced to a median, then upserted into the same
CSV the serial sweep writes, tagged `mpi_marry_go_round`.

    python3 scripts/worst_case_mpi_sweep.py \
        --mpi-binary build/MPI_Marrygomethod \
        --dataset-tool build/antichain_dataset

Run from a build configured with -DNDP_ENABLE_MPI=ON.
"""

import argparse
import csv
import os
import statistics
import subprocess
import tempfile
from pathlib import Path

PROJECT_DIR = Path(__file__).resolve().parent.parent
DEFAULT_OUTPUT = PROJECT_DIR / "plots" / "data" / "worst_case_all_maxima.csv"

COLUMNS = [
    "dimensions",
    "num_points",
    "algorithm",
    "median_ms",
    "min_ms",
    "max_ms",
    "maxima",
    "maxima_fraction",
]

ALGORITHM = "mpi_marry_go_round"


def parse_run(stdout):
    """Pull the total time and the maxima count out of one MPI run."""
    total = None
    maxima = None

    for token in stdout.split():
        if "=" not in token:
            continue

        key, _, value = token.partition("=")

        try:
            if key == "total":
                total = float(value)
            elif key == "maxima":
                maxima = int(value)
        except ValueError:
            continue

    if total is None or maxima is None:
        raise RuntimeError(f"could not parse the run output: {stdout!r}")

    return total, maxima


def upsert(output_path, row):
    """Replace the row for this (dimensions, num_points, algorithm), if any."""
    rows = []

    if output_path.exists():
        with open(output_path, newline="", encoding="utf-8") as existing:
            rows = [
                r
                for r in csv.DictReader(existing)
                if (int(r["dimensions"]), int(r["num_points"]), r["algorithm"])
                != (row["dimensions"], row["num_points"], row["algorithm"])
            ]

    rows.append(row)
    rows.sort(key=lambda r: (int(r["dimensions"]), int(r["num_points"]), r["algorithm"]))

    with open(output_path, "w", newline="", encoding="utf-8") as handle:
        writer = csv.DictWriter(handle, fieldnames=COLUMNS)
        writer.writeheader()
        writer.writerows(rows)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--mpi-binary", required=True)
    parser.add_argument("--dataset-tool", required=True)
    parser.add_argument("--output", default=str(DEFAULT_OUTPUT))
    parser.add_argument("--processes", type=int, default=8)
    parser.add_argument("--repetitions", type=int, default=3)
    parser.add_argument("--dimensions", type=int, nargs="+", default=[5, 6, 7, 8, 9, 10])
    parser.add_argument(
        "--points", type=int, nargs="+", default=[50, 100, 500, 1000, 10000, 100000]
    )
    args = parser.parse_args()

    output_path = Path(args.output)
    output_path.parent.mkdir(parents=True, exist_ok=True)

    # The program writes its own CSVs to ../results relative to the working
    # directory, so it needs a scratch tree shaped like a checkout.
    workdir = Path(tempfile.mkdtemp(prefix="ndp_worst_case_"))
    (workdir / "datasets").mkdir()
    (workdir / "run" / "results").mkdir(parents=True)
    (workdir / "run" / "build").mkdir()

    for dimensions in args.dimensions:
        for points in args.points:
            dataset = workdir / "datasets" / f"{dimensions}D_{points}.txt"

            subprocess.run(
                [args.dataset_tool, str(dataset), str(dimensions), str(points)],
                check=True,
                stdout=subprocess.DEVNULL,
            )

            samples = []
            maxima = None

            for _ in range(args.repetitions):
                completed = subprocess.run(
                    [
                        "mpirun",
                        "-np",
                        str(args.processes),
                        os.path.abspath(args.mpi_binary),
                        str(dataset),
                    ],
                    cwd=workdir / "run" / "build",
                    check=True,
                    capture_output=True,
                    text=True,
                    timeout=300,
                )

                total, maxima = parse_run(completed.stdout)
                samples.append(total * 1000.0)  # seconds -> milliseconds

            if maxima != points:
                raise SystemExit(
                    f"{dimensions}D N={points}: only {maxima} of {points} points "
                    "are maximal, so the input is not an antichain"
                )

            row = {
                "dimensions": dimensions,
                "num_points": points,
                "algorithm": ALGORITHM,
                "median_ms": statistics.median(samples),
                "min_ms": min(samples),
                "max_ms": max(samples),
                "maxima": maxima,
                "maxima_fraction": maxima / points,
            }

            upsert(output_path, row)

            print(
                f"{dimensions}D N={points:>7} P={args.processes} "
                f"median={row['median_ms']:.2f} ms"
            )


if __name__ == "__main__":
    main()
