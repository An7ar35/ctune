#!/bin/bash

printf "Build type: '%s'\n" "$2"

if [[ "$2" == "Release" ]]; then
  if [[ $(/usr/bin/id -u) -ne 0 ]]; then
      printf "Not running as root\n"
      exit
  else
      printf "Updating mandb...\n"
      mandb --quiet
  fi
else
  printf "Skipping mandb update.\n"
fi
