#include "dssps_conv.h"
#include <string.h>

dsp_ret_t dsps_conv_f32(const float *x, int x_len, const float *y, int y_len, float *z) {
    if (!x || !y || !z || x_len <= 0 || y_len <= 0) {
        return DSP_RET_FAIL;
    }

    // Output length will be x_len + y_len - 1
    int z_len = x_len + y_len - 1;
    
    // Clear output array
    memset(z, 0, z_len * sizeof(float));
    
    // Perform convolution
    for (int i = 0; i < x_len; i++) {
        for (int j = 0; j < y_len; j++) {
            z[i + j] += x[i] * y[j];
        }
    }
    
    return DSP_RET_OK;
}