############################################################
## This file is generated automatically by Vivado HLS.
## Please DO NOT edit it.
## Copyright (C) 1986-2018 Xilinx, Inc. All Rights Reserved.
############################################################
open_project FSRCNN
set_top FSRCNN
add_files FSRCNN.cpp -cflags "-D__XFCV_HLS_MODE__ --std=c++11 -I./vitis_vision_library/xfopencv/include"
add_files -tb tb.cpp -cflags "-Ivitis_vision_library/xfopencv/include -Iopencv/include -D__XFCV_HLS_MODE__ --std=c++11 -Wno-unknown-pragmas"
add_files -tb input_image.jpeg -cflags "-Wno-unknown-pragmas"
open_solution "solution1"
set_part {xc7z020clg400-1} -tool vivado
create_clock -period 10 -name default
##source "./lic/solution1/directives.tcl"
csim_design -ldflags {-L <path to lib folder from opencv folder> -lopencv_core420 -lopencv_highgui420 -lopencv_imgproc420 -lopencv_imgcodecs420 -lopencv_flann420 -lopencv_features2d420} -argv {input_image.jpeg} -clean 
csynth_design
#cosim_design -ldflags {-L <path to lib folder from opencv folder> -lopencv_core420 -lopencv_highgui420 -lopencv_imgproc420 -lopencv_imgcodecs420 -lopencv_flann420 -lopencv_features2d420} -argv {input_image.jpeg}
export_design -format ip_catalog

