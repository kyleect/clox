run: build
    ./target/clox

build: clean
    mkdir -p target
    gcc -c src/main.c -o target/main.o
    gcc -c src/chunk.c -o target/chunk.o
    gcc -c src/memory.c -o target/memory.o
    gcc -c src/debug.c -o target/debug.o
    gcc -c src/value.c -o target/value.o
    gcc \
        target/main.o \
        target/chunk.o \
        target/memory.o \
        target/debug.o \
        target/value.o \
        -o target/clox

clean:
    rm -rf target
