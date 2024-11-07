#!/bin/bash

docker build --build-arg CACHEBUST=$(date +%s) -t manus_ros2_for_yudie_tracker .
