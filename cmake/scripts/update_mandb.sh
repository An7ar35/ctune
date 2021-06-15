#!/bin/bash

if [[ $(/usr/bin/id -u) -ne 0 ]]; then
    printf "Not running as root\n"
    exit
else
    printf "Updating man db...\n"
    mandb --quiet
fi
