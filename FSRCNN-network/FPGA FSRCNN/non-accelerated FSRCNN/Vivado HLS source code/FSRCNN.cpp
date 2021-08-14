//Project headers
#include "wrapper.hpp"
///----------------------------------------------------------------------------------------------
/// PADDING
///----------------------------------------------------------------------------------------------
void padding(double input_2D_image[MAX_IN_HEIGHT][MAX_IN_WIDTH], int prev_height, int prev_width, int pad, double padded_2D_image[MAX_PAD_HEIGHT][MAX_PAD_WIDTH])
{
    if (pad > 0)
    {
        int W = prev_width;
        int H = prev_height;
        for (int j = pad; j < W + pad; j++)
        {
            for (int i = pad; i < H + pad; i++)
            {
                padded_2D_image[i][j] = input_2D_image[i - pad][j - pad];
            }
        }
        //Pad the first/last two col and row
        for (int j = 0; j < pad; j++)
        {
            for (int i = pad; i < H + pad; i++)
            {
                padded_2D_image[i][j] = input_2D_image[i - pad][0];  // the leftmost two col
            }
        }
        for (int j = pad; j < W + pad; j++)
        {
            for (int i = H + pad; i < H + 2 * pad; i++)
            {
                padded_2D_image[i][j] = input_2D_image[H - 1][j - pad];// the bottom two rows
            }
        }
        for (int j = W + pad; j < W + 2 * pad; j++)
        {
            for (int i = pad; i < H + pad; i++)
            {
                padded_2D_image[i][j] = input_2D_image[i - pad][W - 1];// the rightmost two col
            }
        }
        for (int j = pad; j < W + pad; j++)
        {
            for (int i = 0; i < pad; i++)
            {
                padded_2D_image[i][j] = input_2D_image[0][j - pad]; // the top two rows
            }
        }
        //Pad the missing eight points
        for (int j = 0; j < pad; j++)
        {
            for (int i = 0; i < pad; i++)
            {
                padded_2D_image[i][j] = input_2D_image[0][0];    // up left corner
            }
        }
        for (int j = 0; j < pad; j++)
        {
            for (int i = H + pad; i < H + 2 * pad; i++)
            {
                padded_2D_image[i][j] = input_2D_image[H - 1][0];  // down left corner
            }
        }
        for (int j = W + pad; j < W + 2 * pad; j++)
        {
            for (int i = H + pad; i < H + 2 * pad; i++)
            {
                padded_2D_image[i][j] = input_2D_image[H - 1][W - 1];// down right corner
            }
        }
        for (int j = W + pad; j < W + 2 * pad; j++)
        {
            for (int i = 0; i < pad; i++)
            {
                padded_2D_image[i][j] = input_2D_image[0][W - 1]; // up right corner
            }
        }
    }
    else
    {
        for (int j = 0; j < prev_width; j++)
        {
            for (int i = 0; i < prev_height; i++)
            {
                padded_2D_image[i][j] = input_2D_image[i][j];
            }
        }
    }
}
///----------------------------------------------------------------------------------------
///CORRELATION FUNCTION FOR 2D INPUT
///----------------------------------------------------------------------------------------
void correlate_channel(double prev_output_channel[MAX_IN_HEIGHT][MAX_IN_WIDTH], int prev_height, int prev_width, double filter[MAX_FILTERSIZE], int filter_size, double correlate_img[MAX_IN_HEIGHT][MAX_IN_WIDTH])
{
    int stride = 1;
    int filter_width = sqrt(filter_size);
    int padding_number = int((filter_width - 1) * .5);
    int row_index_after_stride, col_index_after_stride;
    static double padded_channel[MAX_PAD_HEIGHT][MAX_PAD_WIDTH] = {0.0};
    padding(prev_output_channel, prev_height, prev_width, padding_number, padded_channel);
    double sum;
    int filter_index;
    for (int i = 0; i < MAX_IN_HEIGHT; i++)
    {
        for (int j = 0; j < MAX_IN_WIDTH; j++)
        {
            row_index_after_stride = i * stride;
            col_index_after_stride = j * stride;
            filter_index = 0;
            sum = 0;
            for (int k_x = row_index_after_stride; k_x < row_index_after_stride + filter_width; k_x++)
            {
                for (int k_y = col_index_after_stride; k_y < col_index_after_stride + filter_width; k_y++)
                {
                    sum = sum + padded_channel[k_x][k_y] * filter[filter_index];
                    filter_index = filter_index + 1;
                }
            }
            correlate_img[i][j] = sum;
        }
    }
}
///----------------------------------------------------------------------------------------
///DECCORELATION FUNCTION FOR 2D image
///----------------------------------------------------------------------------------------
void decorr_channel(double prev_output_channel[MAX_IN_HEIGHT][MAX_IN_WIDTH], int prev_height, int prev_width, double filter[MAX_FILTERSIZE], int stride, double decorrelate_img[MAX_OUT_HEIGHT][MAX_OUT_WIDTH])
{
    int filter_width = sqrt(FILTERSIZE_LAYER8); //9
    int padding_number = 1;
    static double padded_channel[MAX_PAD_HEIGHT][MAX_PAD_WIDTH];
    static double expanded_channel[MAX_EXPANDED_HEIGHT][MAX_EXPANDED_HEIGHT]; //H_e = (H_p-1)*stride+f = 43 //45
    for (int i = 0; i<MAX_EXPANDED_HEIGHT; i++)
    {
        for (int j = 0; j<MAX_EXPANDED_HEIGHT; j++)
        {
            expanded_channel[i][j] = 0;
        }
    }
    padding(prev_output_channel, prev_height, prev_width, padding_number, padded_channel);
    int H_p = prev_height + 2*padding_number;
    int W_p = prev_width + 2*padding_number;
    int H_o = prev_height * stride;
    int W_o = prev_width * stride;
    int row_index_after_stride, col_index_after_stride;
    for (int i = 0; i < H_p; i++)
    {
        for (int j = 0; j < W_p; j++)
        {
            row_index_after_stride = i * stride;
            col_index_after_stride = j * stride;
            for (int x = row_index_after_stride; x < row_index_after_stride + filter_width; x++)
            {
                for (int y = col_index_after_stride; y < col_index_after_stride + filter_width; y++)
                {
                    expanded_channel[x][y] = expanded_channel[x][y] + padded_channel[i][j] * filter[(x-row_index_after_stride)*filter_width+y-col_index_after_stride];
                }
            }
        }
    }
    int inf_lim = int((filter_width + 1) / 2) + stride * padding_number;
    int height_sup_lim = H_o + int((filter_width + 1) / 2) + stride * padding_number;
    int width_sup_lim = W_o + int((filter_width + 1) / 2) + stride * padding_number;
    for (int i = inf_lim; i < height_sup_lim; i++)
    {
        for (int j = inf_lim; j < width_sup_lim; j++)
        {
            decorrelate_img[i-inf_lim][j-inf_lim] = expanded_channel[i][j];
        }
    }
}
///-----------------------------------------------------------------------------------------
///TOP FUNCTION
///----------------------------------------------------------------------------------------
void FSRCNN(stream_t& stream_in, stream_t& stream_out, ap_uint<12> height, ap_uint<12> width, ap_uint<8> scale_factor)
{
    //Implements all ports as an AXI4-Lite interface.
    //The HLS tool produces an associated set of C driver files
    //during the Export RTL process. By default, the HLS tool
    //groups all function arguments specified as an AXI4-Lite
    //(s_axilite) interface into a single AXI4-Lite port
#pragma HLS INTERFACE s_axilite register port=height bundle=scalar_parameters
#pragma HLS INTERFACE s_axilite register port=width bundle=scalar_parameters
#pragma HLS INTERFACE s_axilite register port=scale_factor bundle=scalar_parameters
    //No block-level I/O protocol. Using the ap_ctrl_none mode might
    //prevent the design from being verified using the C/RTL co-simulation feature.
#pragma HLS INTERFACE ap_ctrl_none port=return
    //Implements all ports as an AXI4-Stream interface.
#pragma HLS INTERFACE axis register both port = stream_in
#pragma HLS INTERFACE axis register both port = stream_out
    //if you change the input image be aware that the second dimension
    //of the buffer must be larger than the the width of the input image
    static package_t buffer[MAX_IN_HEIGHT][MAX_IN_WIDTH];
    static package_t pix_out;
    static int depth = NR_OF_IMAGE_CHANNELS;
    static int out_height = height * scale_factor;
    static int out_width = width * scale_factor;
    static double out_layer_prev[MAX_IN_HEIGHT][MAX_IN_WIDTH][MAX_OUT_CHANNELS];
    static double out_layer_prev2[MAX_IN_HEIGHT][MAX_IN_WIDTH][MAX_OUT_CHANNELS];
    static double out_img[MAX_OUT_HEIGHT][MAX_OUT_WIDTH][NR_OF_IMAGE_CHANNELS];
    static double out_layer[MAX_OUT_HEIGHT][MAX_OUT_WIDTH][MAX_OUT_CHANNELS] = {0.0};
    static double img_channel[MAX_IN_HEIGHT][MAX_IN_WIDTH];
    static double channel_from_prev_out_layer[MAX_IN_HEIGHT][MAX_IN_WIDTH];
    static double subfilter_layer[MAX_FILTERSIZE];
    static double correlate_img[MAX_IN_HEIGHT][MAX_IN_WIDTH];
    static double decorrelate_img[MAX_OUT_HEIGHT][MAX_OUT_WIDTH];
    static double decorr_temp[MAX_IN_HEIGHT][MAX_IN_WIDTH];
    static double aux_sum;
    int nr = 0;
    for(int i=0; i<height; i++)
    {
        for(int j=0; j<width; j++)
        {
            stream_in>>buffer[i][j];
        }
    }
    for (int c = 0; c < depth; c++)
    {
        for (int j = 0; j < height; j++)
        {
            for (int i = 0; i < width; i++)
            {
                img_channel[j][i] = (double)buffer[j][i].data.range(8*(c+1)-1,8*c)/255;
            }
        }
        //===================Correlation for layer 1================================
        for (int i = 0; i < FILTERS_LAYER1; i++)
        {
            for (int j = 0; j < CHANNELS_LAYER1; j++)
            {
                for (int k = 0; k < FILTERSIZE_LAYER1; k++)
                {
                    subfilter_layer[k] = weights_layer1[j][k][i];
                }
            }
            correlate_channel(img_channel, height, width, subfilter_layer, FILTERSIZE_LAYER1, correlate_img);
            for (int j = 0; j < height; j++)
            {
                for (int k = 0; k < width; k++)
                {
                    out_layer[j][k][i] = out_layer[j][k][i] + correlate_img[j][k];
                    aux_sum = out_layer[j][k][i] + biases_layer1[i];
                    out_layer[j][k][i] = max(aux_sum, 0.0) + parametric_relu[0] * min(aux_sum, 0.0);
                    out_layer_prev[j][k][i] = out_layer[j][k][i];
                    out_layer[j][k][i] = 0.0;
                }
            }
        }
        //===================Correlation for layer 2================================
        for (int i = 0; i < FILTERS_LAYER2; i++)
        {
            for (int j = 0; j < CHANNELS_LAYER2; j++)
            {
                for (int k = 0; k < FILTERSIZE_LAYER2; k++)
                {
                    subfilter_layer[k] = weights_layer2[j][k][i];
                }
                for (int jj = 0; jj < height; jj++)
                {
                    for (int kk = 0; kk < width; kk++)
                    {
                        channel_from_prev_out_layer[jj][kk] = out_layer_prev[jj][kk][j];
                    }
                }
                correlate_channel(channel_from_prev_out_layer, height, width, subfilter_layer, FILTERSIZE_LAYER2, correlate_img);
                for (int jj = 0; jj < height; jj++)
                {
                    for (int kk = 0; kk < width; kk++)
                    {
                        out_layer[jj][kk][i] = out_layer[jj][kk][i] + correlate_img[jj][kk];
                    }
                }
            }
            for (int jj = 0; jj < height; jj++)
            {
                for (int kk = 0; kk < width; kk++)
                {
                    aux_sum = out_layer[jj][kk][i] + biases_layer2[i];
                    out_layer[jj][kk][i] = max(aux_sum, 0.0) + parametric_relu[1] * min(aux_sum, 0.0);
                    out_layer_prev2[jj][kk][i] = out_layer[jj][kk][i];
                    out_layer[jj][kk][i] = 0.0;
                }
            }
        }
        //===================Correlation for layer 3================================
        for (int i = 0; i < FILTERS_LAYER3; i++)
        {
            for (int j = 0; j < CHANNELS_LAYER3; j++)
            {
                for (int k = 0; k < FILTERSIZE_LAYER3; k++)
                {
                    subfilter_layer[k] = weights_layer3[j][k][i];
                }
                for (int jj = 0; jj < height; jj++)
                {
                    for (int kk = 0; kk < width; kk++)
                    {
                        channel_from_prev_out_layer[jj][kk] = out_layer_prev2[jj][kk][j];
                    }
                }
                correlate_channel(channel_from_prev_out_layer, height, width, subfilter_layer, FILTERSIZE_LAYER3, correlate_img);
                for (int jj = 0; jj < height; jj++)
                {
                    for (int kk = 0; kk < width; kk++)
                    {
                        out_layer[jj][kk][i] = out_layer[jj][kk][i] + correlate_img[jj][kk];
                    }
                }
            }
            for (int jj = 0; jj < height; jj++)
            {
                for (int kk = 0; kk < width; kk++)
                {
                    aux_sum = out_layer[jj][kk][i] + biases_layer3[i];
                    out_layer[jj][kk][i] = max(aux_sum, 0.0) + parametric_relu[2] * min(aux_sum, 0.0);
                    out_layer_prev[jj][kk][i] = out_layer[jj][kk][i];
                    out_layer[jj][kk][i] = 0.0;
                }
            }
        }
        //===================Correlation for layer 4================================
        for (int i = 0; i < FILTERS_LAYER4; i++)
        {
            for (int j = 0; j < CHANNELS_LAYER4; j++)
            {
                for (int k = 0; k < FILTERSIZE_LAYER4; k++)
                {
                    subfilter_layer[k] = weights_layer4[j][k][i];
                }
                for (int jj = 0; jj < height; jj++)
                {
                    for (int kk = 0; kk < width; kk++)
                    {
                        channel_from_prev_out_layer[jj][kk] = out_layer_prev[jj][kk][j];
                    }
                }
                correlate_channel(channel_from_prev_out_layer, height, width, subfilter_layer, FILTERSIZE_LAYER4, correlate_img);
                for (int jj = 0; jj < height; jj++)
                {
                    for (int kk = 0; kk < width; kk++)
                    {
                        out_layer[jj][kk][i] = out_layer[jj][kk][i] + correlate_img[jj][kk];
                    }
                }
            }
            for (int jj = 0; jj < height; jj++)
            {
                for (int kk = 0; kk < width; kk++)
                {
                    aux_sum = out_layer[jj][kk][i] + biases_layer4[i];
                    out_layer[jj][kk][i] = max(aux_sum, 0.0) + parametric_relu[3] * min(aux_sum, 0.0);
                    out_layer_prev2[jj][kk][i] = out_layer[jj][kk][i];
                    out_layer[jj][kk][i] = 0.0;
                }
            }
        }
        //===================Correlation for layer 5================================
        for (int i = 0; i < FILTERS_LAYER5; i++)
        {
            for (int j = 0; j < CHANNELS_LAYER5; j++)
            {
                for (int k = 0; k < FILTERSIZE_LAYER5; k++)
                {
                    subfilter_layer[k] = weights_layer5[j][k][i];
                }
                for (int jj = 0; jj < height; jj++)
                {
                    for (int kk = 0; kk < width; kk++)
                    {
                        channel_from_prev_out_layer[jj][kk] = out_layer_prev2[jj][kk][j];
                    }
                }
                correlate_channel(channel_from_prev_out_layer, height, width, subfilter_layer, FILTERSIZE_LAYER5, correlate_img);
                for (int jj = 0; jj < height; jj++)
                {
                    for (int kk = 0; kk < width; kk++)
                    {
                        out_layer[jj][kk][i] = out_layer[jj][kk][i] + correlate_img[jj][kk];
                    }
                }
            }
            for (int jj = 0; jj < height; jj++)
            {
                for (int kk = 0; kk < width; kk++)
                {
                    aux_sum = out_layer[jj][kk][i] + biases_layer5[i];
                    out_layer[jj][kk][i] = max(aux_sum, 0.0) + parametric_relu[4] * min(aux_sum, 0.0);
                    out_layer_prev[jj][kk][i] = out_layer[jj][kk][i];
                    out_layer[jj][kk][i] = 0.0;
                }
            }
        }
        //===================Correlation for layer 6================================
        for (int i = 0; i < FILTERS_LAYER6; i++)
        {
            for (int j = 0; j < CHANNELS_LAYER6; j++)
            {
                for (int k = 0; k < FILTERSIZE_LAYER6; k++)
                {
                    subfilter_layer[k] = weights_layer6[j][k][i];
                }
                for (int jj = 0; jj < height; jj++)
                {
                    for (int kk = 0; kk < width; kk++)
                    {
                        channel_from_prev_out_layer[jj][kk] = out_layer_prev[jj][kk][j];
                    }
                }
                correlate_channel(channel_from_prev_out_layer, height, width, subfilter_layer, FILTERSIZE_LAYER6, correlate_img);
                for (int jj = 0; jj < height; jj++)
                {
                    for (int kk = 0; kk < width; kk++)
                    {
                        out_layer[jj][kk][i] = out_layer[jj][kk][i] + correlate_img[jj][kk];
                    }
                }
            }
            for (int jj = 0; jj < height; jj++)
            {
                for (int kk = 0; kk < width; kk++)
                {
                    aux_sum = out_layer[jj][kk][i] + biases_layer6[i];
                    out_layer[jj][kk][i] = max(aux_sum, 0.0) + parametric_relu[5] * min(aux_sum, 0.0);
                    out_layer_prev2[jj][kk][i] = out_layer[jj][kk][i];
                    out_layer[jj][kk][i] = 0.0;
                }
            }
        }
        //===================Correlation for layer 7================================
        for (int i = 0; i < FILTERS_LAYER7; i++)
        {
            for (int j = 0; j < CHANNELS_LAYER7; j++)
            {
                for (int k = 0; k < FILTERSIZE_LAYER7; k++)
                {
                    subfilter_layer[k] = weights_layer7[j][k][i];
                }
                for (int jj = 0; jj < height; jj++)
                {
                    for (int kk = 0; kk < width; kk++)
                    {
                        channel_from_prev_out_layer[jj][kk] = out_layer_prev2[jj][kk][j];
                    }
                }
                correlate_channel(channel_from_prev_out_layer, height, width, subfilter_layer, FILTERSIZE_LAYER7, correlate_img);
                for (int jj = 0; jj < height; jj++)
                {
                    for (int kk = 0; kk < width; kk++)
                    {
                        out_layer[jj][kk][i] = out_layer[jj][kk][i] + correlate_img[jj][kk];
                    }
                }
            }
            for (int jj = 0; jj < height; jj++)
            {
                for (int kk = 0; kk < width; kk++)
                {
                    aux_sum = out_layer[jj][kk][i] + biases_layer7[i];
                    out_layer[jj][kk][i] = max(aux_sum, 0.0) + parametric_relu[6] * min(aux_sum, 0.0);
                    out_layer_prev[jj][kk][i] = out_layer[jj][kk][i];
                    out_layer[jj][kk][i] = 0.0;
                }
            }
        }
        //====================Decorrelation for layer 8=============================
        for (int i = 0; i < FILTERS_LAYER8; i++)
        {
            for (int j = 0; j < CHANNELS_LAYER8; j++)
            {
                for (int k = 0; k < FILTERSIZE_LAYER8; k++)
                {
                    subfilter_layer[k] = weights_layer8[i][k][j];
                }
                for (int k_h = 0; k_h < height; k_h++)
                {
                    for (int k_w = 0; k_w < width; k_w++)
                    {
                        decorr_temp[k_h][k_w] = out_layer_prev[k_h][k_w][j];
                    }
                }
                decorr_channel(decorr_temp, height, width, subfilter_layer, scale_factor, decorrelate_img);
                for (int jj = 0; jj < out_height; jj++)
                {
                    for (int kk = 0; kk < out_width; kk++)
                    {
                        out_layer[jj][kk][i] = out_layer[jj][kk][i] + decorrelate_img[jj][kk];
                    }
                }
            }
            for (int jj = 0; jj < out_height; jj++)
            {
                for (int kk = 0; kk < out_width; kk++)
                {
                    out_img[jj][kk][c] = out_layer[jj][kk][i] + biases_layer8; //normed c color output image
                    out_img[jj][kk][c] = out_img[jj][kk][c] * 255;             //c color output image
                    out_layer[jj][kk][i] = 0.0;
                }
            }
            //std::cout<<"-------------terminate one color--------"<<std::endl;
        }
    }
    for (int j = 0; j < out_height; j++)
    {
        for (int i = 0; i < out_width; i++)
        {
            pix_out = buffer[i][j];
            //check for overflow and repair
            for (int c = 0; c < depth; c++)
            {
                if(out_img[j][i][c]>255)
                    out_img[j][i][c] = 255;
                if(out_img[j][i][c]<0)
                    out_img[j][i][c] = 0;
            }
            pix_out.data = (int)out_img[j][i][0] + SHIFT_LEFT_WITH_8_BITS * (int)out_img[j][i][1] + SHIFT_LEFT_WITH_16_BITS * (int)out_img[j][i][2];
            stream_out<<pix_out;
        }
    }
}
