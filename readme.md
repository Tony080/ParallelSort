# Parallel Sorting with MPI<br>
Author: [Chengzhi Hu](https://github.com/Tony-Hu)
## 1. Introduction
This is a parallel sorting using MPI.<br>
You shall install [MPI](https://www.open-mpi.org/) before run this program.<br>
The architecture is described below:<br>

The program first read an unsorted file as input. The unsorted file contains 1 number per line.<br>
By default, *MAIN_PROCESS*(0 ~ max node #) defined as a macro at the top of the code will do the I/O, and other process(node) will just wait.<br>
Then *MAIN_PROCESS* will send the equivalent copy of input array to each node via *MPI_Scatterv* method.<br>
Each node will perform a *quick sort* in <stdlib> to sort.<br>
Then merges will be performed. That is, treat the node named with **even rank** as **master node**. And treat the node named with **odd rank** as **slave node**.<br>
Then merge these node as we perform a *bottom up merge sort*.<br>
As a final result, *MAIN_PROCESS* will hold the final sorted array and print them out in a file.<br><br>

## 2. How to use
You can just download the **main.c** and **makefile**. And in your terminal, enter the directory where your files downloaded. And just use command *make* to run the program.<br>
Notice that, you can modify the **makefile** such that you can run with different number of nodes, given different input & output files.<br>
*By default*, <br>
*NODE* = 2<br>
*INPUT* = sm.dat<br>
*OUTPUT* = sm-sol.txt<br><br>
Attention: you can't give a *NODE* number that is greater than the actual physical cores you have.

