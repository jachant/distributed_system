"""
Генератор случайного входного файла: скопление из n тел.

Запуск:
    python3 tools/gen_input.py <n> <output_file> [seed]

Массы ~1e22 кг (крупные астероиды), координаты в кубе со стороной 1e9 м,
скорости до 50 м/с. Подходит для замеров времени на большом n.
"""
import random
import sys

if len(sys.argv) < 3:
    sys.exit("usage: python3 tools/gen_input.py <n> <output_file> [seed]")

n, path = int(sys.argv[1]), sys.argv[2]
random.seed(int(sys.argv[3]) if len(sys.argv) > 3 else 1)

with open(path, "w") as f:
    f.write(f"{n}\n")
    for _ in range(n):
        m = 1e22 * (1 + random.random())
        pos = [1e9 * (random.random() - 0.5) for _ in range(3)]
        vel = [100 * (random.random() - 0.5) for _ in range(3)]
        f.write(" ".join(f"{v:.6e}" for v in [m, *pos, *vel]) + "\n")
