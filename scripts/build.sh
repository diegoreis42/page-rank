#!/bin/bash
cd ..

set -e

mkdir -p build
cd build
cmake ..
make
