NODE = 2 # Todify here if you want to run in different number of nodes.
INPUT = sm.dat # This is your unsort file.
OUTPUT = sm-sol.txt # This is your sorted file after run the parallel sort.
runsort: sort
	mpirun -np $(NODE) ./sort $(INPUT) $(OUTPUT)
sort: main.c
	mpicc -o sort main.c -std=c99
