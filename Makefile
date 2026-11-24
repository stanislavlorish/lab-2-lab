CC ?= gcc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic
TARGET := farmer
SOURCES := main.c input.c game_time.c inventory.c variant_sort.c item_catalog.c diary.c resource_report.c adventure.c
HEADERS := $(wildcard *.h)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET)

clean:
	rm -f $(TARGET) $(TARGET).exe *.o
