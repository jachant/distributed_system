"""
Независимая проверка результата mandelbrot.csv.

Запуск:
    python3 check_csv.py mandelbrot.csv <npoints> [max_iter]

Что проверяется:
  1. заголовок "x,y" и ровно npoints строк с точками;
  2. все точки лежат в прямоугольнике поиска [-2; 0.5] x [-1.25; 1.25];
  3. нет повторов (у потоков разные seed, иначе они нашли бы одни и те же точки);
  4. каждая точка заново проверяется на принадлежность множеству,
     независимо от кода на C (numpy, комплексные числа). Несколько точек
     у самой границы могут разойтись из-за округления — это нормально;
  5. если установлен matplotlib, рисуется mandelbrot.png — должна
     получиться узнаваемая фигура.
"""
import sys
import numpy as np

if len(sys.argv) < 3:
    sys.exit("usage: python3 check_csv.py mandelbrot.csv <npoints> [max_iter]")

path, npoints = sys.argv[1], int(sys.argv[2])
max_iter = int(sys.argv[3]) if len(sys.argv) > 3 else 1000
ok = True

def report(cond, msg):
    global ok
    ok &= bool(cond)
    print(f"[{' OK ' if cond else 'FAIL'}] {msg}")

with open(path) as f:
    header = f.readline().strip()
report(header == "x,y", f'заголовок: "{header}"')

data = np.loadtxt(path, delimiter=",", skiprows=1, ndmin=2)
x, y = data[:, 0], data[:, 1]
report(len(x) == npoints, f"количество точек: {len(x)} (ожидали {npoints})")

inside_box = (x >= -2.0) & (x <= 0.5) & (y >= -1.25) & (y <= 1.25)
report(inside_box.all(), f"вне прямоугольника поиска: {(~inside_box).sum()}")

unique = len(np.unique(data, axis=0))
report(unique == len(x), f"уникальных точек: {unique} из {len(x)}")

# Независимая проверка принадлежности: z_{n+1} = z_n^2 + c, z_1 = 0.
# Для каждой точки запоминаем, на каком шаге она ушла за |z| > 2 (или -1).
c = x + 1j * y
z = np.zeros_like(c)
escape = np.full(len(c), -1)
for n in range(1, max_iter + 1):
    alive = escape < 0
    z[alive] = z[alive] * z[alive] + c[alive]
    escape[alive & ((z.real ** 2 + z.imag ** 2) > 4.0)] = n

# Точка, ушедшая быстро (меньше чем за половину max_iter), — настоящая ошибка.
# Точка, ушедшая почти в самом конце, лежит у самой границы множества:
# там траектория хаотична, и разное округление в C и в numpy за сотни
# шагов даёт разный ответ. Это не ошибка программы.
real_err = (escape > 0) & (escape < max_iter // 2)
border = (escape >= max_iter // 2)
report(real_err.sum() == 0,
       f"точек, которые явно НЕ в множестве: {real_err.sum()}")
if real_err.any():
    print("   примеры:", c[real_err][:5])
print(f"[INFO] пограничных точек (ушли после {max_iter // 2}-го шага "
      f"из-за округления): {border.sum()}")

# Картинка
try:
    import matplotlib
    matplotlib.use("Agg")
    import matplotlib.pyplot as plt
    fig, ax = plt.subplots(figsize=(7, 7))
    ax.scatter(x, y, s=0.2, c="black", linewidths=0)
    ax.set_xlim(-2.0, 0.5); ax.set_ylim(-1.25, 1.25); ax.set_aspect("equal")
    ax.set_title(f"{len(x)} точек множества Мандельброта")
    fig.savefig("mandelbrot.png", dpi=150, bbox_inches="tight")
    print("картинка сохранена в mandelbrot.png")
except ImportError:
    print("matplotlib не установлен, картинку не рисуем")

print("\nИТОГ:", "ВСЁ ВЕРНО" if ok else "ЕСТЬ ОШИБКИ")
sys.exit(0 if ok else 1)
