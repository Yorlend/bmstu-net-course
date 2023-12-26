CC = gcc
CFLAGS = -Iinc -g -ggdb
SRC = $(wildcard src/*.c)
OBJ = $(SRC:src/%.c=build/%.o)

build: $(OBJ)
	$(CC) -o build/app $^

build/%.o: src/%.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

run:
	cd static && ../build/app

clean:
	rm -rf build

.PHONY: build clean
