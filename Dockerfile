FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive

# Install build dependencies, Clazy, and Qt5 for static analysis
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    make \
    pkg-config \
    libcairo2-dev \
    libfreetype6-dev \
    libjpeg-dev \
    clang \
    libclang-dev \
    clazy \
    qt5-qmake \
    qtbase5-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /work
