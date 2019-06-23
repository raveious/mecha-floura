#!/bin/bash

repo_dir="$(git rev-parse --show-toplevel)"
repo_name="$(basename $repo_dir)"

if [ -z "$(docker image ls | grep $repo_name)" ]; then
    echo "Creating development environment image..."
    docker build -t $repo_name .
fi

if [ -c "$1" ]; then
    echo "Mounting $1 into the container..."
    device_arg="--device $1"
fi

docker run -it --rm -u $(id -u $UID):$(id -g $UID) $device_arg -v $repo_dir:/$repo_name $repo_name
