# Copyright 2018 gRPC authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.#
#
# Based on https://hub.docker.com/r/grpc/cxx.

FROM ubuntu:focal as build

ENV TZ=Asia/Shanghai
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN apt-get update && apt-get install -y \
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
  && apt-get clean

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

# copy current directory into the image and run `make`
COPY . /SDKMinimalClient_Linux
WORKDIR /SDKMinimalClient_Linux
RUN make

ENTRYPOINT /SDKMinimalClient_Linux/SDKMinimalClient.out
