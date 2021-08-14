#pragma once
#include <ap_fixed.h>             /* Bring the ap_fixed type */
#include <ap_int.h>               /* Brings the ap_uint type */
#include <hls_stream.h>           /* Brings the stream */
#include "common/xf_common.h"     /* Brings the macros for channel and data width*/
#include "common/xf_infra.h"
//#include "common/xf_utility.h"
#define NPC1 XF_NPPC1             /* Define 1 pixel version */
#define NR_OF_IMAGE_CHANNELS 3
#define PADDING_NUMBER 2
#define NR_OF_NEEDED_ROWS_PER_PIXEL 4
#define SHIFT_LEFT_WITH_8_BITS 256
#define SHIFT_LEFT_WITH_16_BITS 65536
//#define WIDTH 24
//#define INT_WIDTH 12             /* Justified by the maximum dimensions of the input image*/
//typedef ap_fixed<WIDTH, INT_WIDTH> float24_t;
//#define WIDTH2 1
//#define INT_WIDTH2 0             /* Justified by the maximum dimensions of the input image*/
//typedef ap_fixed<WIDTH2, INT_WIDTH2> float1_t; //for a = -0.1f*interpolation_parameter
const int WIDTH_MAX = 1024;
const int HEIGHT_MAX = 1024;
const int PPC = 1;                               /* pixels per clock*/
const int PIXEL_WIDTH = PPC*8*4;                 /* one pixel has 32 bits: 8x3 bits for RGB colors and 8 bits are reserved*/
typedef ap_axiu<PIXEL_WIDTH, 1, 1, 1> package_t;
typedef hls::stream<package_t> stream_t;         /* Define the AXI Stream type */
using namespace xf;

//-------------------------------------------------------------------------------------------------------------------------------------
float module(float s);
float interpolation_kernel(float s, float a);
/* Declare the top function - This function needs redundant inputs */
void bicubic(stream_t& stream_in, stream_t& stream_out, ap_uint<12> height, ap_uint<12> width, ap_uint<4> interpolation_parameter, ap_uint<8> scale_factor);
