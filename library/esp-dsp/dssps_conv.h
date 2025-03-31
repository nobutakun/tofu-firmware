#ifndef _DSPS_CONV_H_
#define _DSPS_CONV_H_

#include "dsp_platform.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Perform convolution of two arrays
 *
 * @param x First input array
 * @param x_len Length of first array
 * @param y Second input array
 * @param y_len Length of second array
 * @param z Output array (must be of length x_len + y_len - 1)
 * @return ESP_OK on success
 */
dsp_ret_t dsps_conv_f32(const float *x, int x_len, const float *y, int y_len, float *z);

#ifdef __cplusplus
}
#endif

#endif // _DSPS_CONV_H_