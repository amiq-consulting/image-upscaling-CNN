///OpenCV - Xilinx custom version
//#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
//#include "opencv2/imgproc/imgproc.hpp"
///Software tools for testbench - They must be in this order
#include "common/xf_sw_utils.h"
#include "common/xf_axi.h"
///Import wrapper
#include "wrapper.hpp"
#include <chrono>
using namespace std::chrono;

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "Invalid Number of Arguments!\nUsage:\n");
    fprintf(stderr, "%s <input image path> \n", argv[0]);
    return -1;
  }
  cv::Mat in_img, out_hw;
  //Load input image
  printf("Reading image...\n");
  in_img = cv::imread(argv[1], 1);
  if (in_img.data == NULL) {
    fprintf(stderr, "Can not open image at %s\n", argv[1]);
    return -1;
  }
  //Initialize function parameters
  ap_uint<12> height = in_img.rows;
  ap_uint<12> width = in_img.cols;
  ap_uint<8> scale_factor = 2;
  //Create the hardware streams
  out_hw.create(scale_factor*height, scale_factor*width, CV_8UC3);
  //Generate the streams from the input image
  stream_t src_hw, sink_hw;
  cvMat2AXIvideoxf<NPC1>(in_img, src_hw);
  //Execute accelerator
  auto start = high_resolution_clock::now();
  FSRCNN(src_hw, sink_hw, height, width, scale_factor);
  auto stop = high_resolution_clock::now();
  auto duration = duration_cast<seconds>(stop - start);
  cout << "Simulation time [seconds] on CPU for one channel is: "<<duration.count() << endl;
  //Retrieve the output and put it into a cv::Mat
  AXIvideo2cvMatxf<NPC1>(sink_hw, out_hw);
  cv::imshow("output_image_window_title", out_hw);
  //Wait for a keystroke in the window
  int k = cv::waitKey(0);
  cv::imwrite("output_image.jpeg", out_hw);

  return 0;
}
