# Bicubic-Interpolation

The bicubic interpolation method is widely adopted in the software world of image processing (Photoshop, Avid, Final Cut Pro and After Effects). 
For more details you can check the AMIQ Consulting blog post. 

*Non-Accelerated-Bicubic-Interpolation* directory contains the files needed to run the Python bicubic algorithm on the CPU. 
The CPU-run Python version of the algorithm has a sequential execution even if there is no numerical dependence that requires this. 

Using the advantages offered by FPGA and the parallelization potential of the algorithm, I accelerated the bicubic algorithm on FPGA using HLS, significantly reducing the execution time. 
*Accelerated-Bicubic-Interpolation* directory contains the *Vivado-HLS-Source-Code* directory and the *PYNQ-Z1-files* directory. 
*Vivado-HLS-Source-Code* directory contains the C/C++ source files which are needed to generate the RTL implementation of the accelerator. 
In *PYNQ-Z1-files* directory you can find all the necessary files to run the bicubic accelerator on the FPGA. 
*PYNQ-Z1-files* directory contains the bitstream and .hwh files that are ported on the board, but also the Python testbench with which you can test the functionality of the module.
