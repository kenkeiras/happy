#!/usr/bin/env bash

set -euo pipefail

# Remake
make clean
make

# Simple test
echo -e "\n\n\x1b[7mLet's try to cat\x1b[0m"
time bin/happy dictionary $'flag stars are made of weird stuff'

# Little more complex
echo -e "\n\n\x1b[7mThen let's add one\x1b[0m"
time bin/happy dictionary $'ek`f\x1frs`qr\x1f`qd\x1fl`cd\x1fne\x1fvdhqc\x1frstee'

# A “tiny” bit more
echo -e "\n\n\x1b[7mROT13 it\x1b[0m"
time bin/happy dictionary $'synt fgnef ner znqr bs jrveq fghss'
