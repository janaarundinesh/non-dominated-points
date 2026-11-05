"""Measures the MPI marry-go-round on the worst-case antichain.

The antichain input is written by `antichain_dataset`, so the MPI rows describe
exactly the same points the serial worst-case rows were measured on.

Two outputs:

* the per-configuration rows for one process count are upserted into the
  worst-case CSV the serial sweep writes, tagged `mpi_marry_go_round`, which is
  what the runtime panel plots;
* every phase of every run is written to the scaling CSV, so the question of
  which component grows with the process count can be answered from the data.

    python3 scripts/worst_case_mpi_sweep.py \
        --mpi-binary build/MPI_Marrygomethod \
        --dataset-tool build/antichain_dataset \
        --processes 1 2 4 8 16 \
        --dimensions 5 10 --points 10000 100000

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
DEFAULT_SCALING_OUTPUT = PROJECT_DIR / "plots" / "data" / "worst_case_mpi_scaling.csv"

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

PHASES = {
    "total": "median_total_ms",
    "local": "median_local_ms",
    "cross": "median_crossfilter_ms",
    "comm": "median_communication_ms",
    "barrier": "median_barrier_ms",
    "scatter": "median_scatter_ms",
    "gather": "median_gather_ms",
}

SCALING_COLUMNS = [
    "dimensions",
    "num_points",
    "num_processes",
    *PHASES.values(),
    "maxima",
    "repetitions",
]


def parse_run(stdout):
    """Pull the timings and the maxima count out of one MPI run."""
    values = {}

    for token in stdout.split():
        if "=" not in token:
            continue

        key, _, value = token.partition("=")

        if key not in PHASES and key != "maxima":
            continue

        try:
            values[key] = int(value) if key == "maxima" else float(value)
        except ValueError:
            continue

    if "total" not in values or "maxima" not in values:
        raise RuntimeError(f"could not parse the run output: {stdout!r}")

    return values


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
    rows.sort(
        key=lambda r: (int(r["dimensions"]), int(r["num_points"]), r["algorithm"])
    )
    write_csv(output_path, COLUMNS, rows)


def upsert_scaling(output_path, row):
    """Replace the row for this (dimensions, num_points, num_processes), if any."""
    rows = []

    if output_path.exists():
        with open(output_path, newline="", encoding="utf-8") as existing:
            rows = [
                r
                for r in csv.DictReader(existing)
                if (
                    int(r["dimensions"]),
                    int(r["num_points"]),
                    int(r["num_processes"]),
                )
                != (row["dimensions"], row["num_points"], row["num_processes"])
            ]

    rows.append(row)
    rows.sort(
        key=lambda r: (
            int(r["dimensions"]),
            int(r["num_points"]),
            int(r["num_processes"]),
        )
    )
    write_csv(output_path, SCALING_COLUMNS, rows)


def write_csv(path, columns, rows):
    path.parent.mkdir(parents=True, exist_ok=True)

    with open(path, "w", newline="", encoding="utf-8") as handle:
        writer = csv.DictWriter(handle, fieldnames=columns)
        writer.writeheader()
        writer.writerows(rows)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--mpi-binary", required=True)
    parser.add_argument("--dataset-tool", required=True)
    parser.add_argument("--output", default=str(DEFAULT_OUTPUT))
    parser.add_argument("--scaling-output", default=str(DEFAULT_SCALING_OUTPUT))
    parser.add_argument("--processes", type=int, nargs="+", default=[8])
    parser.add_argument(
        "--all-maxima-processes",
        type=int,
        default=8,
        help="which process count is written to the runtime CSV",
    )
    parser.add_argument("--repetitions", type=int, default=3)
    parser.add_argument(
        "--dimensions", type=int, nargs="+", default=[5, 6, 7, 8, 9, 10]
    )
    parser.add_argument(
        "--points", type=int, nargs="+", default=[50, 100, 500, 1000, 10000, 100000]
    )
    args = parser.parse_args()

    output_path = Path(args.output)
    scaling_path = Path(args.scaling_output)

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

            for processes in args.processes:
                samples = []

                for _ in range(args.repetitions):
                    completed = subprocess.run(
                        [
                            "mpirun",
                            "-np",
                            str(processes),
                            os.path.abspath(args.mpi_binary),
                            str(dataset),
                        ],
                        cwd=workdir / "run" / "build",
                        check=True,
                        capture_output=True,
                        text=True,
                        timeout=300,
                    )

                    samples.append(parse_run(completed.stdout))

                maxima = samples[0]["maxima"]

                if max(sample["maxima"] for sample in samples) != points:
                    raise SystemExit(
                        f"{dimensions}D N={points}: only {maxima} of {points} points "
                        "are maximal, so the input is not an antichain"
                    )

                medians = {
                    column: statistics.median(
                        sample[key] * 1000.0 for sample in samples
                    )
                    for key, column in PHASES.items()
                }

                upsert_scaling(
                    scaling_path,
                    {
                        "dimensions": dimensions,
                        "num_points": points,
                        "num_processes": processes,
                        **{k: round(v, 4) for k, v in medians.items()},
                        "maxima": maxima,
                        "repetitions": len(samples),
                    },
                )

                totals = [sample["total"] * 1000.0 for sample in samples]

                if processes == args.all_maxima_processes:
                    upsert(
                        output_path,
                        {
                            "dimensions": dimensions,
                            "num_points": points,
                            "algorithm": ALGORITHM,
                            "median_ms": statistics.median(totals),
                            "min_ms": min(totals),
                            "max_ms": max(totals),
                            "maxima": maxima,
                            "maxima_fraction": maxima / points,
                        },
                    )

                print(
                    f"{dimensions:2d}D N={points:>7} P={processes:>2} "
                    f"total={medians['median_total_ms']:8.2f} ms "
                    f"gather={medians['median_gather_ms']:8.2f} ms "
                    f"comm={medians['median_communication_ms']:8.2f} ms "
                    f"local={medians['median_local_ms']:8.2f} ms"
                )


if __name__ == "__main__":
    main()
