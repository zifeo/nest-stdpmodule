#!/usr/bin/env bash

cd ~
git clone https://github.com/zifeo/NEST-STDPModule.git stdpmodule
cd stdpmodule
./bootstrap.sh
./configure --with-nest=/opt/nest/bin/nest-config
make -j 16
make install
