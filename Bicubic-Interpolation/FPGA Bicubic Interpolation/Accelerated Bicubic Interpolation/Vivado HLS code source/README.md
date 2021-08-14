  # Vivado HLS code source
  To run the simulation and the synthesis, you need to:
- download Vivado HLx 2018.2 from "https://www.xilinx.com/support/download/index.html/content/xilinx/en/downloadNav/vivado-design-tools/archive.html" (it is not guaranteed to work with another version);
- download all the files into a directory;
- change <path to the lib directory from opencv directory> from run_hls.tcl script;
- open Vivado HLS 2018.2 Command Prompt;
- go to the directory where you saved all the project files;
- run the following command in the Vivado Command Prompt: vivado_hls -f run_hls.tcl.

If you want to run the co-simulation you have to comment the following line '#pragma HLS INTERFACE ap_ctrl_none port=return' (using the ap_ctrl_none mode might prevent the design from being verified using the C/RTL co-simulation feature) and uncomment the line which starts with 'cosim_design' from run_hls.tcl script.

You can see the synthesis report/co-simulation report if you open the project with Vivado HLS GUI (only after running the code using the provided tcl script). However, if you want to run the code using Vivado HLS GUI, you have to make the necessary configurations according to run_hls.tcl script.

Known issue:   
 Co-simualtion takes a long time due to the OPMODE warning being displayed.
