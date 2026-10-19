# Моделирование движения N тел под действием сил тяготения
#
#   make            — собрать ./program
#   make run        — Солнце и 4 планеты на 2 года, шаг 30 минут
#   make test       — тесты на задачах с известным ответом
#   make plot       — run + картинка trajectories.png (нужен matplotlib)
#   make bench      — замер времени на 1, 2, 4, 8 потоках (1000 тел, 100 шагов)
#   make clean      — удалить всё собранное

CC      = clang
CFLAGS  = -std=c17 -O2 -Wall -Wextra -Iinclude
LDFLAGS = -pthread -lm

BUILD   = build
TARGET  = task2

SRCS    = src/main.c src/cli.c src/input.c src/physics.c \
          src/barrier.c src/simulation.c src/csv_writer.c
OBJS    = $(SRCS:src/%.c=$(BUILD)/%.o)

YEAR    = 31557600
BENCH_N = 1000

.PHONY: all run test plot bench clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Любой .o пересобирается, если поменялся его .c или любой заголовок
$(BUILD)/%.o: src/%.c $(wildcard include/*.h)
	@mkdir -p $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET) $$((2 * $(YEAR))) data/inner_planets.txt 1 1800

$(BUILD)/test_nbody: tests/test_nbody.c $(BUILD)/physics.o $(BUILD)/barrier.o $(BUILD)/simulation.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

test: $(BUILD)/test_nbody
	./$(BUILD)/test_nbody

plot: run
	python3 tools/plot_trajectories.py trajectories.csv trajectories.png

data/random_$(BENCH_N).txt: tools/gen_input.py
	python3 tools/gen_input.py $(BENCH_N) $@

bench: $(TARGET) data/random_$(BENCH_N).txt
	@for t in 1 2 4 8; do \
		printf "threads = %d: " $$t; \
		./$(TARGET) 1000 data/random_$(BENCH_N).txt $$t 10 | grep Elapsed; \
	done

clean:
	rm -rf $(BUILD) $(TARGET) trajectories.csv trajectories.png data/random_*.txt
