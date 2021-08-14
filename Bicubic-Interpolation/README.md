# Bicubic-Interpolation
CPU Bicubic Interpolation directory contains all the files needed to run a Jupyter Notebook project with the bicubic algorithm on the CPU. The CPU-run Python version of the algorithm has a sequential execution even if there is no numeric dependency that requires this. 

Using the advantages offered by FPGA and the parallelization potential of the algorithm, I accelerated the bicubic algorithm on FPGA using HLS, significantly reducing the execution time. FPGA Bicubic Interpolation directory contains the files necessary to generate the RTL implementation of the accelerator, the bitstream and hwh files that are ported on the board, but also the Python testbench with which you can test the functionality of the module.

# Tools used
**Simulate, Synthesize, Cosimulate accelerator:** Vivado HLS 2018.2  

**Block diagram:** Vivado 2018.2  

**The initial algorithm is tested with the following hardware platform:** x64 CPU - Intel Core i5-6200U with Windows 10 Operating System  

**The accelerator is tested with the following hardware platform:** Xilinx - Zynq-7000 SoC  

