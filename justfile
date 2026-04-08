build:
    mkdir -p target
    gcc -c src/main.c -o target/main.o
    gcc -c src/chunk.c -o target/chunk.o
    gcc -c src/memory.c -o target/memory.o
    gcc target/main.o target/chunk.o target/memory.o -o target/clox

clean:
    rm -rf target
