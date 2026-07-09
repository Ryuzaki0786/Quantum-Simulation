import numpy as np
import matplotlib.pyplot as plt

data = np.loadtxt("schrodinger_output.csv", delimiter=",")   # rows = snapshots, cols = |psi|^2 at each x

length = 1.0
n_points = data.shape[1]
dx = length / (n_points - 1)
x = np.linspace(0, length, n_points)

center = 0.5            # whatever you set
width  = 0.02           # whatever you set
barrier_right = center + width / 2.0

past  = x > barrier_right
final = data[-1]
T = np.sum(final[past]) / np.sum(final)
print(f"Transmission coefficient T = {T:.6e}")

# --- Check 1 & 2: motion + spreading (visual) ---
plt.figure(figsize=(10, 6))
n_snaps = data.shape[0]
for i in range(0, n_snaps, max(1, n_snaps // 8)):   # overlay ~8 snapshots
    plt.plot(x, data[i], label=f"snapshot {i}")
plt.xlabel("x")
plt.ylabel(r"$|\psi|^2$")
plt.title("Schrodinger wavepacket evolution (Crank-Nicolson)")
plt.legend()
plt.savefig("schrodinger_evolution.png", dpi=120)
plt.show()

# --- Check 3: probability conservation (the payoff) ---
print("Total probability per snapshot (should stay ~constant):")
for i in range(n_snaps):
    total = np.sum(data[i]) * dx
    print(f"  snapshot {i:3d}:  {total:.6f}")