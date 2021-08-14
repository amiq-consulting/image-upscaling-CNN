# FSRCNN
CPU FSRCNN directory contains all the files needed to run the FSRCNN network on the CPU using a Jupyter Notebook. The CPU-run Python version of the network involves a sequential execution of the network layers even if there is no numeric dependency that requires this. 

Using the advantages offered by FPGA and the parallelization potential of the FSRCNN, I accelerated the CNN on FPGA using HLS, significantly reducing the execution time. FPGA FSRCNN directory contains two versions of the network: the accelerated FSRCNN and the non-accelerated FSRCNN. Accelerated FSRCNN directory contains the files necessary to generate the RTL implementation of the accelerator, the bitstream and hwh files that are ported on the board, but also the Python testbench with which you can test the functionality of the FPGA accelerator.  

# Tools used
**Simulate, Synthesize, Cosimulate accelerator:** Vivado HLS 2018.2  

**Block diagram:** Vivado 2018.2  

**The initial network is tested with the following hardware platform:** x64 CPU - Intel Core i5-6200U with Windows 10 Operating System  

**The accelerator is tested with the following hardware platform:** Xilinx - Zynq-7000 SoC
