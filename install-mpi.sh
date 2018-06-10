if [ ! -d "$HOME/local/$MPI_IMPL/bin" ]; then
  wget --no-check-certificate https://download.open-mpi.org/release/open-mpi/v3.1/openmpi-3.1.0.tar.bz2
  tar -xjf openmpi-3.1.0.tar.bz2
  cd openmpi-3.1.0
  ./configure --prefix=$HOME/local/OpenMPI3.1.0 && make -j 4 && make install && make clean
  cd ../../
  else
  echo "Found existing MPI 3.1.0"
fi
PATH = $HOME/local/OpenMPI3.1.0/bin;$PATH
  
