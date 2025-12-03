/*
 * Copyright (c) 2016 Open-RnD Sp. z o.o.
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Modified for ei2 1D CNN model - uses random array input instead of button presses
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/random/random.h>
#include <stddef.h>

#define INFERENCE_INTERVAL_SEC 5  /* Run inference every 5 seconds */
#define INPUT_ARRAY_SIZE 300      /* ei2 model expects 300 input values */

/* C-callable shim implemented in src/ei2_glue.cpp */
int ei2_classify_array(const float *input_array, size_t array_size,
                       const char **out_label, float *out_score);

void timer_expiry(struct k_timer *timer_id)
{
    /* Generate random array for proof of concept */
    static float input_array[INPUT_ARRAY_SIZE];
    
    /* Fill array with random values between -1.0 and 1.0 */
    for (size_t i = 0; i < INPUT_ARRAY_SIZE; i++) {
        /* Generate random value in range [-1.0, 1.0] */
        /* sys_rand32_get() returns uint32_t, convert to float in [-1, 1] */
        uint32_t rand_val = sys_rand32_get();
        input_array[i] = ((float)(rand_val % 2000) / 1000.0f) - 1.0f;
    }

    printk("Running inference with random array...\n");

    /* ---- Edge Impulse ei2 call (C shim in ei2_glue.cpp) ---- */
    uint32_t t0 = k_cycle_get_32();
    const char *label = NULL;
    float score = 0.0f;
    int e = ei2_classify_array(input_array, INPUT_ARRAY_SIZE,
                                &label, &score);
    uint32_t t1 = k_cycle_get_32();

    /* Convert CPU cycles -> microseconds (uses system clock freq) */
    uint32_t dt_us = k_cyc_to_us_floor32(t1 - t0);

    if (e == 0 && label) {
        printk("EI2 Class: %s (%.2f), latency: %u us (~%u inferences/s)\n",
            label, (double)score, dt_us,
            dt_us ? (1000000u / dt_us) : 0);
    } else {
        printk("EI2 classify error: %d\n", e);
    }
}

K_TIMER_DEFINE(inference_timer, timer_expiry, NULL);

int main(void)
{
    printk("EI2 1D CNN Model - Random Array Input Demo\n");
    printk("Running inference every %d seconds...\n", INFERENCE_INTERVAL_SEC);
    
    /* Start periodic timer for inference */
    k_timer_start(&inference_timer, K_SECONDS(INFERENCE_INTERVAL_SEC),
                  K_SECONDS(INFERENCE_INTERVAL_SEC));
    
    /* Run once immediately */
    timer_expiry(NULL);
    
    /* Main loop - just sleep */
    while (1) {
        k_msleep(1000);
    }
    
    return 0;
}
