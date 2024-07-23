# Use the official ROS2 Foxy base image
FROM osrf/ros:foxy-desktop

# Update the package list and install necessary tools
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    python3-colcon-common-extensions \
    python3-rosdep \
    python3-vcstool \
    # Required to install GRPC
    build-essential \
    git \
    libtool \
    libzmq3-dev \
    # Only required for building the SDK
    zlib1g-dev \
    libspdlog-dev \
    # Only required for building the minimal client
    libncurses5-dev \
    # Only required for visual studio debugging
    gdb \
    && rm -rf /var/lib/apt/lists/*

ENV GRPC_RELEASE_TAG="v1.28.1"

RUN git clone -b ${GRPC_RELEASE_TAG} https://github.com/grpc/grpc /var/local/git/grpc && \
    cd /var/local/git/grpc && \
    git submodule update --init --recursive

RUN echo "-- installing protobuf" && \
    cd /var/local/git/grpc/third_party/protobuf && \
    ./autogen.sh && ./configure --enable-shared && \
    make -j$(nproc) && make -j$(nproc) check && make install && make clean && ldconfig

RUN echo "-- installing grpc" && \
    cd /var/local/git/grpc && \
    make -j$(nproc) && make install && make clean && ldconfig

# Initialize rosdep
RUN rosdep update
RUN apt-get update && apt-get install libfmt-dev && rm -rf /var/lib/apt/lists/*

# Create a workspace
RUN mkdir -p /root/ros2_ws/src
WORKDIR /root/ros2_ws

# Add a build argument to force rebuilds
ARG CACHEBUST=1

# Copy the package source code to the workspace
COPY . /root/ros2_ws/src/manus_ros2

# Build the package
RUN . /opt/ros/foxy/setup.sh && colcon build

# Source the workspace setup
RUN echo "source /root/ros2_ws/install/setup.bash" >> /root/.bashrc

# Set the entrypoint
ENTRYPOINT [ "/root/ros2_ws/src/manus_ros2/entrypoint.sh" ]
