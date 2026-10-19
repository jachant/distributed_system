"""
Рисует траектории из trajectories.csv (нужен matplotlib).

Запуск:
    python3 tools/plot_trajectories.py [trajectories.csv] [trajectories.png]
"""
import csv
import sys

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

src = sys.argv[1] if len(sys.argv) > 1 else "trajectories.csv"
dst = sys.argv[2] if len(sys.argv) > 2 else "trajectories.png"

with open(src) as f:
    rows = list(csv.reader(f))
header, data = rows[0], [list(map(float, r)) for r in rows[1:]]
n = (len(header) - 1) // 2

fig, ax = plt.subplots(figsize=(7, 7))
for k in range(n):
    xs = [r[1 + 2 * k] for r in data]
    ys = [r[2 + 2 * k] for r in data]
    ax.plot(xs, ys, linewidth=0.8)
    ax.plot(xs[-1], ys[-1], "o", markersize=3, color=ax.lines[-1].get_color())
ax.set_aspect("equal")
ax.set_xlabel("x, м")
ax.set_ylabel("y, м")
ax.set_title(f"Траектории {n} тел, t от 0 до {data[-1][0]:g} с")
fig.savefig(dst, dpi=150, bbox_inches="tight")
print(f"saved {dst}")
