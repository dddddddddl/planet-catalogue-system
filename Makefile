CC = cc
CFLAGS ?= -std=c17 -O2 -Wall -Wextra -Wpedantic
PYTHON ?= python3
TARGET = build/planet_catalogue

.PHONY: all test clean
all: $(TARGET)

$(TARGET): src/planet_catalogue.c src/portable.h
	mkdir -p build
	$(CC) $(CFLAGS) src/planet_catalogue.c -o $(TARGET)

test: $(TARGET)
	$(PYTHON) tests/test_catalogue.py $(TARGET)

clean:
	rm -rf build
