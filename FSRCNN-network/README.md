# Fast Super-Resolution Convolutional Neural Network - FSRCNN 

*Non-Accelerated-FSRCNN* directory contains the *Python-FSRCNN* directory and the *Vivado-HLS-Source-Code* directory. 
In the *Python-FSRCNN* directory you can find the files needed to run the Python FSRCNN on the CPU. 
The CPU-run Python version of the network has a sequential execution even if there is no numerical dependence that requires this. 
I ported the sequential version of the network to Vivado HLS. 
In the *Vivado-HLS-Source-Code* directory you can find the C/C++ source code which generates a synthesizable module. 

Using the advantages offered by the FPGA and the parallelization potential of the FSRCNN, I accelerated the CNN on FPGA using HLS, significantly reducing the execution time. 
*Accelerated-FSRCNN* directory contains the *Vivado-HLS-Source-Code* directory, the *Vivado-Block-Diagram* directory and the *PYNQ-Z1-files* directory. 
*Vivado-HLS-Source-Code* directory contains the C/C++ source files which are needed to generate the RTL implementation of the accelerator. 
The *Vivado-Block-Diagram* directory contains the block diagram figure which shows the connections between the FSRCNN module (custom IP) and the DMA block. 
Designing this block diagram in Vivado allows you to generate the FSRCNN bitstream and .hwh files that are ported to the FPGA.
In *PYNQ-Z1-files* directory you can find all the necessary files to run the FSRCNN accelerator on the FPGA. 
*PYNQ-Z1-files* directory contains the bitstream and .hwh files that are ported on the board, but also the Python testbench with which you can test the functionality of the module.
