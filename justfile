cli := "build/clox"
docker_image := "clox"

default:
    just --list

# Run the clox CLI
run *args: build
    ./{{ cli }} {{ args }}

# Generate cmake files
cmake:
    mkdir -p build
    cd build && cmake ..

# Build the code
build:
    mkdir -p build
    cd build && make

# Clean the build artifacts
clean:
    cd build && make clean

# Run the tests
test *args: build
    ./tests.sh {{ args }}

# Build the docker image
docker-build:
    docker build -t {{ docker_image }} .

# Run clox CLI in docker container
docker-run:
    docker run -it --init --name {{ docker_image }} --rm {{ docker_image }} 
