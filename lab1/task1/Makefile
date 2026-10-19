# Параллельный поиск точек множества Мандельброта
#
#   make            — собрать ./program
#   make run        — запустить на 4 потоках и 100000 точках
#   make test       — тесты на известных точках
#   make check      — запустить программу и проверить CSV (нужен python3 + numpy)
#   make bench      — замер времени на 1, 2, 4, 8 потоках
#   make clean      — удалить всё собранное

CC      = clang
CFLAGS  = -std=c17 -O2 -Wall -Wextra -Iinclude
LDFLAGS = -pthread

BUILD   = build
TARGET  = task1

SRCS    = src/main.c src/cli.c src/mandelbrot.c src/search.c \
          src/csv_writer.c src/my_rand.c
OBJS    = $(SRCS:src/%.c=$(BUILD)/%.o)

THREADS ?= 4
POINTS  ?= 100000

.PHONY: all run test check bench clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Любой .o пересобирается, если поменялся его .c или любой заголовок
$(BUILD)/%.o: src/%.c $(wildcard include/*.h)
	@mkdir -p $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET) $(THREADS) $(POINTS)

$(BUILD)/test_mandelbrot: tests/test_mandelbrot.c $(BUILD)/mandelbrot.o
	$(CC) $(CFLAGS) -o $@ $^ -lm

test: $(BUILD)/test_mandelbrot
	./$(BUILD)/test_mandelbrot

check: $(TARGET)
	./$(TARGET) $(THREADS) $(POINTS)
	python3 tests/check_csv.py mandelbrot.csv $(POINTS)

bench: $(TARGET)
	@for t in 1 2 4 8; do \
		printf "threads = %d: " $$t; \
		./$(TARGET) $$t $(POINTS) | grep Elapsed; \
	done

clean:
	rm -rf $(BUILD) $(TARGET) mandelbrot.csv mandelbrot.png