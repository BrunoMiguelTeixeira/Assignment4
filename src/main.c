/*
 * Copyright (c) 2015 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/gpio.h>

#define I2C0_NODE DT_NODELABEL(tempsensor)

/* size of stack area used by each thread */
#define TASKA_STACK_SIZE 1024
/* scheduling priority used by each thread */
#define TASKA_PRIORITY 5


static const struct i2c_dt_spec dev_i2c = I2C_DT_SPEC_GET(I2C0_NODE);

uint8_t data=0x00;
int ret;
int ready=0;

void task_thread_A(void)
{
	printk("THREAD A Started!\n");
	/*updating data variable every 10ms*/
	while(1)
	{
		ret = i2c_read_dt(&dev_i2c, &data,sizeof(data));
		printk("Read %dºCelcius\n", data);
		k_msleep(200);
	}
}
/*Thread like teach */
K_THREAD_STACK_DEFINE(TaskA_stack_area, TASKA_STACK_SIZE);
struct k_thread TaskA_thread_data;
k_tid_t TaskA_tid;


void main(void)
{
	TaskA_tid = k_thread_create(&TaskA_thread_data, TaskA_stack_area,
        K_THREAD_STACK_SIZEOF(TaskA_stack_area), task_thread_A,
        NULL, NULL, NULL, TASKA_PRIORITY, 0, K_NO_WAIT);


	/*Check if the device is ready to be used*/
	if (!device_is_ready(dev_i2c.bus)) {
		printk("I2C: Device is not ready.\n");
		return;
	}

	return;

}


