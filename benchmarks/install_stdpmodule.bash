#!/usr/bin/env bash

export DEBIAN_FRONTEND=noninteractive

cd ~
git clone https://github.com/zifeo/NEST-STDPModule.git stdpmodule
cd stdpmodule
./bootstrap.sh
./configure --with-nest=/opt/nest/bin/nest-config
make -j 8
make install
