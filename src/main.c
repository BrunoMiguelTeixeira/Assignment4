#include <errno.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/gpio.h>

/* Refer to dts file */
#define I2C0_NODE DT_NODELABEL(tempsensor)
#define GPIO0_NODE DT_NODELABEL(gpio0)

/* size of stack area used by each thread */
#define STACK_SIZE 1024

/* Priority used by each thread */
#define TASK_TEMP_PRIORITY 5
#define TASK_BUTTONS_PRIORITY 5
#define TASK_LEDS_PRIORITY 5

/* Create threads stack space*/
K_THREAD_STACK_DEFINE(task_temp_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(task_buttons_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(task_leds_stack, STACK_SIZE);

/* Variable for threads data*/
struct k_thread task_temp_data;
struct k_thread task_buttons_data;
struct k_thread task_leds_data;

/* Threads ID*/
k_tid_t task_temp_tid;
k_tid_t task_buttons_tid;
k_tid_t task_leds_tid;

/* Threads prototypes */
void ButtonsUpdate(void *argA, void *argB, void *argC);
void TemperatureUpdate(void *argA, void *argB, void *argC);
void LedsUpdate(void *argA, void *argB, void *argC);

/*----*/
static const struct i2c_dt_spec dev_i2c = I2C_DT_SPEC_GET(I2C0_NODE);

const uint8_t buttons_pins[] = {11, 12, 24, 25};

uint8_t data = 0x00;
int ret;
int output_period = 10; /* ms */


/*Real Time application database*/
struct RTDB
{
	int LED0, LED1, LED2, LED3;
	int BUT0, BUT1, BUT2, BUT3;
	int TEMP;
};

void main(void)
{
	/* Check if the device is ready to be used,place holder for now */
	if (!device_is_ready(dev_i2c.bus))
	{
		printk("I2C: Device is not ready.\n");
		return;
	}

	/* Thread creation */
	task_temp_tid = k_thread_create(&task_temp_data, task_temp_stack,
									K_THREAD_STACK_SIZEOF(task_temp_stack), TemperatureUpdate,
									NULL, NULL, NULL, TASK_TEMP_PRIORITY, 0, K_NO_WAIT);

	task_buttons_tid = k_thread_create(&task_buttons_data, task_buttons_stack,
									   K_THREAD_STACK_SIZEOF(task_buttons_stack), ButtonsUpdate,
									   NULL, NULL, NULL, TASK_BUTTONS_PRIORITY, 0, K_NO_WAIT);

	task_leds_tid = k_thread_create(&task_leds_data, task_leds_stack,
									K_THREAD_STACK_SIZEOF(task_leds_stack), LedsUpdate,
									NULL, NULL, NULL, TASK_LEDS_PRIORITY, 0, K_NO_WAIT);

	return;
}

void TemperatureUpdate(void *argA, void *argB, void *argC)
{
	/*updating data variable every 10ms*/
	int64_t start_time;

	printk("THREAD A Started!\n");
	/* infinite cycle */
	while (1)
	{
		start_time = k_uptime_get();

		ret = i2c_read_dt(&dev_i2c, &data, sizeof(data));

		printk("Read %dºCelcius\n", data);
		/* Make thread sleep for the remaining of time before next check */
		if (k_uptime_get() - start_time < output_period)
		{
			/*gets the most updated remaining time to sleep*/
			k_msleep(output_period - k_uptime_get());
		}
		else
		{
			printk("Failed to meet requirement!");
		}
	}
}

void ButtonsUpdate(void *argA, void *argB, void *argC)
{
}

void LedsUpdate(void *argA, void *argB, void *argC)
{
}