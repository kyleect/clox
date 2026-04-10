cli := "build/clox"

run *args: build
    ./{{ cli }} {{ args }}

cmake:
    mkdir -p build
    cd build && cmake ..

build:
    mkdir -p build
    cd build && make

clean:
    cd build && make clean
