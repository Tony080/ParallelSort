cd $HOME/local/OpenMPI3.1.0/bin
mkdir -p openmpi && cd openmpi
wget --no-check-certificate https://download.open-mpi.org/release/open-mpi/v3.1/openmpi-3.1.0.tar.bz2
tar -xjf openmpi-3.1.0.tar.bz2
cd openmpi-3.1.0
./configure --prefix=$HOME/local/OpenMPI3.1.0 && make -j 4 && make install && make clean
PATH = $HOME/local/OpenMPI3.1.0/bin;$PATH
cd ../../
