#!/bin/bash

# docker run --rm --net=host --ipc=host --pid=host --dns 192.168.1.1 -m 5g --name manus_ros2 -it manus_ros2

docker stop manus_ros2
docker rm manus_ros2
docker run --net=host --ipc=host --pid=host --dns 192.168.1.1 --name manus_ros2 --restart unless-stopped -it manus_ros2 
