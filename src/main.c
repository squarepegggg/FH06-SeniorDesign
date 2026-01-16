/*
 * Copyright (c) 2016 Open-RnD Sp. z o.o.
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * NOTE: If you are looking into an implementation of button events with
 * debouncing, check out `input` subsystem and `samples/subsys/input/input_dump`
 * example instead.
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/util.h>
#include <zephyr/sys/printk.h>
#include <inttypes.h>

#define SLEEP_TIME_MS	1
#define WINDOW 10
#define WINDOW_MS 2000  /* your 2-second window in milliseconds */


/*
 * Get button configuration from the devicetree sw0 alias. This is mandatory.
 */
#define SW0_NODE	DT_ALIAS(sw0)
#define TIMER_INSTANCE 0
#if !DT_NODE_HAS_STATUS_OKAY(SW0_NODE)
#error "Unsupported board: sw0 devicetree alias is not defined"
#endif
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET_OR(SW0_NODE, gpios,
							      {0});
static struct gpio_callback button_cb_data;
static uint32_t counter = 0;	// counter per button presses
uint32_t lastPressTime = 0;
uint32_t deltaSum = 0;
uint32_t deltaCount = 0;
bool filled = false;
/* C-callable shim implemented in src/ei_glue.cpp */
int ei_classify_three(float presses, float avg_ms, float window_ms,
                      const char **out_label, float *out_score);
int ei_classify_csv(const char *csv_file_path);

void timer_expiry(struct k_timer *timer_id){
	/* Take a snapshot of current stats while interrupts are off */
	__disable_irq();

	/* Compute average only if we have at least one interval */
	uint32_t avgMS = 0;
	if (deltaCount > 0) {
		uint32_t avgCycle = deltaSum / deltaCount;          /* average cycles */
		avgMS = k_cyc_to_ms_floor32(avgCycle);              /* cycles -> ms */
		printk("Avg Response Time: %u ms over %u presses\n", avgMS, deltaCount);
	} else {
		printk("Avg Response Time: 0\n");
	}

	/* Snapshot values we'll feed to the classifier BEFORE resetting */
	uint32_t presses_snapshot   = counter;
	uint32_t avg_ms_snapshot    = avgMS;

	/* ---- Edge Impulse call (C shim in ei_glue.cpp) ---- */
	uint32_t t0 = k_cycle_get_32();
	const char *label = NULL;
	float score = 0.0f;
	int e = ei_classify_three((float)presses_snapshot,
							(float)avg_ms_snapshot,
							(float)WINDOW_MS,
							&label, &score);
	uint32_t t1 = k_cycle_get_32();

	/* Convert CPU cycles -> microseconds (uses system clock freq) */
	uint32_t dt_us = k_cyc_to_us_floor32(t1 - t0);

	if (e == 0 && label) {
		printk("EI Class: %s (%.2f), latency: %u us (~%u inferences/s)\n",
			label, (double)score, dt_us,
			dt_us ? (1000000u / dt_us) : 0);
	} else {
		printk("EI classify error: %d\n", e);
	}

	/* Print and then reset counters for the next window */
	printk("Number of Button Presses: %u\n", presses_snapshot);

	/* Reset state for next window */
	deltaSum = 0;
	deltaCount = 0;
	lastPressTime = 0;
	counter = 0;

	__enable_irq();
}


K_TIMER_DEFINE(my_timer,timer_expiry,NULL);


/*
 * The led0 devicetree alias is optional. If present, we'll use it
 * to turn on the LED whenever the button is pressed.
 */
static struct gpio_dt_spec led = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led0), gpios,
						     {0});

void button_pressed(const struct device *dev, struct gpio_callback *cb,
		    uint32_t pins)
{
	uint32_t now = k_cycle_get_32();
	if(lastPressTime != 0){
		uint32_t delta = now - lastPressTime;
		deltaSum += delta;
		deltaCount++;
	}
	lastPressTime = now;
	counter++;
	printk("%u\n",counter);	// prints # of button presses
	//printk("Button pressed at %" PRIu32 "\n", k_cycle_get_32());
}

int main(void)
{
	printk("Starting CSV classification...\n");

    // The path to the CSV file.
    // NOTE: This assumes your device has a filesystem where this file is located.
    // If running in a simulator, this path might work directly.
    // For real hardware, you'll need to load the file onto a flash filesystem.
    const char *csv_path = "mock_3axis_25window_edge_impulse.csv";

    int result = ei_classify_csv(csv_path);

    if (result == 0) {
        printk("CSV classification completed successfully.\n");
    } else {
        printk("CSV classification failed with error code: %d\n", result);
    }

    // Idle the main thread
    while (1) {
        k_msleep(1000);
    }

	return 0;
}