# syntax=docker/dockerfile:1
FROM archlinux:latest
LABEL maintainers="E.A.Davison <eadavison at protonmail dot com>"

# Base install
RUN pacman-key --init \
 && pacman -Syu --noconfirm \
 && pacman -S --noconfirm gcc git make cmake ffmpeg openssl curl ncurses libbsd libpulse

# Build & install ctune
WORKDIR /ctune
COPY . .
RUN mkdir cmake-build-docker \
 && cd cmake-build-docker \
 && cmake ../ \
 && cd .. \
 && cmake . -DCMAKE_BUILD_TYPE=Release \
 && cmake --build . \
 && cmake --install .

# Cleanup
RUN cd / \
 && rm -rf /ctune \
 && pacman -Rsuc --noconfirm gcc git make cmake

# Execute
ENTRYPOINT [ "ctune" ]
CMD [ "--debug" ]