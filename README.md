# Accelerate Image Upscaling Algorithms on FPGA Using HLS

In this repository you can find the image upscaling methods I implemented and accelerated - the forward propagation of the Fast Super-Resolution Convolutional Neural Network (FSRCNN) - https://arxiv.org/pdf/1608.00367.pdf) and the bicubic interpolation algorithm  used as a reference for the machine learning method. 
For more details you can check the AMIQ Consulting blog post. 

Using the advantages offered by the FPGA and the parallelization potential of the algorithms spcecified above, I accelerated the FSRCNN and the bicubic interpolation algorithm on FPGA using HLS, significantly reducing the execution time. 

# Tools used
**Operating System:** Windows 10  

**Simulate, Synthesize, Cosimulate accelerator:** Vivado HLS 2019.1 

 	Libraries used  

	OpenCV version:   4.5.3  - https://github.com/opencv/opencv  

	xfOpenCV version: 2019.1 - https://github.com/Xilinx/xfopencv  


**Block diagram:** Vivado 2019.1  

**The initial algorithms are tested with the following hardware platform:** x64 CPU - Intel Core i5-6200U 

**The accelerators are tested with the following hardware platform:** Xilinx - Zynq-7000 SoC  

# Blog post
*How to Accelerate an Image Upscaling CNN on FPGA using HLS* - https://www.amiq.com/consulting/blog/.  

# License
The application is available for free under the Apache License 2.
