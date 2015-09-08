#!/usr/bin/env bash

set -euo pipefail
IFS=$'\n'

# Remake
make clean
make

# Simple test
echo -e "\n\n\x1b[7mLet's check memory usage...\x1b[0m"
time valgrind bin/happy score dictionary $'flag stars are made of weird stuff'

# Model consistency checks
echo -e "\n\n\x1b[7mWord vs void\x1b[0m\n"
emptyScore=`bin/happy score dictionary ''`
echo "Score for empty strings: $emptyScore"

for w in `cat minidic`;do
    wScore=`bin/happy score dictionary "$w"`
    echo "[$w] $wScore > $emptyScore"

    [ $wScore -gt $emptyScore ]
    ok=$?
    if [ $ok -ne 0 ];then
        echo 'NOOOOOO!'
        break
    fi
done

echo -e "\n\n\x1b[7mWord vs self\x1b[0m\n"
check="this is a sample check"
firstScore=`bin/happy score dictionary "$check"`
secondScore=`bin/happy score dictionary "$check"`
echo "$firstScore == $secondScore"
[ $firstScore -eq $secondScore ]


echo -e "\n\n\x1b[7mWord vs noise\x1b[0m\n"

size=10
for w in `cat minidic`;do
    noise=`python -c "import os; print os.urandom($size)"`
    noiseScore=`bin/happy score dictionary "$noise"`

    wScore=`bin/happy score dictionary "$w"`
    echo "[$w] $wScore > $noiseScore [$size]"

    [ $wScore -gt $noiseScore ]
    ok=$?
    size=$(( $size + 500 ))
    if [ $ok -ne 0 ];then
        echo 'NOOOOOO!'
        break
    fi
done


echo -e '\nGreat!'
