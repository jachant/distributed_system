# Своя блокировка чтения-записи на примере односвязного списка
#
#   make            — собрать ./program
#   make run        — 4 потока, своя блокировка, нагрузка как в примере ниже
#   make test       — тесты обеих блокировок и списка
#   make bench      — сравнение lib и my на 1, 2, 4, 8 потоках, 3 нагрузки
#   make clean      — удалить всё собранное
#
# Параметры нагрузки (stdin программы):
#   ключей в начальном списке, всего операций, доля поисков, доля вставок

CC      = clang
# _POSIX_C_SOURCE: при строгом -std=c17 без него в Linux не видны
# pthread_rwlock_* и nanosleep (на macOS они видны и так)
CFLAGS  = -std=c17 -D_POSIX_C_SOURCE=200809L -O2 -Wall -Wextra -Iinclude
LDFLAGS = -pthread

BUILD   = build
TARGET  = task3

SRCS    = src/main.c src/cli.c src/list.c src/workload.c \
          src/my_rwlock.c src/rwlock_api.c src/my_rand.c
OBJS    = $(SRCS:src/%.c=$(BUILD)/%.o)

# Нагрузка для make run: 1000 ключей, 100000 операций,
# 80% поиск, 10% вставка, 10% удаление
WORKLOAD ?= 1000 100000 0.8 0.1

# Нагрузки для make bench (через «;»): только чтение, почти только чтение
# (как в примере из pth_ll_rwl.c) и много записей
BENCH_WORKLOADS = 1000 1000000 1.0 0.0;1000 1000000 0.999 0.0005;1000 100000 0.8 0.1

.PHONY: all run test bench clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Любой .o пересобирается, если поменялся его .c или любой заголовок
$(BUILD)/%.o: src/%.c $(wildcard include/*.h)
	@mkdir -p $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	echo $(WORKLOAD) | ./$(TARGET) 4 my

TEST_OBJS = $(BUILD)/list.o $(BUILD)/workload.o $(BUILD)/my_rwlock.o \
            $(BUILD)/rwlock_api.o $(BUILD)/my_rand.o

$(BUILD)/test_rwlock: tests/test_rwlock.c $(TEST_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

test: $(BUILD)/test_rwlock
	./$(BUILD)/test_rwlock

bench: $(TARGET)
	@echo "$(BENCH_WORKLOADS)" | tr ';' '\n' | while read wl; do \
		echo "Нагрузка (ключей, операций, доля поисков, доля вставок): $$wl"; \
		printf "  %-8s %10s %10s\n" threads lib my; \
		for t in 1 2 4 8; do \
			lib=$$(echo $$wl | ./$(TARGET) $$t lib | awk '/Elapsed/ {print $$4}'); \
			my=$$(echo $$wl  | ./$(TARGET) $$t my  | awk '/Elapsed/ {print $$4}'); \
			printf "  %-8s %10.4f %10.4f\n" $$t $$lib $$my; \
		done; \
	done

clean:
	rm -rf $(BUILD) $(TARGET)
