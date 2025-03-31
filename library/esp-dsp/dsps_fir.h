#ifndef _DSPS_FIR_H_
#define _DSPS_FIR_H_

#include "dsp_platform.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float* coeffs;    // Filter coefficients
    float* delay;     // Delay line
    int coeffs_len;   // Length of coefficient array
} fir_f32_t;

/**
 * @brief Initialize FIR filter structure
 *
 * @param fir Pointer to FIR filter structure
 * @param coeffs Array of filter coefficients
 * @param delay Array for delay line (must be same length as coeffs)
 * @param coeffs_len Length of coefficient array
 * @return ESP_OK on success
 */
dsp_ret_t dsps_fir_init_f32(fir_f32_t *fir, float *coeffs, float *delay, int coeffs_len);

/**
 * @brief Process FIR filter
 *
 * @param fir Pointer to FIR filter structure
 * @param input Input array
 * @param output Output array
 * @param len Length of input/output arrays
 * @return ESP_OK on success
 */
dsp_ret_t dsps_fir_f32(fir_f32_t *fir, const float *input, float *output, int len);

#ifdef __cplusplus
}
#endif

#endif // _DSPS_FIR_H_
