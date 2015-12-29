#!/usr/bin/env bash

export DEBIAN_FRONTEND=noninteractive

apt update
apt upgrade -y
until apt install -y git-all git vim openmpi-bin openmpi-common build-essential autoconf automake libtool libltdl7-dev \
    libreadline6-dev libncurses5-dev libgsl0-dev python-all-dev python-numpy python-scipy python-matplotlib \
    cython ipython
do
    echo "Retry apt"
    sleep 1
done
apt update
apt install -y libopenmpi-dev
apt install --fix-missing
