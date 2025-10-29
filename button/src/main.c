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

void timer_expiry(struct k_timer *timer_id){
	__disable_irq();
	if(deltaCount > 0){
		uint32_t avgCycle = deltaSum / deltaCount;	// calculates avg cycles 
		uint32_t avgMS = k_cyc_to_ms_floor32(avgCycle);
		printk("Avg Response Time: %u ms over %u presses\n",avgMS,deltaCount);

	} else {
		printk("Avg Response Time: 0\n");	
	}
	deltaSum = 0;	// sum of respones times
	deltaCount = 0;	// total count
	lastPressTime = 0;	// reference to last button press time
	printk("Number of Button Presses: %d\n",counter);
	counter = 0;	// resets button presses
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
	printk("%d\n",counter);	// prints # of button presses
	//printk("Button pressed at %" PRIu32 "\n", k_cycle_get_32());
}

int main(void)
{
	
	k_timer_start(&my_timer,K_SECONDS(2),K_SECONDS(2));	// Total time per window
	int ret;

	if (!gpio_is_ready_dt(&button)) {
		printk("Error: button device %s is not ready\n",
		       button.port->name);
		return 0;
	}

	ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
	if (ret != 0) {
		printk("Error %d: failed to configure %s pin %d\n",
		       ret, button.port->name, button.pin);
		return 0;
	}

	ret = gpio_pin_interrupt_configure_dt(&button,
					      GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0) {
		printk("Error %d: failed to configure interrupt on %s pin %d\n",
			ret, button.port->name, button.pin);
		return 0;
	}

	gpio_init_callback(&button_cb_data, button_pressed, BIT(button.pin));
	gpio_add_callback(button.port, &button_cb_data);
	printk("Set up button at %s pin %d\n", button.port->name, button.pin);

	if (led.port && !gpio_is_ready_dt(&led)) {
		printk("Error %d: LED device %s is not ready; ignoring it\n",
		       ret, led.port->name);
		led.port = NULL;
	}
	if (led.port) {
		ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT);
		if (ret != 0) {
			printk("Error %d: failed to configure LED device %s pin %d\n",
			       ret, led.port->name, led.pin);
			led.port = NULL;
		} else {
			printk("Set up LED at %s pin %d\n", led.port->name, led.pin);
		}
	}

	printk("Press the button\n");
	if (led.port) {
		while (1) {
			/* If we have an LED, match its state to the button's. */
			int val = gpio_pin_get_dt(&button);

			if (val >= 0) {
				gpio_pin_set_dt(&led, val);
			}
			k_msleep(SLEEP_TIME_MS);
		}
	}
	return 0;
}


