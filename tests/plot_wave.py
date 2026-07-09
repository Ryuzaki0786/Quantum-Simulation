import numpy as np
import matplotlib.pyplot as plt

# Each row = wave displacement across all grid points at one timestep
data = np.loadtxt("wave.csv", delimiter=",")

print(f"Snapshots (timesteps): {data.shape[0]}")
print(f"Grid points (space):   {data.shape[1]}")

# Plot a few snapshots overlaid, to SEE the wave evolve
plt.figure(figsize=(10, 6))
snapshots = [0, 50, 100, 150, 200, 300]   # which timesteps to draw
for n in snapshots:
    plt.plot(data[n], label=f"step {n}")

plt.xlabel("Grid point (space)")
plt.ylabel("Displacement u")
plt.title("1D Wave Equation — Gaussian pulse splitting and reflecting")
plt.legend()
plt.grid(True, alpha=0.3)
plt.savefig("wave_snapshots.png", dpi=120)
plt.show()