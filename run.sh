#!/bin/bash

docker run --net=host --ipc=host --pid=host --dns 192.168.1.1 --name manus_ros2 --restart unless stopped -it manus_ros2

