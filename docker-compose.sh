HOST_UID="$(id -u)" HOST_GID="$(id -g)" HOST_UNAME="$(whoami)" docker-compose -f docker-compose.yml run --rm build
docker rmi ctune-build:latest