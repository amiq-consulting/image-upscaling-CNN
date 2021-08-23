//Project headers
#include "wrapper.hpp"

//Top function
void bicubic(stream_t& stream_in, stream_t& stream_out, ap_uint<12> height, ap_uint<12> width, ap_uint<4> interpolation_parameter, ap_uint<8> scale_factor)
{
    //Implements all ports as an AXI4-Lite interface.
    //The HLS tool produces an associated set of C driver files
    //during the Export RTL process. By default, the HLS tool
    //groups all function arguments specified as an AXI4-Lite
    //(s_axilite) interface into a single AXI4-Lite port
#pragma HLS INTERFACE s_axilite register port=height bundle=scalar_parameters
#pragma HLS INTERFACE s_axilite register port=width bundle=scalar_parameters
#pragma HLS INTERFACE s_axilite register port=interpolation_parameter bundle=scalar_parameters
#pragma HLS INTERFACE s_axilite register port=scale_factor bundle=scalar_parameters
    //No block-level I/O protocol. Using the ap_ctrl_none mode might
    //prevent the design from being verified using the C/RTL co-simulation
    //feature.
#pragma HLS INTERFACE ap_ctrl_none port=return
    // Implements all ports as an AXI4-Stream interface.
#pragma HLS INTERFACE axis register both port = stream_in
#pragma HLS INTERFACE axis register both port = stream_out

    static package_t buffer[NR_OF_NEEDED_ROWS_PER_PIXEL][WIDTH_MAX];
    //#pragma HLS ARRAY_PARTITION variable=buffer complete dim=1  //the latency
    //is increased (x10), but BRAM is reduced (x3).
   // ap_uint<12> dH = scale_factor*(height-PADDING_NUMBER*2);
   // ap_uint<12> dW = scale_factor*(width-PADDING_NUMBER*2);
   // ap_uint<2>   C = NR_OF_IMAGE_CHANNELS;
    float a = -interpolation_parameter*0.1f;
    float h = 1.0f/scale_factor;
    float x, y, x1, x2, x3, x4, y1, y2, y3, y4;
    int previous = -1;
    for (int i = 0; i < scale_factor*height; i++)
    {
#pragma HLS LOOP_TRIPCOUNT min=512 max=512
        if (previous != int(i*h))
        {
            for(int i=0; i<NR_OF_NEEDED_ROWS_PER_PIXEL-1; i++)
            {
#pragma HLS LOOP_TRIPCOUNT min=3 max=3
                for(int j=0; j<width; j++)
                {
#pragma HLS LOOP_TRIPCOUNT min=256 max=256
//#pragma HLS UNROLL factor = 8
                    //shift the rows in the buffer from down to up to make room for the new row
                    buffer[i][j] = buffer[i + 1][j];
                }
            }
            for(int k=0; k<width; k++)
            {
#pragma HLS LOOP_TRIPCOUNT min=256 max=256
#pragma HLS PIPELINE
                //Read some data (one row) from the stream; if it's not available then
                 //wait until it becomes available.
                stream_in >> buffer[3][k];
            }
        }
        previous = int(i*h);
        y = i * h + 2;
        y1 = 1 + y - floor(y);
        y2 = y - floor(y);
        y3 = floor(y) + 1 - y;
        y4 = floor(y) + 2 - y;
        //prepare coefficients for interpolation
        float mat_r[] = { interpolation_kernel(y1,a), interpolation_kernel(y2,a), interpolation_kernel(y3,a), interpolation_kernel(y4,a) };
        for (int j = 0; j < scale_factor*width; j++)
        {
#pragma HLS LOOP_TRIPCOUNT min=512 max=512
//#pragma HLS UNROLL factor = 4
            x = j * h + 2;
            x1 = 1 + x - floor(x);
            x2 = x - floor(x);
            x3 = floor(x) + 1 - x;
            x4 = floor(x) + 2 - x;
            //prepare coefficients for interpolation
            float mat_l[] = { interpolation_kernel(x1,a), interpolation_kernel(x2,a), interpolation_kernel(x3,a), interpolation_kernel(x4,a) };
            package_t pix_out;
            //Normally with an AXI Stream, there's a side-channel in addition to the main data.
            //In that channel there's a bit called TLAST, which is used to indicate the final element
            //of the data. Normally your block would detect TLAST and stop processing at that point.
            //Generally the input comes from an AXI DMA block (which already knows about TLAST and
            //will set it correctly) and the output goes to another DMA block (which will understand
            //the TLAST coming from this block). Both take that data to/from RAM via Zynq PS.
            //As soon as the output DMA moves some more data to RAM, space will open up in the stream
            //and the block will continue.
            //pix_out has TLAST and TUSER signals
            pix_out = buffer[3][j%width];
            //for every channel, for every output pixel, the interpolation is performed */
            float rez00 = (mat_l[0] * (float)buffer[0][int(x - x1)].data.range(7,0) + mat_l[1] * (float)buffer[0][int(x - x2)].data.range(7,0) + mat_l[2] * (float)buffer[0][int(x + x3)].data.range(7,0) + mat_l[3] * (float)buffer[0][int(x + x4)].data.range(7,0)) * mat_r[0] +
                   (mat_l[0] * (float)buffer[1][int(x - x1)].data.range(7,0) + mat_l[1] * (float)buffer[1][int(x - x2)].data.range(7,0) + mat_l[2] * (float)buffer[1][int(x + x3)].data.range(7,0) + mat_l[3] * (float)buffer[1][int(x + x4)].data.range(7,0)) * mat_r[1] +
                   (mat_l[0] * (float)buffer[2][int(x - x1)].data.range(7,0) + mat_l[1] * (float)buffer[2][int(x - x2)].data.range(7,0) + mat_l[2] * (float)buffer[2][int(x + x3)].data.range(7,0) + mat_l[3] * (float)buffer[2][int(x + x4)].data.range(7,0)) * mat_r[2] +
                   (mat_l[0] * (float)buffer[3][int(x - x1)].data.range(7,0) + mat_l[1] * (float)buffer[3][int(x - x2)].data.range(7,0) + mat_l[2] * (float)buffer[3][int(x + x3)].data.range(7,0) + mat_l[3] * (float)buffer[3][int(x + x4)].data.range(7,0)) * mat_r[3];

            float rez11 = (mat_l[0] * (float)buffer[0][int(x - x1)].data.range(15,8) + mat_l[1] * (float)buffer[0][int(x - x2)].data.range(15,8) + mat_l[2] * (float)buffer[0][int(x + x3)].data.range(15,8) + mat_l[3] * (float)buffer[0][int(x + x4)].data.range(15,8)) * mat_r[0] +
                   (mat_l[0] * (float)buffer[1][int(x - x1)].data.range(15,8) + mat_l[1] * (float)buffer[1][int(x - x2)].data.range(15,8) + mat_l[2] * (float)buffer[1][int(x + x3)].data.range(15,8) + mat_l[3] * (float)buffer[1][int(x + x4)].data.range(15,8)) * mat_r[1] +
                   (mat_l[0] * (float)buffer[2][int(x - x1)].data.range(15,8) + mat_l[1] * (float)buffer[2][int(x - x2)].data.range(15,8) + mat_l[2] * (float)buffer[2][int(x + x3)].data.range(15,8) + mat_l[3] * (float)buffer[2][int(x + x4)].data.range(15,8)) * mat_r[2] +
                   (mat_l[0] * (float)buffer[3][int(x - x1)].data.range(15,8) + mat_l[1] * (float)buffer[3][int(x - x2)].data.range(15,8) + mat_l[2] * (float)buffer[3][int(x + x3)].data.range(15,8) + mat_l[3] * (float)buffer[3][int(x + x4)].data.range(15,8)) * mat_r[3];

            float rez22 = (mat_l[0] * (float)buffer[0][int(x - x1)].data.range(23,16) + mat_l[1] * (float)buffer[0][int(x - x2)].data.range(23,16) + mat_l[2] * (float)buffer[0][int(x + x3)].data.range(23,16) + mat_l[3] * (float)buffer[0][int(x + x4)].data.range(23,16)) * mat_r[0] +
                   (mat_l[0] * (float)buffer[1][int(x - x1)].data.range(23,16) + mat_l[1] * (float)buffer[1][int(x - x2)].data.range(23,16) + mat_l[2] * (float)buffer[1][int(x + x3)].data.range(23,16) + mat_l[3] * (float)buffer[1][int(x + x4)].data.range(23,16)) * mat_r[1] +
                   (mat_l[0] * (float)buffer[2][int(x - x1)].data.range(23,16) + mat_l[1] * (float)buffer[2][int(x - x2)].data.range(23,16) + mat_l[2] * (float)buffer[2][int(x + x3)].data.range(23,16) + mat_l[3] * (float)buffer[2][int(x + x4)].data.range(23,16)) * mat_r[2] +
                   (mat_l[0] * (float)buffer[3][int(x - x1)].data.range(23,16) + mat_l[1] * (float)buffer[3][int(x - x2)].data.range(23,16) + mat_l[2] * (float)buffer[3][int(x + x3)].data.range(23,16) + mat_l[3] * (float)buffer[3][int(x + x4)].data.range(23,16)) * mat_r[3];
            if(rez00 >= 255)
                rez00 = 255;
            if(rez11 >= 255)
                rez11 = 255;
            if(rez22 >= 255)
                rez22 = 255;
            if(rez00 < 0)
                rez00 = 0;
            if(rez11 < 0)
                rez11 = 0;
            if(rez22 < 0)
                rez22 = 0;
            //Create the value for the (i,j) pixel from the upscaled image.
            pix_out.data = (int)rez00 + SHIFT_LEFT_WITH_8_BITS *(int)rez11 + SHIFT_LEFT_WITH_16_BITS * (int)rez22;
            //Write some data to the stream; if it's full then wait until there's space.
            stream_out << pix_out;
        }
    }
}
