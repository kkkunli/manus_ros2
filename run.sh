#!/bin/bash

CONTAINER_NAME="manus_ros2_for_yudie_tracker"

# Function to clean up the container when the script exits
cleanup() {
    echo "Cleaning up..."
    docker stop $CONTAINER_NAME > /dev/null 2>&1
    docker rm $CONTAINER_NAME > /dev/null 2>&1
    exit 0
}

# Set the trap to catch exit signals and run the cleanup function
trap cleanup SIGINT SIGTERM

# Loop to keep the container running
while true; do
    echo "Starting the container..."
    docker run -d --rm --net=host --ipc=host --pid=host --dns 192.168.1.1 --name $CONTAINER_NAME manus_ros2_for_yudie_tracker

    echo "Attaching to the container..."
    docker attach $CONTAINER_NAME

    echo "Container exited. Restarting..."
done
