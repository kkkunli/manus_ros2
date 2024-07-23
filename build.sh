#!/bin/bash

docker build --build-arg CACHEBUST=$(date +%s) -t manus_ros2 .
