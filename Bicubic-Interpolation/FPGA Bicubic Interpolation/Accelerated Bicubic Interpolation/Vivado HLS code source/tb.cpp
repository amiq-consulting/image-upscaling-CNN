/* OpenCV - Xilinx custom version */
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
/* Software tools for testbench - They must be in this order */
#include "common/xf_sw_utils.h"
#include "common/xf_axi.h"
/* Import the accelerator */
#include "wrapper.hpp"
using namespace std;
#include <chrono>
using namespace std::chrono;

/* The padding function */
void padding(cv::Mat img, cv::Mat zimg){
    ap_uint<12> H = img.rows;
    ap_uint<12> W = img.cols;
    ap_uint<2> C = img.channels();
    for (int c = 0; c < C; c++) {
        for (int j = PADDING_NUMBER; j < W+PADDING_NUMBER; j++) {
            for (int i = PADDING_NUMBER; i < H+PADDING_NUMBER; i++) {
                zimg.data[C*((W+2*PADDING_NUMBER)*i+j)+c] = img.data[C*(W*(i-PADDING_NUMBER)+j-PADDING_NUMBER)+c];
            }
        }
    }
    //Pad the first/last two col and row
    for (int c = 0; c < C; c++) {
        for (int j = 0; j < PADDING_NUMBER; j++) {
            for (int i = PADDING_NUMBER; i < H + PADDING_NUMBER; i++) {
                zimg.data[C*((W+2*PADDING_NUMBER)*i+j)+c] = img.data[C*(W*(i-PADDING_NUMBER))+c];        // the leftmost two col
            }
        }
        for (int j = PADDING_NUMBER; j < W + PADDING_NUMBER; j++) {
            for (int i = H + PADDING_NUMBER; i < H + 2*PADDING_NUMBER; i++) {
                zimg.data[C*((W+2*PADDING_NUMBER)*i+j)+c] = img.data[C*(W*(H-1)+j-PADDING_NUMBER)+c]; // the top two rows
            }
        }
        for (int j = W + PADDING_NUMBER; j < W + 2*PADDING_NUMBER; j++) {
            for (int i = PADDING_NUMBER; i < H + PADDING_NUMBER; i++) {
                zimg.data[C*((W+2*PADDING_NUMBER)*i+j)+c] = img.data[C*(W*(i-PADDING_NUMBER)+W-1)+c];;// the rightmost two col
            }
        }
        for (int j = PADDING_NUMBER; j < W + PADDING_NUMBER; j++) {
            for (int i = 0; i < PADDING_NUMBER; i++) {
                zimg.data[C*((W+2*PADDING_NUMBER)*i+j)+c] = img.data[C*(j-PADDING_NUMBER)+c];;        // the bottom two rows
            }
        }
        //Pad the missing eight points
        for (int j = 0; j < PADDING_NUMBER; j++) {
            for (int i = 0; i < PADDING_NUMBER; i++) {
                zimg.data[C*((W+2*PADDING_NUMBER)*i+j)+c] = img.data[c];                             // down left corner (4 points)

            }
        }
        for (int j = 0; j < PADDING_NUMBER; j++) {
            for (int i = H + PADDING_NUMBER; i < H + 2*PADDING_NUMBER; i++) {
                zimg.data[C*((W+2*PADDING_NUMBER)*i+j)+c] = img.data[C*(W*(H-1))+c];                 // up left corner   (4 points)
            }
        }
        for (int j = W + PADDING_NUMBER; j < W + 2*PADDING_NUMBER; j++) {
            for (int i = H + PADDING_NUMBER; i < H + 2*PADDING_NUMBER; i++) {
                zimg.data[C*((W+2*PADDING_NUMBER)*i+j)+c] = img.data[C*(W*(H-1)+W-1)+c];             // up right corner  (4 points)
            }
        }
        for (int j = W + PADDING_NUMBER; j < W + 2*PADDING_NUMBER; j++) {
            for (int i = 0; i < PADDING_NUMBER; i++) {
                zimg.data[C*((W+2*PADDING_NUMBER)*i+j)+c] = img.data[C*(W-1)+c];                     // down right corner(4 points)
            }
        }
    }
}

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "Invalid Number of Arguments!\nUsage:\n");
    fprintf(stderr, "%s <input image path> \n", argv[0]);
    return -1;
  }
  cv::Mat in_img, out_hw;
  cv::Mat padding_img;
  /* Load input image */
  printf("Reading image...\n");
  in_img = cv::imread(argv[1], 1);
  if (in_img.data == NULL) {
    fprintf(stderr, "Can not open image at %s\n", argv[1]);
    return -1;
  }
  ap_uint<12> height = in_img.rows + PADDING_NUMBER*2;
  ap_uint<12> width = in_img.cols + PADDING_NUMBER*2;
  ap_uint<4> interpolation_parameter = 5;
  ap_uint<8> scale_factor = 2;
  /* Padding the image */
  padding_img.create(height, width, CV_8UC3);
  padding(in_img, padding_img);
  /* Create the hardware streams */
  printf("Hardware processing...\n");
  out_hw.create(scale_factor*in_img.rows, scale_factor*in_img.cols, CV_8UC3);
  /* Generate the streams from the input image */
  stream_t src_hw, sink_hw;
  cvMat2AXIvideoxf<NPC1>(in_img, src_hw);
  /* Execute accelerator */
  auto start = high_resolution_clock::now();
  bicubic(src_hw, sink_hw, in_img.rows, in_img.cols, interpolation_parameter, scale_factor);
  auto stop = high_resolution_clock::now();
  auto duration = duration_cast<microseconds>(stop - start);
  cout << "Simulation time [microseconds] is: "<<duration.count() << endl;
  /* Retrieve the output and put it into a cv::Mat */
  AXIvideo2cvMatxf<NPC1>(sink_hw, out_hw);
  cv::imshow("output_image_window_title", out_hw);
  int k = cv::waitKey(0); // Wait for a keystroke in the window
  cv::imwrite("output_image.jpeg", out_hw);
  printf("Hardware done...\n");
  return 0;
}
