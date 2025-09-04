#!/bin/bash
# build integration tests
cd integration && make clean && make all && cd ..

# build unit tests
cd unit && make clean && make all && cd ..

# clear the console.
clear

# run unit tests
cd unit && make run && cd ..

# run integration tests
cd integration && make run && cd ..
