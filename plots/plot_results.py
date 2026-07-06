from pathlib import Path
import pandas as pd
import matplotlib.pyplot as plt

BASE_DIR = Path(__file__).resolve().parent  # .../plots
PROJECT_DIR = BASE_DIR.parent               # .../non-dominated-points

RESULTS_DIR = PROJECT_DIR / 'results'
FIGURES_DIR = BASE_DIR / 'figures'

# Create the figures directory if it doesn't exist
FIGURES_DIR.mkdir(exist_ok=True)

# Read the CSV file
df = pd.read_csv(RESULTS_DIR / 'benchmark_results.csv')

#create figure
plt.figure(figsize=(10, 6))

# Plotting for each dimension
for dim in df['Dimension'].unique():
    subset = df[df['Dimension'] == dim]
    plt.plot(subset['NPoints'], subset['ExecutionTime_us'], marker='o', label=f'Dimension {dim}')

plt.xlabel('Number of Points')
plt.ylabel('Execution  Time (microseconds)')
plt.title('Runtime vs Number of Points for Different Dimensions')
plt.legend()
plt.grid(True)

# Save the plot as a PNG file
plt.savefig(FIGURES_DIR / 'runtime.png', dpi=300)