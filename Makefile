CC = cc
CFLAGS = -o kryptori
SOURCES = main.c
TARGET = kryptori 

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(SOURCES) $(CFLAGS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all clean

