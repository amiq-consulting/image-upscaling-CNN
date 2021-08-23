# Vivado HLS Source Code
## Flow
1. Download Vivado HLx 2019.1 - https://www.xilinx.com/support/download/index.html/content/xilinx/en/downloadNav/vivado-design-tools/archive.html  
2. OpenCV C/C++ installation (I used the latest OpenCV version until August 2021 - OpenCV 4.5.3)- https://medium.com/csmadeeasy/opencv-c-installation-on-windows-with-mingw-c0fc1499f39
	Steps:
	- Generate OpenCV binaries using Cmake - be aware that C++ compilation process takes a lot of time (in my case it lasted one hour).
	- In the environment variable add a path that points at the bin folder. In my case, I included *C:\opencv_453\install\x64\mingw\bin* in environment variable. Not including the path will cause Simulation/Co-Simulation errors.  
3. xfOpenCV 2019.1 version library installation - https://github.com/Xilinx/xfopencv
	- After xfOpenCV library installation, two changes need to be done in xf_axi.h file which is found on the following path (in my case): *C:\xfopencv-master\include\common\xf_axi.h*. Lines *144* and *259* (*IplImage img = cv_mat;*) must be replaced with *IplImage img = cvIplImage(cv_mat);*.    
4. Environment Setup:  
	To run the **Simulation** and the **Synthesis** using a TCL script, you need to:
	- download all the Vivado HLS source code files into a directory;
	- change all occurrences of the *C:/xfopencv-master/include* and the *C:/opencv_453/install/x64/mingw/lib* paths from *run_hls.tcl script* with your corresponding paths depending on where you downloaded the libraries and the names of the directories;
	- open Vivado HLS 2019.1 Command Prompt;
	- navigate to the directory where you saved all the project files;
	- run the following command in the Vivado Command Prompt: vivado_hls -f run_hls.tcl.  

	If you want to run the **Co-Simulation** you have to comment the following line *#pragma HLS INTERFACE ap_ctrl_none port=return* from the top function from the *FSRCNN.cpp* file (using the *ap_ctrl_none* mode might prevent the design from being verified using the C/RTL Co-Simulation feature) and uncomment the line which starts with *cosim_design* from the *run_hls.tcl script*.  
	If you want to run the Co-Simulation and the design path length is too long, the following errors will be displayed:
	
		ERROR: [COSIM 212-5] *** C/RTL co-simulation file generation failed. ***  
		ERROR: [COSIM 212-4] *** C/RTL co-simulation finished: FAIL ***  
		INFO: [COSIM 212-211] II is measurable only when transaction number is greater than 1 in RTL simulation. Otherwise, they will be marked as all NA. If user wants to calculate them, please make sure there are at least 2 transactions in RTL simulation.   
		
	If you encounter this problem, try reducing the project path length.

	You can see the **Synthesis/Co-Simulation report** if you open the project with Vivado HLS GUI (only after running the code using the provided TCL script). 
However, if you want to run the code using Vivado HLS GUI, you have to make the necessary configurations according to the *run_hls.tcl script*.  

## Versoning  
Using a similar flow with the one described above, the users can install the OpenCV version of their choice, given the fact that 2020.1+ Vivado HLx suites do not provide OpenCV and therefore are required multiple references to the OpenCV library for the HLS tools for C Simulation and Co-Simulation, and the Vision Library for IP compilation. 
Note that the OpenCV installation is only necessary for testbench verification, and is not required for kernel synthesis.  
The bicubic accelerator was not tested with the Vitis tool suite.
The accelerator testbench is also compatible with xfOpenCV 2018.x release, but it is not compatible with Vitis Vision Library.  

## Known issue  
Co-Simualtion takes a long time due to the OPMODE warning being displayed.
