FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive

# Install build dependencies and clang-tidy for static analysis
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    make \
    pkg-config \
    libcairo2-dev \
    libfreetype6-dev \
    libjpeg-dev \
    clang \
    clang-tidy \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /work
