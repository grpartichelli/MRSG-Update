#!/bin/bash

export LD_LIBRARY_PATH=$HOME/simgrid-3.11/lib

./hello_mrsg.bin 2>&1 | $HOME/simgrid-3.11/bin/simgrid-colorizer
