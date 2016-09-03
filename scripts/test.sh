#!/bin/bash
SCRIPT_DIR="$(dirname "$(readlink -f "$0")")"
make bin/exp
rm data/exp*.dat
bin/exp
bin/exp
bin/exp
bin/exp
bin/exp
bin/exp
bin/exp
