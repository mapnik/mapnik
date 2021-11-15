FROM ubuntu:21.04
LABEL description="mapnik" 

ARG DEBIAN_FRONTEND=noninteractive

RUN mkdir /app
COPY . /app
WORKDIR /app

RUN apt update
RUN apt install -y gpg wget
RUN wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null 
RUN echo 'deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ focal main' | tee /etc/apt/sources.list.d/kitware.list >/dev/null
RUN apt update
RUN apt install -y \
    cmake \
    ninja-build \
    build-essential

ENV BUILD_DEPENDENCIES="libicu-dev \
    libfreetype6-dev \
    libharfbuzz-dev \
    libxml2-dev \
    libjpeg-dev \
    libtiff-dev \
    libwebp-dev \
    libcairo2-dev \
    libproj-dev \
    libgdal-dev \
    libboost-filesystem-dev \
    libboost-program-options-dev \
    libboost-regex-dev \
    "

ENV RUNTIME_DEPENDENCIES="libfreetype6 \
    libharfbuzz-bin \
    libxml2 \
    libjpeg8 \
    libtiff5 \
    libwebp6 \
    libcairo2 \
    libproj19 \
    libgdal28 \
    libboost-filesystem1.74-dev \
    libboost-program-options1.74.0 \
    libboost-regex1.74.0 \
    "

RUN apt install -y $BUILD_DEPENDENCIES $RUNTIME_DEPENDENCIES
RUN cmake --preset linux-gcc-release -DBUILD_DEMO_VIEWER=OFF -DBUILD_TESTING=OFF -DBUILD_DEMO_CPP=OFF -DBUILD_BENCHMARK=OFF
RUN cmake --build --preset linux-gcc-release
RUN cmake --build --preset linux-gcc-release --target install

RUN apt autoremove -y --purge $BUILD_DEPENDENCIES