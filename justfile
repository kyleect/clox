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

# Update the test snapshots
test-update: build
    ./tests.sh --update

# Build the docker image
docker-build:
    docker build -t {{ docker_image }} .

# Run clox CLI in docker container
docker-run:
    docker run -it --init --name {{ docker_image }} --rm {{ docker_image }} 

# VERSION.txt => src/version.h
generate_version_header:
    #!/usr/bin/env bash

    xxd -i VERSION.txt | sed 's/\([0-9a-f]\)$/\0, 0x00/' > src/version.h
    echo -e "// This is a generated file. Do not edit!\n// Generated from VERSION.txt\n\n// Language Version: $(cat VERSION.txt)\n$(cat src/version.h)" > src/version.h
