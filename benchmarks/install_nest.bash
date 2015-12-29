#!/usr/bin/env bash

cd ~
git clone https://github.com/nest/nest-simulator.git nest
cd nest
./bootstrap.sh
./configure --with-mpi --prefix=/opt/nest
make -j 16
make install
echo 'export PATH=$PATH:/opt/nest/bin' >> ~/.profile
echo 'export PYTHONPATH=/opt/nest/lib/python2.7/site-packages:$PYTHONPATH' >> ~/.profile
echo 'export PYTHONPATH=/opt/nest/lib64/python2.7/site-packages:$PYTHONPATH' >> ~/.profile
source ~/.profile
