#include "dsps_fir.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
#include "Arduino.h"

// Platform-specific yield implementation
static inline void dsp_yield() {
    #ifdef ESP_PLATFORM
        vTaskDelay(1); // Give 1 tick to other tasks
    #endif
}

dsp_ret_t dsps_fir_init_f32(fir_f32_t *fir, float *coeffs, float *delay, int coeffs_len) {
    if (!fir || !coeffs || !delay || coeffs_len <= 0) {
        return DSP_RET_FAIL;
    }
    
    fir->coeffs = coeffs;
    fir->delay = delay;
    fir->coeffs_len = coeffs_len;
    
    // Initialize delay line to zeros
    memset(delay, 0, coeffs_len * sizeof(float));
    
    return DSP_RET_OK;
}

dsp_ret_t dsps_fir_f32(fir_f32_t *fir, const float *input, float *output, int len) {
    if (!fir || !input || !output || len <= 0) {
        return DSP_RET_FAIL;
    }
    
    // Process in smaller chunks to prevent watchdog triggers
    const int CHUNK_SIZE = 32;
    
    for (int chunk = 0; chunk < len; chunk += CHUNK_SIZE) {
        int chunk_len = (chunk + CHUNK_SIZE > len) ? (len - chunk) : CHUNK_SIZE;
        
        for (int i = 0; i < chunk_len; i++) {
            // Use direct indexing instead of memmove for better performance
            for (int j = fir->coeffs_len - 1; j > 0; j--) {
                fir->delay[j] = fir->delay[j-1];
            }
            fir->delay[0] = input[chunk + i];
            
            // Use SIMD if available
            float sum = 0;
            #if CONFIG_DSP_OPTIMIZED
            for (int j = 0; j < fir->coeffs_len; j += 4) {
                sum += fir->delay[j] * fir->coeffs[j];
                if (j+1 < fir->coeffs_len) sum += fir->delay[j+1] * fir->coeffs[j+1];
                if (j+2 < fir->coeffs_len) sum += fir->delay[j+2] * fir->coeffs[j+2];
                if (j+3 < fir->coeffs_len) sum += fir->delay[j+3] * fir->coeffs[j+3];
            }
            #else
            for (int j = 0; j < fir->coeffs_len; j++) {
                sum += fir->delay[j] * fir->coeffs[j];
            }
            #endif
            output[chunk + i] = sum;
        }
        
        // Feed watchdog using platform-specific yield
        yield();
    }
    
    return DSP_RET_OK;
}
