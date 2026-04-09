cli := "target/clox"

run *args: build
    ./{{ cli }} {{ args }}

build: clean
    mkdir -p target
    gcc -c src/main.c -o target/main.o
    gcc -c src/chunk.c -o target/chunk.o
    gcc -c src/memory.c -o target/memory.o
    gcc -c src/debug.c -o target/debug.o
    gcc -c src/value.c -o target/value.o
    gcc -c src/vm.c -o target/vm.o
    gcc -c src/compiler.c -o target/compiler.o
    gcc -c src/scanner.c -o target/scanner.o
    gcc \
        target/main.o \
        target/chunk.o \
        target/memory.o \
        target/debug.o \
        target/value.o \
        target/vm.o \
        target/compiler.o \
        target/scanner.o \
        -o {{ cli }}

clean:
    rm -rf target
