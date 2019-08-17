#!/bin/bash

repo_dir="$(git rev-parse --show-toplevel)"
repo_name="$(basename $repo_dir)"

if [ -z "$(docker image ls | grep $repo_name)" ]; then
    echo "Creating development environment image..."
    docker build -t $repo_name .
fi

for dev in "$@"; do
    if [ -c "$dev" ]; then
        echo "Mounting $dev into the container..."
        device_arg="$device_arg --device $dev"
    fi
done

docker run -it --rm -u $(id -u $UID):$(id -g $UID) --group-add dialout $device_arg -v $repo_dir:/$repo_name $repo_name
