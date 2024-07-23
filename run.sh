#!/bin/bash

docker run --rm --net=host --ipc=host --pid=host --dns 192.168.1.1 --name manus_ros2 -it manus_ros2
