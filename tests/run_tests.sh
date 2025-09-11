#!/bin/bash
# run unit tests
cd unit && make clean-run && cd ..

# run integration tests
cd integration && make clean-run && cd ..
