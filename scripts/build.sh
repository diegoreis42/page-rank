#!/bin/bash
set -e

ROOT_DIR=$(dirname $(dirname $(readlink -fm $0)))
cd $ROOT_DIR
mkdir -p build
cp -r res/* build
cd build
cmake ..
make -j 8
