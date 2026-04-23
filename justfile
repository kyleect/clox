cli := "build/clox"
docker_image := "clox"

default:
    just --list

# Run the clox CLI
run *args: build
    ./{{ cli }} {{ args }}

# Generate cmake files
cmake:
    ./scripts/cmake.sh

# Build the code
build: generate_version_c
    ./scripts/build.sh

# Clean the build artifacts
clean:
    ./scripts/clean.sh

# Run the tests
test *args: build
    ./scripts/tests.sh {{ args }}

# Update the test snapshots
test-update: build
    ./scripts/tests.sh --update

# Build the docker image
docker-build:
    docker build -t {{ docker_image }} .

# Run clox CLI in docker container
docker-run:
    docker run -it --init --name {{ docker_image }} --rm {{ docker_image }} 

# VERSION.txt => src/version.c
generate_version_c:
    ./scripts/generate_version_c.sh
