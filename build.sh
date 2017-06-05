#!/bin/bash

if [ ! -d "build" ]; then
    mkdir build
fi

output="$PWD/build/"

cmake -H"$PWD/" -B"$output"

cmake --build "$output"