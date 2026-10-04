import random
import time

def generate_test_case(dimensions, num_points, filename=None):
    seed = int(time.time())
    random.seed(seed)

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
        with open(filename, "w") as f:
            f.write("\n".join(lines) + "\n")


D = 10
N = 1000000
savelocation = "./10D_Data/Test6.txt"

generate_test_case(dimensions = D, num_points = N,filename = savelocation)