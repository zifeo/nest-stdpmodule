#!/usr/bin/env bash

export DEBIAN_FRONTEND=noninteractive

apt update
apt upgrade -y
apt install -y git-all git vim openmpi-bin openmpi-common build-essential autoconf automake libtool libltdl7-dev \
    libreadline6-dev libncurses5-dev libgsl0-dev python-all-dev python-numpy python-scipy python-matplotlib \
    cython ipython
apt update
apt install -y libopenmpi-dev
apt install --fix-missing

cd ~
git clone https://github.com/nest/nest-simulator.git nest
cd nest
./bootstrap.sh
./configure --with-mpi --prefix=/opt/nest
make -j 8
make install
echo 'export PATH=$PATH:/opt/nest/bin' >> /etc/bash.bashrc
echo 'export PYTHONPATH=/opt/nest/lib/python2.7/site-packages:$PYTHONPATH' >> /etc/bash.bashrc
echo 'export PYTHONPATH=/opt/nest/lib64/python2.7/site-packages:$PYTHONPATH' >> /etc/bash.bashrc
source /etc/bash.bashrc
