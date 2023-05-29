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

/*Semaphores definition*/
K_SEM_DEFINE(temp_sem, 1, 1);

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
static const struct device *gpio0_dev = DEVICE_DT_GET(GPIO0_NODE);

const uint8_t buttons_pins[] = {11, 12, 24, 25};
const uint8_t leds_pins[] = {13, 14, 15, 16};

uint8_t data = 0x00;

int ret;
int output_period = 10; /* ms */
int input_period = 10;

/*Real Time application database*/
struct RTDB
{
	int LED0;
	int LED1;
	int LED2;
	int LED3;
	int BUT0;
	int BUT1;
	int BUT2;
	int BUT3;
	int TEMP;
};

void main(void)
{
	/* Check if the i2c connected device is ready to be used */
	if (!device_is_ready(dev_i2c.bus))
	{
		printk("I2C: Device is not ready.\n");
		return;
	}
	else
	{
		printk("I2C: Device is ready!\n");
	}

	/*Configure Buttons*/
	int i, ret;
	for (i = 0; i < sizeof(buttons_pins); i++)
	{
		ret = gpio_pin_configure(gpio0_dev, buttons_pins[i], GPIO_INPUT | GPIO_PULL_UP);
		if (ret < 0)
		{
			printk("Error: gpio_pin_configure failed for button %d/pin %d, error:%d\n\r", i + 1, buttons_pins[i], ret);
			return;
		}
		else
		{
			printk("Success: gpio_pin_configure for button %d/pin %d\n\r", i + 1, buttons_pins[i]);
		}
	}

	/*Configure Leds*/
	for (i = 0; i < sizeof(leds_pins); i++)
	{
		ret = gpio_pin_configure(gpio0_dev, leds_pins[i], GPIO_OUTPUT_ACTIVE);
		if (ret < 0)
		{
			printk("Error: gpio_pin_configure failed for led %d/pin %d, error:%d\n\r", i + 1, leds_pins[i], ret);
			return;
		}
		else
		{
			printk("Success: gpio_pin_configure for led %d/pin %d\n\r", i + 1, leds_pins[i]);
		}
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

		k_sem_take(&temp_sem, K_FOREVER); 

		// RTDB.TEMP;

		k_sem_give(&temp_sem);

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