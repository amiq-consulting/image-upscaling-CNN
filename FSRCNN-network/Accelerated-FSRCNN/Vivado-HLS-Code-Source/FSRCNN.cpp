//Project header
#include "wrapper.hpp"
///--------------------------------------------------------------------------------------
my_float_type minn(my_float_type a){
    my_float_type zeroo = 0;
    if(a < zeroo)
        return a;
    else
        return zeroo;
}
my_float_type maxx(my_float_type a){
    my_float_type zeroo = 0;
    if(a > zeroo)
        return a;
    else
        return zeroo;
}
///----------------------------------------------------------------------------------------
///CORRELATION FUNCTION FOR 2D INPUT
///----------------------------------------------------------------------------------------
void correlate_channel(my_float_type prev_output_channel[MAX_FILTERSIZE][MAX_PAD_WIDTH], int prev_height, int prev_width, my_float_type filter[MAX_FILTERSIZE], int filter_size, my_float_type correlate_img[MAX_IN_WIDTH])
{
    int width_filter = sqrt(filter_size);
    int stride = 1;
    int column_index_after_stride;
    my_float_type sum;
    int index_filter;
    for (int index_output_value = 0; index_output_value < MAX_IN_WIDTH; index_output_value++)
    {
        column_index_after_stride = index_output_value * stride;
        index_filter = 0;
        sum = 0;
        for (int k_x = 0; k_x < width_filter; k_x++)
        {
            for (int k_y = column_index_after_stride; k_y < column_index_after_stride + width_filter; k_y++)
            {
                sum = sum + prev_output_channel[k_x][k_y] * filter[index_filter];
                index_filter = index_filter + 1;
            }
        }
        correlate_img[index_output_value] = sum;
    }
}
///----------------------------------------------------------------------------------------
///DECCORELATION FUNCTION FOR 2D input
///----------------------------------------------------------------------------------------
void decorr_channel(my_float_type padded_channel[NR_OF_NEEDED_ROWS_PER_DECORR][PAD_WIDTH_LAYER8], int prev_height, int prev_width, my_float_type filter[FILTERSIZE_LAYER8], int stride, my_float_type expanded_channel[MAX_EXPANDED_HEIGHT][MAX_EXPANDED_WIDTH])
{
    //int filter_width = sqrt(FILTERSIZE_LAYER8);
    //int padding_number = 1;
    //int W_padding = prev_width + 2*padding_number;
    //int H_extended = 2*NR_OF_NEEDED_ROWS_PER_DECORR + int((filter_width + 1) / 2) + stride * padding_number;
    //int H = NR_OF_NEEDED_ROWS_PER_DECORR;
    int row_index_after_stride, column_index_after_stride;
    for (int i = 0; i<MAX_EXPANDED_HEIGHT; i++)
    {
#pragma HLS pipeline
        for (int j = 0; j<MAX_EXPANDED_WIDTH; j++)
        {
#pragma HLS pipeline
            expanded_channel[i][j] = 0;
        }
    }
    for (int i = 0; i < NR_OF_NEEDED_ROWS_PER_DECORR; i++)
    {
        for (int j = 0; j < PAD_WIDTH_LAYER8; j++)
        {
            row_index_after_stride = i * stride;
            column_index_after_stride = j * stride;
            for (int x = row_index_after_stride; x < row_index_after_stride + FILTERSIZE1_LAYER8; x++)
            {
                for (int y = column_index_after_stride; y < column_index_after_stride + FILTERSIZE1_LAYER8; y++)
                {
                    expanded_channel[x][y] = expanded_channel[x][y] + padded_channel[i][j] * filter[(x-row_index_after_stride)*FILTERSIZE1_LAYER8+y-column_index_after_stride];
                }
            }
        }
    }
}
///----------------------------------------------------------------------------------------
///CORRELATION LAYERS
///----------------------------------------------------------------------------------------
void conrr_layer1(stream_t& stream_in, stream_t2& corr1_out, ap_uint<12> height, ap_uint<12> width, ap_uint<3> color)
{
    //subfilter_layer - 2D filter corresponding to an input channel
    static my_float_type subfilter_layer[FILTERSIZE_LAYER1];
    //correlate_img   - 2D output_correlation
    static my_float_type correlate_img[MAX_IN_WIDTH];
    //out_layer       - stores one line for every output channel (filter)
    static my_data out_layer[FILTERS_LAYER1][MAX_IN_WIDTH];
    my_float_type aux_sum;
    //buffer          - data rows from the input image
    static package_t buffer[FILTERSIZE1_LAYER1][PAD_WIDTH_LAYER1];
    //img_channel     - normalized input rows from one channel/color
    static my_float_type img_channel[MAX_FILTERSIZE][MAX_PAD_WIDTH];
    for (int current_line = 0; current_line < MAX_IN_HEIGHT + 2*PADDING_NUMBER_LAYER1; current_line++)
    {
        //shift buffer lines to make space for the new line
        if (current_line >= FILTERSIZE1_LAYER1)
        {
            for (int buffer_line = 0; buffer_line < FILTERSIZE1_LAYER1 - 1; buffer_line++)
            {
                for (int buffer_col = 0; buffer_col < MAX_IN_WIDTH + 2 * PADDING_NUMBER_LAYER1; buffer_col++)
                {
                    buffer[buffer_line][buffer_col] = buffer[buffer_line + 1][buffer_col];
                    img_channel[buffer_line][buffer_col] = img_channel[buffer_line + 1][buffer_col];
                }
            }
        }
        //read new lines, normalize the values and perform padding
        //to keep the size of the input image after correlation operation
        if (current_line == 0 || (current_line >= PADDING_NUMBER_LAYER1 + 1 && current_line < FILTERSIZE1_LAYER1 - 1))
        {
            //read a new line and normalize the values
            for (int buffer_col = PADDING_NUMBER_LAYER1; buffer_col < MAX_IN_WIDTH + PADDING_NUMBER_LAYER1; buffer_col++)
            {
#pragma HLS unroll
                stream_in>>buffer[current_line][buffer_col];
                img_channel[current_line][buffer_col] = (my_float_type)buffer[current_line][buffer_col].data.range(8*(color+1)-1,8*color)/255;
            }
            //pad the first padding_number columns
            for (int buffer_col = 0; buffer_col < PADDING_NUMBER_LAYER1; buffer_col++)
            {
#pragma HLS unroll
                buffer[current_line][buffer_col] = buffer[current_line][PADDING_NUMBER_LAYER1];
                img_channel[current_line][buffer_col] = img_channel[current_line][PADDING_NUMBER_LAYER1];
            }
            //pad the last padding_number columns
            for (int buffer_col = MAX_IN_WIDTH + PADDING_NUMBER_LAYER1; buffer_col < MAX_IN_WIDTH + 2*PADDING_NUMBER_LAYER1; buffer_col++)
            {
#pragma HLS unroll
                buffer[current_line][buffer_col] = buffer[current_line][MAX_IN_WIDTH + PADDING_NUMBER_LAYER1 - 1];
                img_channel[current_line][buffer_col] = img_channel[current_line][MAX_IN_WIDTH + PADDING_NUMBER_LAYER1 - 1];
            }
        }
        //in a nominal scenario, only the FILTERSIZE1 line will be filled with new data
        if (current_line < PADDING_NUMBER_LAYER1 + MAX_IN_HEIGHT && current_line >= FILTERSIZE1_LAYER1 -1 && current_line != 0)
        {
            //read a new line and normalize the values
            for (int buffer_col = PADDING_NUMBER_LAYER1; buffer_col < MAX_IN_WIDTH + PADDING_NUMBER_LAYER1; buffer_col++)
            {
                stream_in>>buffer[FILTERSIZE1_LAYER1 - 1][buffer_col];
                img_channel[FILTERSIZE1_LAYER1 - 1][buffer_col] = (my_float_type)buffer[FILTERSIZE1_LAYER1 - 1][buffer_col].data.range(8*(color+1)-1,8*color)/255;
            }
            //pad the first padding_number columns
            for (int buffer_col = 0; buffer_col < PADDING_NUMBER_LAYER1; buffer_col++)
            {
#pragma HLS unroll
                buffer[FILTERSIZE1_LAYER1 - 1][buffer_col] = buffer[FILTERSIZE1_LAYER1 - 1][PADDING_NUMBER_LAYER1];
                img_channel[FILTERSIZE1_LAYER1 - 1][buffer_col] = img_channel[FILTERSIZE1_LAYER1 - 1][PADDING_NUMBER_LAYER1];
            }
            //pad the last padding_number columns
            for (int buffer_col = MAX_IN_WIDTH + PADDING_NUMBER_LAYER1; buffer_col < MAX_IN_WIDTH + 2*PADDING_NUMBER_LAYER1; buffer_col++)
            {
#pragma HLS unroll
                buffer[FILTERSIZE1_LAYER1 - 1][buffer_col] = buffer[FILTERSIZE1_LAYER1 - 1][MAX_IN_WIDTH + PADDING_NUMBER_LAYER1 - 1];
                img_channel[FILTERSIZE1_LAYER1 - 1][buffer_col] = img_channel[FILTERSIZE1_LAYER1 - 1][MAX_IN_WIDTH + PADDING_NUMBER_LAYER1 - 1];
            }
        }
        //perform padding for the upper part of the input image
        if (current_line <= PADDING_NUMBER_LAYER1 && current_line > 0)
        {
            for (int buffer_col = 0; buffer_col < MAX_IN_WIDTH + 2 * PADDING_NUMBER_LAYER1; buffer_col++)
            {
#pragma HLS unroll
                buffer[current_line][buffer_col] = buffer[0][buffer_col];
                img_channel[current_line][buffer_col] = img_channel[0][buffer_col];
            }
        }
        //perform padding for the bottom part of the input image
        if (current_line >= MAX_IN_HEIGHT + PADDING_NUMBER_LAYER1)
        {
            for (int buffer_col = 0; buffer_col < MAX_IN_WIDTH + 2 * PADDING_NUMBER_LAYER1; buffer_col++)
            {
                buffer[FILTERSIZE1_LAYER1 - 1][buffer_col] = buffer[FILTERSIZE1_LAYER1 - 2][buffer_col];
                img_channel[FILTERSIZE1_LAYER1 - 1][buffer_col] = img_channel[FILTERSIZE1_LAYER1 - 2][buffer_col];
            }
        }
        //perform correlation when is enough data
        if (current_line >= FILTERSIZE1_LAYER1 - 1)
        {
            for (int current_filter = 0; current_filter < FILTERS_LAYER1; current_filter++)
            {
                for (int current_channel = 0; current_channel < CHANNELS_LAYER1; current_channel++)
                {
                    for (int filter_element = 0; filter_element < FILTERSIZE_LAYER1; filter_element++)
                    {
                        subfilter_layer[filter_element] = weights_layer1[current_channel][filter_element][current_filter];
                    }
                }
                correlate_channel(img_channel, MAX_IN_HEIGHT, MAX_IN_WIDTH, subfilter_layer, FILTERSIZE_LAYER1, correlate_img);
                for (int k = 0; k < MAX_IN_WIDTH; k++)
                {
                    out_layer[current_filter][k].data = correlate_img[k];
                    aux_sum = out_layer[current_filter][k].data + biases_layer1[current_filter];
                    out_layer[current_filter][k].data = maxx(aux_sum) + parametric_relu[0] * minn(aux_sum);
                    out_layer[current_filter][k].keep = buffer[PADDING_NUMBER_LAYER1][PADDING_NUMBER_LAYER1 + k].keep;
                    out_layer[current_filter][k].user = buffer[PADDING_NUMBER_LAYER1][PADDING_NUMBER_LAYER1 + k].user;
                    out_layer[current_filter][k].last = buffer[PADDING_NUMBER_LAYER1][PADDING_NUMBER_LAYER1 + k].last;
                    out_layer[current_filter][k].id   = buffer[PADDING_NUMBER_LAYER1][PADDING_NUMBER_LAYER1 + k].id;
                    out_layer[current_filter][k].dest = buffer[PADDING_NUMBER_LAYER1][PADDING_NUMBER_LAYER1 + k].dest;
                    corr1_out<<out_layer[current_filter][k];
                }
            }
        }
    }
}
void conrr_layer2(stream_t2& corr1_out, stream_t2& corr2_out, ap_uint<12> height, ap_uint<12> width, ap_uint<2> color)
{
    static my_float_type subfilter_layer[FILTERSIZE_LAYER2];
    static my_float_type correlate_img[MAX_IN_WIDTH];
    my_float_type aux_sum;
    static my_data img_channel[FILTERSIZE1_LAYER2][CHANNELS_LAYER2][PAD_WIDTH_LAYER2];
    static my_float_type channel_from_prev_out_layer[MAX_FILTERSIZE][MAX_PAD_WIDTH];
    for (int current_line = 0; current_line < MAX_IN_HEIGHT + 2*PADDING_NUMBER_LAYER2; current_line++)
    {
        static my_data out_layer[FILTERS_LAYER2][MAX_IN_WIDTH];
        for (int i = 0; i < FILTERS_LAYER2; i++){
           for (int j = 0; j < MAX_IN_WIDTH; j++){
               out_layer[i][j].data = 0;
           }
        }
        for (int current_input_channel = 0; current_input_channel < CHANNELS_LAYER2; current_input_channel++)
        {
            //shift buffer lines to make space for the new line of every channel
            if (current_line > FILTERSIZE1_LAYER2 - 1)
            {
                for (int filter_line = 0; filter_line < FILTERSIZE1_LAYER2 - 1; filter_line++)
                {
                    for (int index_input_element = 0; index_input_element < MAX_IN_WIDTH + 2 * PADDING_NUMBER_LAYER2; index_input_element++)
                    {
                        img_channel[filter_line][current_input_channel][index_input_element] = img_channel[filter_line+1][current_input_channel][index_input_element];
                    }
                }
            }
            //read new lines and perform padding
            //to keep the size of the input image after correlation operation
            if (current_line == 0 || (current_line >= PADDING_NUMBER_LAYER2 + 1 && current_line < FILTERSIZE1_LAYER2 - 1))
            {
                //read a new line from the current input channel
                for (int index_input_element = PADDING_NUMBER_LAYER2; index_input_element < MAX_IN_WIDTH + PADDING_NUMBER_LAYER2; index_input_element++)
                {
                    corr1_out>>img_channel[current_line][current_input_channel][index_input_element];
                }
                //pad the first padding_number columns of the current input channel
                for (int index_input_element = 0; index_input_element < PADDING_NUMBER_LAYER2; index_input_element++)
                {
                    img_channel[current_line][current_input_channel][index_input_element] = img_channel[current_line][current_input_channel][PADDING_NUMBER_LAYER2];
                }
                //pad the last padding_number columns of the current input channel
                for (int index_input_element = MAX_IN_WIDTH + PADDING_NUMBER_LAYER2; index_input_element < MAX_IN_WIDTH + 2*PADDING_NUMBER_LAYER2; index_input_element++)
                {
                    img_channel[current_line][current_input_channel][index_input_element] = img_channel[current_line][current_input_channel][MAX_IN_WIDTH + PADDING_NUMBER_LAYER2 - 1];
                }
            }
            //in a nominal scenario, only the FILTERSIZE1 line will be filled with new data
            if (current_line < PADDING_NUMBER_LAYER2 + MAX_IN_HEIGHT && current_line >= FILTERSIZE1_LAYER2 -1 && current_line != 0)
            {
                //read a new line from the current input channel
                for (int index_input_element = PADDING_NUMBER_LAYER2; index_input_element < MAX_IN_WIDTH + PADDING_NUMBER_LAYER2; index_input_element++)
                {
                    corr1_out>>img_channel[FILTERSIZE1_LAYER2 -1][current_input_channel][index_input_element];
                }
                //pad the first padding_number columns of the current input channel
                for (int index_input_element = 0; index_input_element < PADDING_NUMBER_LAYER2; index_input_element++)
                {
                    img_channel[FILTERSIZE1_LAYER2 -1][current_input_channel][index_input_element] = img_channel[FILTERSIZE1_LAYER2 -1][current_input_channel][PADDING_NUMBER_LAYER2];
                }
                //pad the last padding_number columns of the current input channel
                for (int index_input_element = MAX_IN_WIDTH + PADDING_NUMBER_LAYER2; index_input_element < MAX_IN_WIDTH + 2*PADDING_NUMBER_LAYER2; index_input_element++)
                {
                    img_channel[FILTERSIZE1_LAYER2 -1][current_input_channel][index_input_element] = img_channel[FILTERSIZE1_LAYER2 -1][current_input_channel][MAX_IN_WIDTH + PADDING_NUMBER_LAYER2 - 1];
                }
            }
            //perform padding for the upper part of the current input channel
            if (current_line <= PADDING_NUMBER_LAYER2 && current_line > 0)
            {
                for (int index_input_element = 0; index_input_element < MAX_IN_WIDTH + 2 * PADDING_NUMBER_LAYER2; index_input_element++)
                {
                    img_channel[current_line][current_input_channel][index_input_element] = img_channel[0][current_input_channel][index_input_element];
                }
            }
            //perform padding for the bottom part of the current input channel
            if (current_line >= MAX_IN_HEIGHT + PADDING_NUMBER_LAYER2)
            {
                for (int index_input_element = 0; index_input_element < MAX_IN_WIDTH + 2 * PADDING_NUMBER_LAYER2; index_input_element++)
                {
                    img_channel[FILTERSIZE1_LAYER2 - 1][current_input_channel][index_input_element] = img_channel[FILTERSIZE1_LAYER2 - 2][current_input_channel][index_input_element];
                }
            }
        }
        //perform correlation when is enough data
        if (current_line >= FILTERSIZE1_LAYER2 - 1)
        {
            for (int current_filter = 0; current_filter < FILTERS_LAYER2; current_filter++)
            {
                for (int current_input_channel = 0; current_input_channel < CHANNELS_LAYER2; current_input_channel++)
                {
                    for (int subfilter_element = 0; subfilter_element < FILTERSIZE_LAYER2; subfilter_element++)
                    {
                        subfilter_layer[subfilter_element] = weights_layer2[current_input_channel][subfilter_element][current_filter];
                    }
                    for (int input_line = 0; input_line < FILTERSIZE1_LAYER2; input_line++)
                    {
                        for (int index_input_element = 0; index_input_element < MAX_IN_WIDTH + 2*PADDING_NUMBER_LAYER2; index_input_element++)
                        {
                            channel_from_prev_out_layer[input_line][index_input_element] = img_channel[input_line][current_input_channel][index_input_element].data;
                        }
                    }
                    correlate_channel(channel_from_prev_out_layer, MAX_IN_HEIGHT, MAX_IN_WIDTH, subfilter_layer, FILTERSIZE_LAYER2, correlate_img);
                    for (int index_input_element = 0; index_input_element < MAX_IN_WIDTH; index_input_element++)
                    {
                        out_layer[current_filter][index_input_element].data = out_layer[current_filter][index_input_element].data + correlate_img[index_input_element];
                    }
                }
                for (int index_input_element = 0; index_input_element < MAX_IN_WIDTH; index_input_element++)
                {
                    aux_sum = out_layer[current_filter][index_input_element].data + biases_layer2[current_filter];
                    out_layer[current_filter][index_input_element].data = maxx(aux_sum) + parametric_relu[1] * minn(aux_sum);
                    out_layer[current_filter][index_input_element].keep = img_channel[PADDING_NUMBER_LAYER2][0][PADDING_NUMBER_LAYER2 + index_input_element].keep;
                    out_layer[current_filter][index_input_element].user = img_channel[PADDING_NUMBER_LAYER2][0][PADDING_NUMBER_LAYER2 + index_input_element].user;
                    out_layer[current_filter][index_input_element].last = img_channel[PADDING_NUMBER_LAYER2][0][PADDING_NUMBER_LAYER2 + index_input_element].last;
                    out_layer[current_filter][index_input_element].id   = img_channel[PADDING_NUMBER_LAYER2][0][PADDING_NUMBER_LAYER2 + index_input_element].id;
                    out_layer[current_filter][index_input_element].dest = img_channel[PADDING_NUMBER_LAYER2][0][PADDING_NUMBER_LAYER2 + index_input_element].dest;
                    corr2_out<<out_layer[current_filter][index_input_element];
                }
            }
        }
    }
}
void conrr_layer3(stream_t2& corr2_out, stream_t2& corr3_out, ap_uint<12> height, ap_uint<12> width, ap_uint<2> color)
{
    static my_float_type subfilter_layer[FILTERSIZE_LAYER3];
    static my_float_type correlate_img[MAX_IN_WIDTH];
    my_float_type aux_sum;
    static my_data img_channel[FILTERSIZE1_LAYER3][CHANNELS_LAYER3][PAD_WIDTH_LAYER3];
    static my_float_type channel_from_prev_out_layer[MAX_FILTERSIZE][MAX_PAD_WIDTH];
    for (int current_line = 0; current_line < MAX_IN_HEIGHT + 2*PADDING_NUMBER_LAYER3; current_line++)
    {
        static my_data out_layer[FILTERS_LAYER3][MAX_IN_WIDTH];
        for (int i = 0; i < FILTERS_LAYER3; i++){
           for (int j = 0; j < MAX_IN_WIDTH; j++){
               out_layer[i][j].data = 0;
           }
        }
        for (int current_input_channel = 0; current_input_channel < CHANNELS_LAYER3; current_input_channel++)
        {
            //shift buffer lines to make space for the new line of every channel
            if (current_line > FILTERSIZE1_LAYER3 - 1)
            {
                for (int filter_line = 0; filter_line < FILTERSIZE1_LAYER3 - 1; filter_line++)
                {
                    for (int index_input_element = 0; index_input_element < MAX_IN_WIDTH + 2 * PADDING_NUMBER_LAYER3; index_input_element++)
                    {
                        img_channel[filter_line][current_input_channel][index_input_element] = img_channel[filter_line+1][current_input_channel][index_input_element];
                    }
                }
            }
            //read new lines and perform padding to keep the size of the input
            //image after correlation operation
            if (current_line == 0 || (current_line >= PADDING_NUMBER_LAYER3 + 1 && current_line < FILTERSIZE1_LAYER3 - 1))
            {
                //read a new line from the current input channel
                for (int index_input_element = PADDING_NUMBER_LAYER3; index_input_element < MAX_IN_WIDTH + PADDING_NUMBER_LAYER3; index_input_element++)
                {
                    corr2_out>>img_channel[current_line][current_input_channel][index_input_element];
                }
                //pad the first padding_number columns of the current input channel
                for (int index_input_element = 0; index_input_element < PADDING_NUMBER_LAYER3; index_input_element++)
                {
                    img_channel[current_line][current_input_channel][index_input_element] = img_channel[current_line][current_input_channel][PADDING_NUMBER_LAYER3];
                }
                //pad the last padding_number columns of the current input channel
                for (int index_input_element = MAX_IN_WIDTH + PADDING_NUMBER_LAYER3; index_input_element < MAX_IN_WIDTH + 2*PADDING_NUMBER_LAYER3; index_input_element++)
                {
                    img_channel[current_line][current_input_channel][index_input_element] = img_channel[current_line][current_input_channel][MAX_IN_WIDTH + PADDING_NUMBER_LAYER3 - 1];
                }
            }
            //in a nominal scenario, only the FILTERSIZE1 line will be filled with new data
            if (current_line < PADDING_NUMBER_LAYER3 + MAX_IN_HEIGHT && current_line >= FILTERSIZE1_LAYER3 -1 && current_line != 0)
            {
                //read a new line from the current input channel
                for (int index_input_element = PADDING_NUMBER_LAYER3; index_input_element < MAX_IN_WIDTH + PADDING_NUMBER_LAYER3; index_input_element++)
                {
                    corr2_out>>img_channel[FILTERSIZE1_LAYER3 -1][current_input_channel][index_input_element];
                }
                //pad the first padding_number columns of the current input channel
                for (int index_input_element = 0; index_input_element < PADDING_NUMBER_LAYER3; index_input_element++)
                {
                    img_channel[FILTERSIZE1_LAYER3 -1][current_input_channel][index_input_element] = img_channel[FILTERSIZE1_LAYER3 -1][current_input_channel][PADDING_NUMBER_LAYER3];
                }
                //pad the last padding_number columns of the current input channel
                for (int index_input_element = MAX_IN_WIDTH + PADDING_NUMBER_LAYER3; index_input_element < MAX_IN_WIDTH + 2*PADDING_NUMBER_LAYER3; index_input_element++)
                {
                    img_channel[FILTERSIZE1_LAYER3 -1][current_input_channel][index_input_element] = img_channel[FILTERSIZE1_LAYER3 -1][current_input_channel][MAX_IN_WIDTH + PADDING_NUMBER_LAYER3 - 1];
                }
            }
            //perform padding for the upper part of the current input channel
            if (current_line <= PADDING_NUMBER_LAYER3 && current_line > 0)
            {
                for (int index_input_element = 0; index_input_element < MAX_IN_WIDTH + 2 * PADDING_NUMBER_LAYER3; index_input_element++)
                {
                    img_channel[current_line][current_input_channel][index_input_element] = img_channel[0][current_input_channel][index_input_element];
                }
            }
            //perform padding for the bottom part of the current input channel
            if (current_line >= MAX_IN_HEIGHT + PADDING_NUMBER_LAYER3)
            {
                for (int index_input_element = 0; index_input_element < MAX_IN_WIDTH + 2 * PADDING_NUMBER_LAYER3; index_input_element++)
                {
                    img_channel[FILTERSIZE1_LAYER3 - 1][current_input_channel][index_input_element] = img_channel[FILTERSIZE1_LAYER3 - 2][current_input_channel][index_input_element];
                }
            }
        }
        //perform correlation when is enough data
        if (current_line >= FILTERSIZE1_LAYER3 - 1)
        {
            for (int current_filter = 0; current_filter < FILTERS_LAYER3; current_filter++)
            {
                for (int current_input_channel = 0; current_input_channel < CHANNELS_LAYER3; current_input_channel++)
                {
                    for (int subfilter_element = 0; subfilter_element < FILTERSIZE_LAYER3; subfilter_element++)
                    {
                        subfilter_layer[subfilter_element] = weights_layer3[current_input_channel][subfilter_element][current_filter];
                    }
                    for (int input_line = 0; input_line < FILTERSIZE1_LAYER3; input_line++)
                    {
                        for (int index_input_element = 0; index_input_element < MAX_IN_WIDTH + 2*PADDING_NUMBER_LAYER3; index_input_element++)
                        {
                            channel_from_prev_out_layer[input_line][index_input_element] = img_channel[input_line][current_input_channel][index_input_element].data;
                        }
                    }
                    correlate_channel(channel_from_prev_out_layer, MAX_IN_HEIGHT, MAX_IN_WIDTH, subfilter_layer, FILTERSIZE_LAYER3, correlate_img);
                    for (int index_input_element = 0; index_input_element < MAX_IN_WIDTH; index_input_element++)
                    {
                        out_layer[current_filter][index_input_element].data = out_layer[current_filter][index_input_element].data + correlate_img[index_input_element];
                    }
                }
                for (int index_input_element = 0; index_input_element < MAX_IN_WIDTH; index_input_element++)
                {
                    aux_sum = out_layer[current_filter][index_input_element].data + biases_layer3[current_filter];
                    out_layer[current_filter][index_input_element].data = maxx(aux_sum) + parametric_relu[2] * minn(aux_sum);
                    out_layer[current_filter][index_input_element].keep = img_channel[PADDING_NUMBER_LAYER3][0][PADDING_NUMBER_LAYER3 + index_input_element].keep;
                    out_layer[current_filter][index_input_element].user = img_channel[PADDING_NUMBER_LAYER3][0][PADDING_NUMBER_LAYER3 + index_input_element].user;
                    out_layer[current_filter][index_input_element].last = img_channel[PADDING_NUMBER_LAYER3][0][PADDING_NUMBER_LAYER3 + index_input_element].last;
                    out_layer[current_filter][index_input_element].id   = img_channel[PADDING_NUMBER_LAYER3][0][PADDING_NUMBER_LAYER3 + index_input_element].id;
                    out_layer[current_filter][index_input_element].dest = img_channel[PADDING_NUMBER_LAYER3][0][PADDING_NUMBER_LAYER3 + index_input_element].dest;
                    corr3_out<<out_layer[current_filter][index_input_element];
                }
            }
        }
    }
}
void conrr_layer4(stream_t2& corr3_out, stream_t2& corr4_out, ap_uint<12> height, ap_uint<12> width, ap_uint<2> color)
{
    static my_float_type subfilter_layer[FILTERSIZE_LAYER4];
    static my_float_type correlate_img[MAX_IN_WIDTH];
    my_float_type aux_sum;
    static my_data img_channel[FILTERSIZE1_LAYER4][CHANNELS_LAYER4][PAD_WIDTH_LAYER3];
    static my_float_type channel_from_prev_out_layer[MAX_FILTERSIZE][MAX_PAD_WIDTH];
    for (int current_line = 0; current_line < MAX_IN_HEIGHT + 2*PADDING_NUMBER_LAYER4; current_line++)
    {
        static my_data out_layer[FILTERS_LAYER4][MAX_IN_WIDTH];
        for (int i = 0; i < FILTERS_LAYER4; i++){
           for (int j = 0; j < MAX_IN_WIDTH; j++){
               out_layer[i][j].data = 0;
           }
        }
        for (int current_input_channel = 0; current_input_channel < CHANNELS_LAYER4; current_input_channel++)
        {
            //shift buffer lines to make space for the new line of every channel
            if (current_line > FILTERSIZE1_LAYER4 - 1)
            {
                for (int filter_line = 0; filter_line < FILTERSIZE1_LAYER4 - 1; filter_line++)
                {
                    for (int index_input_element = 0; index_input_element < MAX_IN_WIDTH + 2 * PADDING_NUMBER_LAYER4; index_input_element++)
                    {
                        img_channel[filter_line][current_input_channel][index_input_element] = img_channel[filter_line+1][current_input_channel][index_input_element];
                    }
                }
            }
            //read new lines, normalize the values and perform padding
            //to keep the size of the input image after correlation operation
            if (current_line == 0 || (current_line >= PADDING_NUMBER_LAYER4 + 1 && current_line < FILTERSIZE1_LAYER4 - 1))
            {
                //read a new line from the current input channel
                for (int index_input_element = PADDING_NUMBER_LAYER4; index_input_element < MAX_IN_WIDTH + PADDING_NUMBER_LAYER4; index_input_element++)
                {
                    corr3_out>>img_channel[current_line][current_input_channel][index_input_element];
                }
                //pad the first padding_number columns of the current input channel
                for (int index_input_element = 0; index_input_element < PADDING_NUMBER_LAYER4; index_input_element++)
                {
                    img_channel[current_line][current_input_channel][index_input_element] = img_channel[current_line][current_input_channel][PADDING_NUMBER_LAYER4];
                }
                //pad the last padding_number columns of the current input channel
                for (int index_input_element = MAX_IN_WIDTH + PADDING_NUMBER_LAYER4; index_input_element < MAX_IN_WIDTH + 2*PADDING_NUMBER_LAYER4; index_input_element++)
                {
                    img_channel[current_line][current_input_channel][index_input_element] = img_channel[current_line][current_input_channel][MAX_IN_WIDTH + PADDING_NUMBER_LAYER4 - 1];
                }
            }
            //in a nominal scenario, only the FILTERSIZE1 line will be filled with new data
            if (current_line < PADDING_NUMBER_LAYER4 + MAX_IN_HEIGHT && current_line >= FILTERSIZE1_LAYER4 -1 && current_line != 0)
            {
                //read a new line from the current input channel
                for (int index_input_element = PADDING_NUMBER_LAYER4; index_input_element < MAX_IN_WIDTH + PADDING_NUMBER_LAYER4; index_input_element++)
                {
                    corr3_out>>img_channel[FILTERSIZE1_LAYER4 -1][current_input_channel][index_input_element];
                }
                //pad the first padding_number columns of the current input channel
                for (int index_input_element = 0; index_input_element < PADDING_NUMBER_LAYER4; index_input_element++)
                {
                    img_channel[FILTERSIZE1_LAYER4 -1][current_input_channel][index_input_element] = img_channel[FILTERSIZE1_LAYER4 -1][current_input_channel][PADDING_NUMBER_LAYER4];
                }
                //pad the last padding_number columns of the current input channel
                for (int index_input_element = MAX_IN_WIDTH + PADDING_NUMBER_LAYER4; index_input_element < MAX_IN_WIDTH + 2*PADDING_NUMBER_LAYER4; index_input_element++)
                {
                    img_channel[FILTERSIZE1_LAYER4 -1][current_input_channel][index_input_element] = img_channel[FILTERSIZE1_LAYER4 -1][current_input_channel][MAX_IN_WIDTH + PADDING_NUMBER_LAYER4 - 1];
                }
            }
            //perform padding for the upper part of the current input channel
            if (current_line <= PADDING_NUMBER_LAYER4 && current_line > 0)
            {
                for (int index_input_element = 0; index_input_element < MAX_IN_WIDTH + 2 * PADDING_NUMBER_LAYER4; index_input_element++)
                {
                    img_channel[current_line][current_input_channel][index_input_element] = img_channel[0][current_input_channel][index_input_element];
                }
            }
            //perform padding for the bottom part of the current input channel
            if (current_line >= MAX_IN_HEIGHT + PADDING_NUMBER_LAYER4)
            {
                for (int index_input_element = 0; index_input_element < MAX_IN_WIDTH + 2 * PADDING_NUMBER_LAYER4; index_input_element++)
                {
                    img_channel[FILTERSIZE1_LAYER4 - 1][current_input_channel][index_input_element] = img_channel[FILTERSIZE1_LAYER4 - 2][current_input_channel][index_input_element];
                }
            }
        }
        //perform correlation when is enough data
        if (current_line >= FILTERSIZE1_LAYER4 - 1)
        {
            for (int current_filter = 0; current_filter < FILTERS_LAYER4; current_filter++)
            {
                for (int current_input_channel = 0; current_input_channel < CHANNELS_LAYER4; current_input_channel++)
                {
                    for (int subfilter_element = 0; subfilter_element < FILTERSIZE_LAYER4; subfilter_element++)
                    {
                        subfilter_layer[subfilter_element] = weights_layer4[current_input_channel][subfilter_element][current_filter];
                    }
                    for (int input_line = 0; input_line < FILTERSIZE1_LAYER4; input_line++)
                    {
                        for (int index_input_element = 0; index_input_element < MAX_IN_WIDTH + 2*PADDING_NUMBER_LAYER4; index_input_element++)
                        {
                            channel_from_prev_out_layer[input_line][index_input_element] = img_channel[input_line][current_input_channel][index_input_element].data;
                        }
                    }
                    correlate_channel(channel_from_prev_out_layer, MAX_IN_HEIGHT, MAX_IN_WIDTH, subfilter_layer, FILTERSIZE_LAYER4, correlate_img);
                    for (int index_input_element = 0; index_input_element < MAX_IN_WIDTH; index_input_element++)
                    {
                        out_layer[current_filter][index_input_element].data = out_layer[current_filter][index_input_element].data + correlate_img[index_input_element];
                    }
                }
                for (int index_input_element = 0; index_input_element < MAX_IN_WIDTH; index_input_element++)
                {
                    aux_sum = out_layer[current_filter][index_input_element].data + biases_layer4[current_filter];
                    out_layer[current_filter][index_input_element].data = maxx(aux_sum) + parametric_relu[3] * minn(aux_sum);
                    out_layer[current_filter][index_input_element].keep = img_channel[PADDING_NUMBER_LAYER4][0][PADDING_NUMBER_LAYER4 + index_input_element].keep;
                    out_layer[current_filter][index_input_element].user = img_channel[PADDING_NUMBER_LAYER4][0][PADDING_NUMBER_LAYER4 + index_input_element].user;
                    out_layer[current_filter][index_input_element].last = img_channel[PADDING_NUMBER_LAYER4][0][PADDING_NUMBER_LAYER4 + index_input_element].last;
                    out_layer[current_filter][index_input_element].id   = img_channel[PADDING_NUMBER_LAYER4][0][PADDING_NUMBER_LAYER4 + index_input_element].id;
                    out_layer[current_filter][index_input_element].dest = img_channel[PADDING_NUMBER_LAYER4][0][PADDING_NUMBER_LAYER4 + index_input_element].dest;
                    corr4_out<<out_layer[current_filter][index_input_element];
                }
            }
        }
    }
}
void conrr_layer5(stream_t2& corr4_out, stream_t2& corr5_out, ap_uint<12> height, ap_uint<12> width, ap_uint<2> color)
{
    static my_float_type subfilter_layer[FILTERSIZE_LAYER5];
    static my_float_type correlate_img[MAX_IN_WIDTH];
    my_float_type aux_sum;
    static my_data img_channel[FILTERSIZE1_LAYER5][CHANNELS_LAYER5][PAD_WIDTH_LAYER5];
    static my_float_type channel_from_prev_out_layer[MAX_FILTERSIZE][MAX_PAD_WIDTH];
    for (int current_line = 0; current_line < MAX_IN_HEIGHT + 2*PADDING_NUMBER_LAYER5; current_line++)
    {
        static my_data out_layer[FILTERS_LAYER5][MAX_IN_WIDTH];
        for (int i = 0; i < FILTERS_LAYER5; i++){
           for (int j = 0; j < MAX_IN_WIDTH; j++){
               out_layer[i][j].data = 0;
           }
        }
        for (int current_input_channel = 0; current_input_channel < CHANNELS_LAYER5; current_input_channel++)
        {
            //shift buffer lines to make space for the new line of every channel
            if (current_line > FILTERSIZE1_LAYER5 - 1)
            {
                for (int filter_line = 0; filter_line < FILTERSIZE1_LAYER5 - 1; filter_line++)
                {
                    for (int index_input_element = 0; index_input_element < MAX_IN_WIDTH + 2 * PADDING_NUMBER_LAYER5; index_input_element++)
                    {
                        img_channel[filter_line][current_input_channel][index_input_element] = img_channel[filter_line+1][current_input_channel][index_input_element];
                    }
                }
            }
            //read new lines, normalize the values and perform padding
            //to keep the size of the input image after correlation operation
            if (current_line == 0 || (current_line >= PADDING_NUMBER_LAYER5 + 1 && current_line < FILTERSIZE1_LAYER5 - 1))
            {
                //read a new line from the current input channel
                for (int index_input_element = PADDING_NUMBER_LAYER5; index_input_element < MAX_IN_WIDTH + PADDING_NUMBER_LAYER5; index_input_element++)
                {
                    corr4_out>>img_channel[current_line][current_input_channel][index_input_element];
                }
                //pad the first padding_number columns of the current input channel
                for (int index_input_element = 0; index_input_element < PADDING_NUMBER_LAYER5; index_input_element++)
                {
                    img_channel[current_line][current_input_channel][index_input_element] = img_channel[current_line][current_input_channel][PADDING_NUMBER_LAYER5];
                }
                //pad the last padding_number columns of the current input channel
                for (int index_input_element = MAX_IN_WIDTH + PADDING_NUMBER_LAYER5; index_input_element < MAX_IN_WIDTH + 2*PADDING_NUMBER_LAYER5; index_input_element++)
                {
                    img_channel[current_line][current_input_channel][index_input_element] = img_channel[current_line][current_input_channel][MAX_IN_WIDTH + PADDING_NUMBER_LAYER5 - 1];
                }
            }
            //in a nominal scenario, only the FILTERSIZE1 line will be filled with new data
            if (current_line < PADDING_NUMBER_LAYER5 + MAX_IN_HEIGHT && current_line >= FILTERSIZE1_LAYER5 -1 && current_line != 0)
            {
                //read a new line from the current input channel
                for (int index_input_element = PADDING_NUMBER_LAYER5; index_input_element < MAX_IN_WIDTH + PADDING_NUMBER_LAYER5; index_input_element++)
                {
                    corr4_out>>img_channel[FILTERSIZE1_LAYER5 -1][current_input_channel][index_input_element];
                }
                //pad the first padding_number columns of the current input channel
                for (int index_input_element = 0; index_input_element < PADDING_NUMBER_LAYER5; index_input_element++)
                {
                    img_channel[FILTERSIZE1_LAYER5 -1][current_input_channel][index_input_element] = img_channel[FILTERSIZE1_LAYER5 -1][current_input_channel][PADDING_NUMBER_LAYER5];
                }
                //pad the last padding_number columns of the current input channel
                for (int index_input_element = MAX_IN_WIDTH + PADDING_NUMBER_LAYER5; index_input_element < MAX_IN_WIDTH + 2*PADDING_NUMBER_LAYER5; index_input_element++)
                {
                    img_channel[FILTERSIZE1_LAYER5 -1][current_input_channel][index_input_element] = img_channel[FILTERSIZE1_LAYER5 -1][current_input_channel][MAX_IN_WIDTH + PADDING_NUMBER_LAYER5 - 1];
                }
            }
            //perform padding for the upper part of the current input channel
            if (current_line <= PADDING_NUMBER_LAYER5 && current_line > 0)
            {
                for (int index_input_element = 0; index_input_element < MAX_IN_WIDTH + 2 * PADDING_NUMBER_LAYER5; index_input_element++)
                {
                    img_channel[current_line][current_input_channel][index_input_element] = img_channel[0][current_input_channel][index_input_element];
                }
            }
            //perform padding for the bottom part of the current input channel
            if (current_line >= MAX_IN_HEIGHT + PADDING_NUMBER_LAYER5)
            {
                for (int index_input_element = 0; index_input_element < MAX_IN_WIDTH + 2 * PADDING_NUMBER_LAYER5; index_input_element++)
                {
                    img_channel[FILTERSIZE1_LAYER5 - 1][current_input_channel][index_input_element] = img_channel[FILTERSIZE1_LAYER5 - 2][current_input_channel][index_input_element];
                }
            }
        }
        //perform correlation when is enough data
        if (current_line >= FILTERSIZE1_LAYER5 - 1)
        {
            for (int current_filter = 0; current_filter < FILTERS_LAYER5; current_filter++)
            {
                for (int current_input_channel = 0; current_input_channel < CHANNELS_LAYER5; current_input_channel++)
                {
                    for (int subfilter_element = 0; subfilter_element < FILTERSIZE_LAYER5; subfilter_element++)
                    {
                        subfilter_layer[subfilter_element] = weights_layer5[current_input_channel][subfilter_element][current_filter];
                    }
                    for (int input_line = 0; input_line < FILTERSIZE1_LAYER5; input_line++)
                    {
                        for (int index_input_element = 0; index_input_element < MAX_IN_WIDTH + 2*PADDING_NUMBER_LAYER5; index_input_element++)
                        {
                            channel_from_prev_out_layer[input_line][index_input_element] = img_channel[input_line][current_input_channel][index_input_element].data;
                        }
                    }
                    correlate_channel(channel_from_prev_out_layer, MAX_IN_HEIGHT, MAX_IN_WIDTH, subfilter_layer, FILTERSIZE_LAYER5, correlate_img);
                    for (int index_input_element = 0; index_input_element < MAX_IN_WIDTH; index_input_element++)
                    {
                        out_layer[current_filter][index_input_element].data = out_layer[current_filter][index_input_element].data + correlate_img[index_input_element];
                    }
                }
                for (int index_input_element = 0; index_input_element < MAX_IN_WIDTH; index_input_element++)
                {
                    aux_sum = out_layer[current_filter][index_input_element].data + biases_layer5[current_filter];
                    out_layer[current_filter][index_input_element].data = maxx(aux_sum) + parametric_relu[4] * minn(aux_sum);
                    out_layer[current_filter][index_input_element].keep = img_channel[PADDING_NUMBER_LAYER5][0][PADDING_NUMBER_LAYER5 + index_input_element].keep;
                    out_layer[current_filter][index_input_element].user = img_channel[PADDING_NUMBER_LAYER5][0][PADDING_NUMBER_LAYER5 + index_input_element].user;
                    out_layer[current_filter][index_input_element].last = img_channel[PADDING_NUMBER_LAYER5][0][PADDING_NUMBER_LAYER5 + index_input_element].last;
                    out_layer[current_filter][index_input_element].id   = img_channel[PADDING_NUMBER_LAYER5][0][PADDING_NUMBER_LAYER5 + index_input_element].id;
                    out_layer[current_filter][index_input_element].dest = img_channel[PADDING_NUMBER_LAYER5][0][PADDING_NUMBER_LAYER5 + index_input_element].dest;
                    corr5_out<<out_layer[current_filter][index_input_element];
                }
            }
        }
    }
}
void conrr_layer6(stream_t2& corr5_out, stream_t2& corr6_out, ap_uint<12> height, ap_uint<12> width, ap_uint<2> color)
{
    static my_float_type subfilter_layer[FILTERSIZE_LAYER6];
    static my_float_type correlate_img[MAX_IN_WIDTH];
    my_float_type aux_sum;
    static my_data img_channel[FILTERSIZE1_LAYER6][CHANNELS_LAYER6][PAD_WIDTH_LAYER6];
    static my_float_type channel_from_prev_out_layer[MAX_FILTERSIZE][MAX_PAD_WIDTH];
    for (int current_line = 0; current_line < MAX_IN_HEIGHT + 2*PADDING_NUMBER_LAYER6; current_line++)
    {
        static my_data out_layer[FILTERS_LAYER6][MAX_IN_WIDTH];
        for (int i = 0; i < FILTERS_LAYER6; i++){
           for (int j = 0; j < MAX_IN_WIDTH; j++){
               out_layer[i][j].data = 0;
           }
        }
        for (int current_input_channel = 0; current_input_channel < CHANNELS_LAYER6; current_input_channel++)
        {
            //shift buffer lines to make space for the new line of every channel
            if (current_line > FILTERSIZE1_LAYER6 - 1)
            {
                for (int filter_line = 0; filter_line < FILTERSIZE1_LAYER6 - 1; filter_line++)
                {
                    for (int index_input_element = 0; index_input_element < MAX_IN_WIDTH + 2 * PADDING_NUMBER_LAYER6; index_input_element++)
                    {
                        img_channel[filter_line][current_input_channel][index_input_element] = img_channel[filter_line+1][current_input_channel][index_input_element];
                    }
                }
            }

            //read new lines, normalize the values and perform padding
            //to keep the size of the input image after correlation operation
            if (current_line == 0 || (current_line >= PADDING_NUMBER_LAYER6 + 1 && current_line < FILTERSIZE1_LAYER6 - 1))
            {
                //read a new line from the current input channel
                for (int index_input_element = PADDING_NUMBER_LAYER6; index_input_element < MAX_IN_WIDTH + PADDING_NUMBER_LAYER6; index_input_element++)
                {
                    corr5_out>>img_channel[current_line][current_input_channel][index_input_element];
                }
                //pad the first padding_number columns of the current input channel
                for (int index_input_element = 0; index_input_element < PADDING_NUMBER_LAYER6; index_input_element++)
                {
                    img_channel[current_line][current_input_channel][index_input_element] = img_channel[current_line][current_input_channel][PADDING_NUMBER_LAYER6];
                }
                //pad the last padding_number columns of the current input channel
                for (int index_input_element = MAX_IN_WIDTH + PADDING_NUMBER_LAYER6; index_input_element < MAX_IN_WIDTH + 2*PADDING_NUMBER_LAYER6; index_input_element++)
                {
                    img_channel[current_line][current_input_channel][index_input_element] = img_channel[current_line][current_input_channel][MAX_IN_WIDTH + PADDING_NUMBER_LAYER6 - 1];
                }
            }
            //in a nominal scenario, only the FILTERSIZE1 line will be filled with new data
            if (current_line < PADDING_NUMBER_LAYER6 + MAX_IN_HEIGHT && current_line >= FILTERSIZE1_LAYER6 -1 && current_line != 0)
            {
                //read a new line from the current input channel
                for (int index_input_element = PADDING_NUMBER_LAYER6; index_input_element < MAX_IN_WIDTH + PADDING_NUMBER_LAYER6; index_input_element++)
                {
                    corr5_out>>img_channel[FILTERSIZE1_LAYER6 -1][current_input_channel][index_input_element];
                }
                //pad the first padding_number columns of the current input channel
                for (int index_input_element = 0; index_input_element < PADDING_NUMBER_LAYER6; index_input_element++)
                {
                    img_channel[FILTERSIZE1_LAYER6 -1][current_input_channel][index_input_element] = img_channel[FILTERSIZE1_LAYER6 -1][current_input_channel][PADDING_NUMBER_LAYER6];
                }
                //pad the last padding_number columns of the current input channel
                for (int index_input_element = MAX_IN_WIDTH + PADDING_NUMBER_LAYER6; index_input_element < MAX_IN_WIDTH + 2*PADDING_NUMBER_LAYER6; index_input_element++)
                {
                    img_channel[FILTERSIZE1_LAYER6 -1][current_input_channel][index_input_element] = img_channel[FILTERSIZE1_LAYER6 -1][current_input_channel][MAX_IN_WIDTH + PADDING_NUMBER_LAYER6 - 1];
                }
            }
            //perform padding for the upper part of the current input channel
            if (current_line <= PADDING_NUMBER_LAYER6 && current_line > 0)
            {
                for (int index_input_element = 0; index_input_element < MAX_IN_WIDTH + 2 * PADDING_NUMBER_LAYER6; index_input_element++)
                {
                    img_channel[current_line][current_input_channel][index_input_element] = img_channel[0][current_input_channel][index_input_element];
                }
            }
            //perform padding for the bottom part of the current input channel
            if (current_line >= MAX_IN_HEIGHT + PADDING_NUMBER_LAYER6)
            {
                for (int index_input_element = 0; index_input_element < MAX_IN_WIDTH + 2 * PADDING_NUMBER_LAYER6; index_input_element++)
                {
                    img_channel[FILTERSIZE1_LAYER6 - 1][current_input_channel][index_input_element] = img_channel[FILTERSIZE1_LAYER6 - 2][current_input_channel][index_input_element];
                }
            }
        }
        //perform correlation when is enough data
        if (current_line >= FILTERSIZE1_LAYER6 - 1)
        {
            for (int current_filter = 0; current_filter < FILTERS_LAYER6; current_filter++)
            {
                for (int current_input_channel = 0; current_input_channel < CHANNELS_LAYER6; current_input_channel++)
                {
                    for (int subfilter_element = 0; subfilter_element < FILTERSIZE_LAYER6; subfilter_element++)
                    {
                        subfilter_layer[subfilter_element] = weights_layer6[current_input_channel][subfilter_element][current_filter];
                    }
                    for (int input_line = 0; input_line < FILTERSIZE1_LAYER6; input_line++)
                    {
                        for (int index_input_element = 0; index_input_element < MAX_IN_WIDTH + 2*PADDING_NUMBER_LAYER6; index_input_element++)
                        {
                            channel_from_prev_out_layer[input_line][index_input_element] = img_channel[input_line][current_input_channel][index_input_element].data;
                        }
                    }
                    correlate_channel(channel_from_prev_out_layer, MAX_IN_HEIGHT, MAX_IN_WIDTH, subfilter_layer, FILTERSIZE_LAYER6, correlate_img);
                    for (int index_input_element = 0; index_input_element < MAX_IN_WIDTH; index_input_element++)
                    {
                        out_layer[current_filter][index_input_element].data = out_layer[current_filter][index_input_element].data + correlate_img[index_input_element];
                    }
                }
                for (int index_input_element = 0; index_input_element < MAX_IN_WIDTH; index_input_element++)
                {
                    aux_sum = out_layer[current_filter][index_input_element].data + biases_layer6[current_filter];
                    out_layer[current_filter][index_input_element].data = maxx(aux_sum) + parametric_relu[5] * minn(aux_sum);
                    out_layer[current_filter][index_input_element].keep = img_channel[PADDING_NUMBER_LAYER6][0][PADDING_NUMBER_LAYER6 + index_input_element].keep;
                    out_layer[current_filter][index_input_element].user = img_channel[PADDING_NUMBER_LAYER6][0][PADDING_NUMBER_LAYER6 + index_input_element].user;
                    out_layer[current_filter][index_input_element].last = img_channel[PADDING_NUMBER_LAYER6][0][PADDING_NUMBER_LAYER6 + index_input_element].last;
                    out_layer[current_filter][index_input_element].id   = img_channel[PADDING_NUMBER_LAYER6][0][PADDING_NUMBER_LAYER6 + index_input_element].id;
                    out_layer[current_filter][index_input_element].dest = img_channel[PADDING_NUMBER_LAYER6][0][PADDING_NUMBER_LAYER6 + index_input_element].dest;
                    corr6_out<<out_layer[current_filter][index_input_element];
                }
            }
        }
    }
}
void conrr_layer7(stream_t2& corr6_out, stream_t2& corr7_out, ap_uint<12> height, ap_uint<12> width, ap_uint<2> color)
{
    //int nr = 0;
    static my_float_type subfilter_layer[FILTERSIZE_LAYER7];
    static my_float_type correlate_img[MAX_IN_WIDTH];
    my_float_type aux_sum;
    static my_data img_channel[FILTERSIZE1_LAYER7][CHANNELS_LAYER7][PAD_WIDTH_LAYER7];
    static my_float_type channel_from_prev_out_layer[MAX_FILTERSIZE][MAX_PAD_WIDTH];
    for (int current_line = 0; current_line < MAX_IN_HEIGHT + 2*PADDING_NUMBER_LAYER7; current_line++)
    {
        static my_data out_layer[FILTERS_LAYER7][MAX_IN_WIDTH];
        for (int i = 0; i < FILTERS_LAYER7; i++){
           for (int j = 0; j < MAX_IN_WIDTH; j++){
               out_layer[i][j].data = 0;
           }
        }
        for (int current_input_channel = 0; current_input_channel < CHANNELS_LAYER7; current_input_channel++)
        {
            //shift buffer lines to make space for the new line of every channel
            if (current_line > FILTERSIZE1_LAYER7 - 1)
            {
                for (int filter_line = 0; filter_line < FILTERSIZE1_LAYER7 - 1; filter_line++)
                {
                    for (int index_input_element = 0; index_input_element < MAX_IN_WIDTH + 2 * PADDING_NUMBER_LAYER7; index_input_element++)
                    {
                        img_channel[filter_line][current_input_channel][index_input_element] = img_channel[filter_line+1][current_input_channel][index_input_element];
                    }
                }
            }
            //read new lines, normalize the values and perform padding
            //to keep the size of the input image after correlation operation
            if (current_line == 0 || (current_line >= PADDING_NUMBER_LAYER7 + 1 && current_line < FILTERSIZE1_LAYER7 - 1))
            {
                //read a new line from the current input channel
                for (int index_input_element = PADDING_NUMBER_LAYER7; index_input_element < MAX_IN_WIDTH + PADDING_NUMBER_LAYER7; index_input_element++)
                {
                    corr6_out>>img_channel[current_line][current_input_channel][index_input_element];
                }
                //pad the first padding_number columns of the current input channel
                for (int index_input_element = 0; index_input_element < PADDING_NUMBER_LAYER7; index_input_element++)
                {
                    img_channel[current_line][current_input_channel][index_input_element] = img_channel[current_line][current_input_channel][PADDING_NUMBER_LAYER7];
                }
                //pad the last padding_number columns of the current input channel
                for (int index_input_element = MAX_IN_WIDTH + PADDING_NUMBER_LAYER7; index_input_element < MAX_IN_WIDTH + 2*PADDING_NUMBER_LAYER7; index_input_element++)
                {
                    img_channel[current_line][current_input_channel][index_input_element] = img_channel[current_line][current_input_channel][MAX_IN_WIDTH + PADDING_NUMBER_LAYER7 - 1];
                }
            }
            //in a nominal scenario, only the FILTERSIZE1 line will be filled with new data
            if (current_line < PADDING_NUMBER_LAYER7 + MAX_IN_HEIGHT && current_line >= FILTERSIZE1_LAYER7 -1 && current_line != 0)
            {
                //read a new line from the current input channel
                for (int index_input_element = PADDING_NUMBER_LAYER7; index_input_element < MAX_IN_WIDTH + PADDING_NUMBER_LAYER7; index_input_element++)
                {
                    corr6_out>>img_channel[FILTERSIZE1_LAYER7 -1][current_input_channel][index_input_element];
                }
                //pad the first padding_number columns of the current input channel
                for (int index_input_element = 0; index_input_element < PADDING_NUMBER_LAYER7; index_input_element++)
                {
                    img_channel[FILTERSIZE1_LAYER7 -1][current_input_channel][index_input_element] = img_channel[FILTERSIZE1_LAYER7 -1][current_input_channel][PADDING_NUMBER_LAYER7];
                }
                //pad the last padding_number columns of the current input channel
                for (int index_input_element = MAX_IN_WIDTH + PADDING_NUMBER_LAYER7; index_input_element < MAX_IN_WIDTH + 2*PADDING_NUMBER_LAYER7; index_input_element++)
                {
                    img_channel[FILTERSIZE1_LAYER7 -1][current_input_channel][index_input_element] = img_channel[FILTERSIZE1_LAYER7 -1][current_input_channel][MAX_IN_WIDTH + PADDING_NUMBER_LAYER7 - 1];
                }
            }
            //perform padding for the upper part of the current input channel
            if (current_line <= PADDING_NUMBER_LAYER7 && current_line > 0)
            {
                for (int index_input_element = 0; index_input_element < MAX_IN_WIDTH + 2 * PADDING_NUMBER_LAYER7; index_input_element++)
                {
                    img_channel[current_line][current_input_channel][index_input_element] = img_channel[0][current_input_channel][index_input_element];
                }
            }
            //perform padding for the bottom part of the current input channel
            if (current_line >= MAX_IN_HEIGHT + PADDING_NUMBER_LAYER7)
            {
                for (int index_input_element = 0; index_input_element < MAX_IN_WIDTH + 2 * PADDING_NUMBER_LAYER7; index_input_element++)
                {
                    img_channel[FILTERSIZE1_LAYER7 - 1][current_input_channel][index_input_element] = img_channel[FILTERSIZE1_LAYER7 - 2][current_input_channel][index_input_element];
                }
            }
        }
        //perform correlation when is enough data
        if (current_line >= FILTERSIZE1_LAYER7 - 1)
        {
            for (int current_filter = 0; current_filter < FILTERS_LAYER7; current_filter++)
            {
                for (int current_input_channel = 0; current_input_channel < CHANNELS_LAYER7; current_input_channel++)
                {
                    for (int subfilter_element = 0; subfilter_element < FILTERSIZE_LAYER7; subfilter_element++)
                    {
                        subfilter_layer[subfilter_element] = weights_layer7[current_input_channel][subfilter_element][current_filter];
                    }
                    for (int input_line = 0; input_line < FILTERSIZE1_LAYER7; input_line++)
                    {
                        for (int index_input_element = 0; index_input_element < MAX_IN_WIDTH + 2*PADDING_NUMBER_LAYER7; index_input_element++)
                        {
                            channel_from_prev_out_layer[input_line][index_input_element] = img_channel[input_line][current_input_channel][index_input_element].data;
                        }
                    }
                    correlate_channel(channel_from_prev_out_layer, MAX_IN_HEIGHT, MAX_IN_WIDTH, subfilter_layer, FILTERSIZE_LAYER7, correlate_img);
                    for (int index_input_element = 0; index_input_element < MAX_IN_WIDTH; index_input_element++)
                    {
                        out_layer[current_filter][index_input_element].data = out_layer[current_filter][index_input_element].data + correlate_img[index_input_element];
                    }
                }
                for (int index_input_element = 0; index_input_element < MAX_IN_WIDTH; index_input_element++)
                {
                    aux_sum = out_layer[current_filter][index_input_element].data + biases_layer7[current_filter];
                    out_layer[current_filter][index_input_element].data = maxx(aux_sum) + parametric_relu[6] * minn(aux_sum);
                    out_layer[current_filter][index_input_element].keep = img_channel[PADDING_NUMBER_LAYER7][0][PADDING_NUMBER_LAYER7 + index_input_element].keep;
                    out_layer[current_filter][index_input_element].user = img_channel[PADDING_NUMBER_LAYER7][0][PADDING_NUMBER_LAYER7 + index_input_element].user;
                    out_layer[current_filter][index_input_element].last = img_channel[PADDING_NUMBER_LAYER7][0][PADDING_NUMBER_LAYER7 + index_input_element].last;
                    out_layer[current_filter][index_input_element].id   = img_channel[PADDING_NUMBER_LAYER7][0][PADDING_NUMBER_LAYER7 + index_input_element].id;
                    out_layer[current_filter][index_input_element].dest = img_channel[PADDING_NUMBER_LAYER7][0][PADDING_NUMBER_LAYER7 + index_input_element].dest;

                    corr7_out<<out_layer[current_filter][index_input_element];
                }
            }
        }
    }
}
///----------------------------------------------------------------------------------------
///DECORRELATION LAYER
///----------------------------------------------------------------------------------------
void decorr_layer8(stream_t2& corr7_out, stream_t2& corr8_out, ap_uint<12> height, ap_uint<12> width, ap_uint<8> scale_factor, ap_uint<2> color)
{
    static my_float_type subfilter_layer[FILTERSIZE_LAYER8];
    static my_float_type decorrelate_img[NR_OF_LAST_OUTPUT_LINES][MAX_OUT_WIDTH];
    static my_float_type decorr_temp[NR_OF_NEEDED_ROWS_PER_DECORR][PAD_WIDTH_LAYER8];
    my_data img_channel[NR_OF_NEEDED_ROWS_PER_DECORR][CHANNELS_LAYER8][PAD_WIDTH_LAYER8];
    static my_float_type expanded_channel[MAX_EXPANDED_HEIGHT][MAX_EXPANDED_WIDTH];
    //int out_height = MAX_IN_HEIGHT * scale_factor;
    //int out_width = MAX_IN_WIDTH * scale_factor;
    //int crop_edge_lines = int((FILTERSIZE1_LAYER8 + 1) / 2) + scale_factor * PADDING_NUMBER_LAYER8; //7
    for (int current_line = 0; current_line < MAX_IN_HEIGHT + 2*PADDING_NUMBER_LAYER8; current_line++)
    {
        static my_data out_layer[NR_OF_LAST_OUTPUT_LINES][FILTERS_LAYER8][MAX_OUT_WIDTH];
        for (int k = 0; k < NR_OF_LAST_OUTPUT_LINES; k++){
            for (int i = 0; i < FILTERS_LAYER8; i++){
                for (int j = 0; j < MAX_OUT_WIDTH; j++){
                    out_layer[k][i][j].data = 0;
                }
            }
        }
        for (int current_input_channel = 0; current_input_channel < CHANNELS_LAYER8; current_input_channel++)
        {
            //shift buffer lines to make space for the new line of every channel
            if (current_line > NR_OF_NEEDED_ROWS_PER_DECORR - 1)//3
            {
                for (int filter_line = 0; filter_line < NR_OF_NEEDED_ROWS_PER_DECORR - 1; filter_line++)
                {
                    for (int index_input_element = 0; index_input_element < MAX_IN_WIDTH + 2 * PADDING_NUMBER_LAYER8; index_input_element++)
                    {
                        img_channel[filter_line][current_input_channel][index_input_element] = img_channel[filter_line+1][current_input_channel][index_input_element];
                    }
                }
            }
            //read new lines and perform padding
            if (current_line == 0 || (current_line >= PADDING_NUMBER_LAYER8 + 1 && current_line < NR_OF_NEEDED_ROWS_PER_DECORR - 1))
            {
                //read a new line from the current input channel
                for (int index_input_element = PADDING_NUMBER_LAYER8; index_input_element < MAX_IN_WIDTH + PADDING_NUMBER_LAYER8; index_input_element++)
                {
                    corr7_out>>img_channel[current_line][current_input_channel][index_input_element];
                }
                //pad the first padding_number columns of the current input channel
                for (int index_input_element = 0; index_input_element < PADDING_NUMBER_LAYER8; index_input_element++)
                {
                    img_channel[current_line][current_input_channel][index_input_element] = img_channel[current_line][current_input_channel][PADDING_NUMBER_LAYER8];
                }
                //pad the last padding_number columns of the current input channel
                for (int index_input_element = MAX_IN_WIDTH + PADDING_NUMBER_LAYER8; index_input_element < MAX_IN_WIDTH + 2*PADDING_NUMBER_LAYER8; index_input_element++)
                {
                    img_channel[current_line][current_input_channel][index_input_element] = img_channel[current_line][current_input_channel][MAX_IN_WIDTH + PADDING_NUMBER_LAYER8 - 1];
                }
            }
            //in a nominal scenario, only the NR_OF_NEEDED_ROWS_PER_DECORR line will be filled with new data
            if (current_line < PADDING_NUMBER_LAYER8 + MAX_IN_HEIGHT && current_line >= NR_OF_NEEDED_ROWS_PER_DECORR - 1 && current_line != 0)
            {
                //read a new line from the current input channel
                for (int index_input_element = PADDING_NUMBER_LAYER8; index_input_element < MAX_IN_WIDTH + PADDING_NUMBER_LAYER8; index_input_element++)
                {
                    corr7_out>>img_channel[NR_OF_NEEDED_ROWS_PER_DECORR - 1][current_input_channel][index_input_element];
                }
                //pad the first padding_number columns of the current input channel
                for (int index_input_element = 0; index_input_element < PADDING_NUMBER_LAYER8; index_input_element++)
                {
                    img_channel[NR_OF_NEEDED_ROWS_PER_DECORR -1][current_input_channel][index_input_element] = img_channel[NR_OF_NEEDED_ROWS_PER_DECORR -1][current_input_channel][PADDING_NUMBER_LAYER8];
                }
                //pad the last padding_number columns of the current input channel
                for (int index_input_element = MAX_IN_WIDTH + PADDING_NUMBER_LAYER8; index_input_element < MAX_IN_WIDTH + 2*PADDING_NUMBER_LAYER8; index_input_element++)
                {
                    img_channel[NR_OF_NEEDED_ROWS_PER_DECORR -1][current_input_channel][index_input_element] = img_channel[NR_OF_NEEDED_ROWS_PER_DECORR -1][current_input_channel][MAX_IN_WIDTH + PADDING_NUMBER_LAYER8 - 1];
                }
            }
            //perform padding for the upper part of the current input channel
            if (current_line <= PADDING_NUMBER_LAYER8 && current_line > 0)
            {
                for (int index_input_element = 0; index_input_element < MAX_IN_WIDTH + 2 * PADDING_NUMBER_LAYER8; index_input_element++)
                {
                    img_channel[current_line][current_input_channel][index_input_element] = img_channel[0][current_input_channel][index_input_element];
                }
            }
            //perform padding for the bottom part of the current input channel
            if (current_line >= MAX_IN_HEIGHT + PADDING_NUMBER_LAYER8)
            {
                for (int index_input_element = 0; index_input_element < MAX_IN_WIDTH + 2 * PADDING_NUMBER_LAYER8; index_input_element++)
                {
                    img_channel[NR_OF_NEEDED_ROWS_PER_DECORR - 1][current_input_channel][index_input_element] = img_channel[NR_OF_NEEDED_ROWS_PER_DECORR - 2][current_input_channel][index_input_element];
                }
            }
        }
        //perform correlation when is enough data
        if (current_line >= NR_OF_NEEDED_ROWS_PER_DECORR - 1)
        {
            for (int current_filter = 0; current_filter < 1; current_filter++)
            {
                for (int current_input_channel = 0; current_input_channel < CHANNELS_LAYER8; current_input_channel++)
                {
                    for (int subfilter_element = 0; subfilter_element < FILTERSIZE_LAYER8; subfilter_element++)
                    {
                        subfilter_layer[subfilter_element] = weights_layer8[current_filter][subfilter_element][current_input_channel];
                    }
                    //extract NEEDED_NUMBER_OF_ROWS_PER_DECORR lines from the current channel
                    //to perform decorrelation for the current channel
                    for (int input_line = 0; input_line < NR_OF_NEEDED_ROWS_PER_DECORR; input_line++)
                    {
                        for (int index_input_element = 0; index_input_element < MAX_IN_WIDTH + 2*PADDING_NUMBER_LAYER8; index_input_element++)
                        {
                            decorr_temp[input_line][index_input_element] = img_channel[input_line][current_input_channel][index_input_element].data;
                        }
                    }
                    //perform deccorelation for the current channel
                    decorr_channel(decorr_temp, MAX_IN_HEIGHT, MAX_IN_WIDTH, subfilter_layer, SCALE_FACT, expanded_channel);
                    //extract only useful data
                    if (current_line < MAX_IN_HEIGHT +  2*PADDING_NUMBER_LAYER8 - 1)
                    {
                        for (int output_line = 0; output_line < SCALE_FACT; output_line++)
                        {
                            for (int output_col = CROP_EDGE_LINES; output_col < MAX_OUT_WIDTH + CROP_EDGE_LINES; output_col++)
                            {
                                    decorrelate_img[output_line][output_col-CROP_EDGE_LINES] = expanded_channel[CROP_EDGE_LINES + output_line][output_col];
                            }
                            for (int index_input_element = 0; index_input_element < MAX_OUT_WIDTH; index_input_element++)
                            {
                                out_layer[output_line][current_filter][index_input_element].data = out_layer[output_line][current_filter][index_input_element].data + decorrelate_img[output_line][index_input_element];
                            }
                        }
                    }
                    else
                    {
                        for (int output_line = 0; output_line < NR_OF_LAST_OUTPUT_LINES; output_line++)
                        {
                            for (int output_col = CROP_EDGE_LINES; output_col < MAX_OUT_WIDTH + CROP_EDGE_LINES; output_col++)
                            {
                                decorrelate_img[output_line][output_col-CROP_EDGE_LINES] = expanded_channel[CROP_EDGE_LINES + output_line][output_col];
                            }
                            for (int index_input_element = 0; index_input_element < MAX_OUT_WIDTH; index_input_element++)
                            {
                                out_layer[output_line][current_filter][index_input_element].data = out_layer[output_line][current_filter][index_input_element].data + decorrelate_img[output_line][index_input_element];
                            }
                        }
                    }
                }
                if (current_line < MAX_IN_HEIGHT +  2*PADDING_NUMBER_LAYER8 - 1)
                {
                    for (int output_line = 0; output_line < SCALE_FACT; output_line++)
                    {
                        for (int index_input_element = 0; index_input_element < MAX_OUT_WIDTH; index_input_element++)
                        {
                            out_layer[output_line][current_filter][index_input_element].data = (out_layer[output_line][current_filter][index_input_element].data + biases_layer8);

                            out_layer[output_line][current_filter][index_input_element].dest = img_channel[PADDING_NUMBER_LAYER8 + output_line][0][PADDING_NUMBER_LAYER8 + (index_input_element%MAX_IN_WIDTH)].dest;
                            out_layer[output_line][current_filter][index_input_element].id   = img_channel[PADDING_NUMBER_LAYER8 + output_line][0][PADDING_NUMBER_LAYER8 + (index_input_element%MAX_IN_WIDTH)].id;
                            out_layer[output_line][current_filter][index_input_element].keep = img_channel[PADDING_NUMBER_LAYER8 + output_line][0][PADDING_NUMBER_LAYER8 + (index_input_element%MAX_IN_WIDTH)].keep;
                            out_layer[output_line][current_filter][index_input_element].last = img_channel[PADDING_NUMBER_LAYER8 + output_line][0][PADDING_NUMBER_LAYER8 + (index_input_element%MAX_IN_WIDTH)].last;
                            if(index_input_element == 0 && current_line == 4 && output_line == 0)
                                out_layer[output_line][current_filter][index_input_element].user = 1;
                            else
                                out_layer[output_line][current_filter][index_input_element].user = 0;
                            corr8_out<<out_layer[output_line][current_filter][index_input_element];
                        }
                    }
                }
                else
                {
                    for (int output_line = 0; output_line < NR_OF_LAST_OUTPUT_LINES; output_line++)
                    {
                        for (int index_input_element = 0; index_input_element < MAX_OUT_WIDTH; index_input_element++)
                        {
                            out_layer[output_line][current_filter][index_input_element].data = (out_layer[output_line][current_filter][index_input_element].data + biases_layer8);

                            out_layer[output_line][current_filter][index_input_element].dest = img_channel[PADDING_NUMBER_LAYER8][0][PADDING_NUMBER_LAYER8 + (index_input_element%MAX_IN_WIDTH)].dest;
                            out_layer[output_line][current_filter][index_input_element].id   = img_channel[PADDING_NUMBER_LAYER8][0][PADDING_NUMBER_LAYER8 + (index_input_element%MAX_IN_WIDTH)].id;
                            out_layer[output_line][current_filter][index_input_element].keep = img_channel[PADDING_NUMBER_LAYER8][0][PADDING_NUMBER_LAYER8 + (index_input_element%MAX_IN_WIDTH)].keep;
                            out_layer[output_line][current_filter][index_input_element].last = img_channel[PADDING_NUMBER_LAYER8][0][PADDING_NUMBER_LAYER8 + (index_input_element%MAX_IN_WIDTH)].last;
                            out_layer[output_line][current_filter][index_input_element].user = 0;
                            corr8_out<<out_layer[output_line][current_filter][index_input_element];
                        }
                    }
                }
            }
        }
    }
}
///----------------------------------------------------------------------------------------
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
    stream_t2 corr1_out("corr1_out");
    stream_t2 corr2_out("corr2_out");
    stream_t2 corr3_out("corr3_out");
    stream_t2 corr4_out("corr4_out");
    stream_t2 corr5_out("corr5_out");
    stream_t2 corr6_out("corr6_out");
    stream_t2 corr7_out("corr7_out");
    stream_t2 corr8_out("corr8_out");
    my_data pix_in;
    package_t pix_out;
    float data_util;
    int color = 0;
#pragma HLS dataflow
    conrr_layer1(stream_in, corr1_out, height, width, color);
    conrr_layer2(corr1_out, corr2_out, height, width, color);
    conrr_layer3(corr2_out, corr3_out, height, width, color);
    conrr_layer4(corr3_out, corr4_out, height, width, color);
    conrr_layer5(corr4_out, corr5_out, height, width, color);
    conrr_layer6(corr5_out, corr6_out, height, width, color);
    conrr_layer7(corr6_out, corr7_out, height, width, color);
    decorr_layer8(corr7_out, corr8_out, height, width, scale_factor, color);
    for (int i = 0; i < MAX_OUT_HEIGHT*MAX_OUT_WIDTH; i++)
    {
#pragma HLS pipeline
        corr8_out>>pix_in;
        data_util = (pix_in.data*UNNORMALIZE_CONSTANT);
        //repair overflow
        if(data_util > 255)
            data_util = 255;
        if(data_util < 0)
            data_util = 0;
        pix_out.data = (int)data_util;
        pix_out.dest = pix_in.dest;
        pix_out.id   = pix_in.id;
        pix_out.keep = pix_in.keep;
        pix_out.last = pix_in.last;
        pix_out.strb = 0;
        pix_out.user = pix_in.user;
        stream_out<<pix_out;
    }
}
