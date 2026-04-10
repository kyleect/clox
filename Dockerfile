# -------- Build Stage --------
FROM ubuntu:22.04 AS builder

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .

RUN mkdir -p build && cd build && cmake .. && make

# -------- Runtime Stage --------
FROM ubuntu:22.04

WORKDIR /app

# Copy only the compiled binary
COPY --from=builder /app/build/clox .

CMD ["./clox"]