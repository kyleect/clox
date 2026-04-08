build:
    mkdir -p target
    gcc src/main.c -o target/main

clean:
    rm -rf target
