open_project FSRCNN
set_top FSRCNN
add_files FSRCNN.cpp -cflags "-D__XFCV_HLS_MODE__ --std=c++11 -IC:/xfopencv-master/include"
add_files -tb tb.cpp -cflags "-IC:/xfopencv-master/include -IC:/opencv_453/install/include -D__XFCV_HLS_MODE__ --std=c++11 -Wno-unknown-pragmas"
add_files -tb input_image.jpeg -cflags "-Wno-unknown-pragmas"
open_solution "solution1"
set_part {xc7z020clg400-1} -tool vivado
create_clock -period 10 -name default
csim_design -ldflags {-L C:/opencv_453/install/x64/mingw/lib -lopencv_core453 -lopencv_highgui453 -lopencv_imgproc453 -lopencv_imgcodecs453 -lopencv_flann453 -lopencv_features2d453} -argv {input_image.jpeg} -clean 
csynth_design
#cosim_design -ldflags {-L C:/opencv_453/install/x64/mingw/lib -lopencv_core453 -lopencv_highgui453 -lopencv_imgproc453 -lopencv_imgcodecs453 -lopencv_flann453 -lopencv_features2d453} -argv {input_image.jpeg}
export_design -format ip_catalog
