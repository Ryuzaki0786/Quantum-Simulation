import numpy as np
import matplotlib.pyplot as plt

# Each row = heat displacement across all grid points at one timestep
data = np.loadtxt("heat.csv", delimiter=",")

print(f"Snapshots (timesteps): {data.shape[0]}")
print(f"Grid points (space):   {data.shape[1]}")

# Plot a few snapshots overlaid, to SEE the wave evolve
plt.figure(figsize=(10, 6))
snapshots = [0, 50, 100, 150, 175]   # which timesteps to draw
for n in snapshots:
    plt.plot(data[n], label=f"step {n}")

plt.xlabel("Grid point (space)")
plt.ylabel("Temperature u")
plt.title("1D Heat Equation — step function diffusing")
plt.legend()
plt.grid(True, alpha=0.3)
plt.savefig("heat_snapshots.png", dpi=120)
plt.show()