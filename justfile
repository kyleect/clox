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

test *args: build
    ./tests.sh {{ args }}

docker-build:
    docker build -t clox .

docker-run:
    docker run -it --init --name clox --rm clox 
