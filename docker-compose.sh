#!/bin/bash

declare -A MAP=(
  ["arch"]="build-arch"
  ["ubuntu"]="build-ubuntu"
)

if [[ -n "${MAP[$1]}" ]]; then
  printf "Launching docker build: %s\n" "${MAP[$1]}"
  HOST_UID="$(id -u)" HOST_GID="$(id -g)" HOST_UNAME="$(whoami)" AUDIO_GID="$(getent group audio | cut -d: -f3)" docker-compose -f docker-compose.yml run --rm "${MAP[$1]}"
  docker rmi ctune-"${MAP[$1]}":latest
else
  printf "Invalid argument. Builds available:\n"
  for i in "${!MAP[@]}"; do
    printf "  > %s\n" "$i"
  done
fi