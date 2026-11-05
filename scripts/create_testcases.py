"""Generates the benchmark datasets in DataSets/.

    python3 scripts/create_testcases.py             # every dimension and size
    python3 scripts/create_testcases.py --seed 7    # reproducible run

Each dataset holds `num_points` points with `dimensions` coordinates, drawn
uniformly from [0, 1] and rounded to six decimals for storage.
"""

import argparse
import os
import random
import time

DIMENSIONS = [5, 6, 7, 8, 9, 10]
POINT_COUNTS = [50, 100, 500, 1000, 10000, 100000, 1000000]

REPO_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def generate_test_case(dimensions, num_points, filename=None):
    lines = []

    # Header
    lines.append(f"{dimensions} {num_points}")

    # Generate points
    for _ in range(num_points):
        point = [
            f"{random.random():.6f}"
            for _ in range(dimensions)
        ]
        lines.append(" ".join(point))

    # save to file
    if filename:
        os.makedirs(os.path.dirname(filename), exist_ok=True)
        with open(filename, "w") as f:
            f.write("\n".join(lines) + "\n")


def main():
    parser = argparse.ArgumentParser(description="Generate the benchmark datasets.")
    parser.add_argument(
        "--out",
        default=os.path.join(REPO_ROOT, "DataSets"),
        help="directory to write the <D>D_Data/ folders into",
    )
    parser.add_argument(
        "--dimensions",
        type=int,
        nargs="+",
        default=DIMENSIONS,
        help="dimensions to generate",
    )
    parser.add_argument(
        "--points",
        type=int,
        nargs="+",
        default=POINT_COUNTS,
        help="point counts to generate, each written as its matching Test<N>",
    )
    parser.add_argument(
        "--seed",
        type=int,
        default=None,
        help="seed for a reproducible run; defaults to the current time",
    )
    args = parser.parse_args()

    unknown = sorted(set(args.points) - set(POINT_COUNTS))
    if unknown:
        parser.error(
            f"point counts {unknown} have no Test<N> name; "
            f"expected a subset of {POINT_COUNTS}"
        )

    random.seed(args.seed if args.seed is not None else int(time.time()))

    for dimensions in args.dimensions:
        for test, num_points in enumerate(POINT_COUNTS, start=1):
            if num_points not in args.points:
                continue
            filename = os.path.join(
                args.out, f"{dimensions}D_Data", f"Test{test}.txt"
            )
            generate_test_case(
                dimensions=dimensions,
                num_points=num_points,
                filename=filename,
            )
            print(f"{dimensions}D {num_points:>8} points -> {filename}")


if __name__ == "__main__":
    main()
