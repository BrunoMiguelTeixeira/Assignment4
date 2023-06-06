#include <errno.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>
#include <string.h>
#include "cmdproc.h"


/* Refer to dts file */
#define I2C0_NODE DT_NODELABEL(tempsensor)
#define GPIO0_NODE DT_NODELABEL(gpio0)
#define UART0_NODE DT_NODELABEL(uart0)

/* size of stack area used by each thread */
#define STACK_SIZE 1024

/* Priority used by each thread */
#define TASK_TEMP_PRIORITY 5
#define TASK_BUTTONS_PRIORITY 5
#define TASK_LEDS_PRIORITY 5
#define TASK_COMMAND_PRIORITY 5

/* UART Defines*/
#define SLEEP_TIME_MS 100
#define RECEIVE_BUFF_SIZE 10
#define TX_BUFF_SIZE 20
#define RECEIVE_TIMEOUT 100
#define TX_TIMEOUT 100

/*Semaphores definition*/
K_SEM_DEFINE(temp_sem, 1, 1);
K_SEM_DEFINE(led_sem, 1, 1);
K_SEM_DEFINE(button_sem, 1, 1);
K_SEM_DEFINE(command_sem, 0, 1);

/* Create threads stack space*/
K_THREAD_STACK_DEFINE(task_temp_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(task_buttons_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(task_leds_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(task_command_stack,STACK_SIZE);

/* Variable for threads data*/
struct k_thread task_temp_data;
struct k_thread task_buttons_data;
struct k_thread task_leds_data;
struct k_thread task_command_data;

/* Threads ID*/
k_tid_t task_temp_tid;
k_tid_t task_buttons_tid;
k_tid_t task_leds_tid;
k_tid_t task_command_tid;

/* Threads prototypes */
void ButtonsUpdate(void *argA, void *argB, void *argC);
void TemperatureUpdate(void *argA, void *argB, void *argC);
void LedsUpdate(void *argA, void *argB, void *argC);
void Command_Process(void *argA, void *argB, void *argC);

/*Device structs*/
static const struct i2c_dt_spec dev_i2c = I2C_DT_SPEC_GET(I2C0_NODE);
static const struct device *gpio0_dev = DEVICE_DT_GET(GPIO0_NODE);
static const struct device *uart = DEVICE_DT_GET(UART0_NODE);

const uint8_t buttons_pins[] = {11, 12, 24, 25};
const uint8_t leds_pins[] = {13, 14, 15, 16};

const struct uart_config uart_cfg = {
	.baudrate = 115200,
	.parity = UART_CFG_PARITY_NONE,
	.stop_bits = UART_CFG_STOP_BITS_1,
	.data_bits = UART_CFG_DATA_BITS_8,
	.flow_ctrl = UART_CFG_FLOW_CTRL_NONE};

static uint8_t tx_buf[] = {"nRF Connect SDK Fundamentals Course\n\r"
						   "Press 1-3 on your keyboard to toggle LEDS 1-3 on your development kit\n\r"};

/* STEP 10.1.2 - Define the receive buffer */
static uint8_t rx_buf[RECEIVE_BUFF_SIZE] = {0};

int ret;
int output_period = 10; /* ms */
int input_period = 10;

/*Real Time application database*/
struct RTDB RTDB;

static void uart_cb(const struct device *dev, struct uart_event *evt, void *user_data);

/* main */
void main(void)
{
	RTDB.led[0]=0;
	RTDB.led[1]=1;
	/* Check if uart is ready */
	if (!device_is_ready(uart))
	{
		printk("UART: Device is not ready.\n");
		return;
	}
	else
	{
		printk("UART: Device is ready!\n");
	}

	/* Configure UART */
	int err = uart_configure(uart, &uart_cfg);

	if (err == -ENOSYS)
	{
		return;
	}

	/* Define UART Callback function*/
	ret = uart_callback_set(uart, uart_cb, NULL);
	if (ret)
	{
		return;
	}

	/*Start UART transmition*/
	ret = uart_tx(uart, tx_buf, sizeof(tx_buf), SYS_FOREVER_MS);
	if (ret)
	{
		return;
	}
	
	/*Start UART receiver*/
	ret = uart_rx_enable(uart, rx_buf, sizeof rx_buf, RECEIVE_TIMEOUT);
	if (ret)
	{
		return;
	}

	/* Check  I2C device */
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

	task_command_tid = k_thread_create(&task_command_data, task_command_stack,
									K_THREAD_STACK_SIZEOF(task_command_stack), Command_Process,
									NULL, NULL, NULL, TASK_COMMAND_PRIORITY, 0, K_NO_WAIT);
	
	return;
}

void TemperatureUpdate(void *argA, void *argB, void *argC)
{
	/*updating data variable every 10ms*/
	uint8_t data = 0x00;
	int64_t start_time;

	printk("THREAD Temp Started!\n");
	/* infinite cycle */
	while (1)
	{
		start_time = k_uptime_get();

		ret = i2c_read_dt(&dev_i2c, &data, sizeof(data));

		k_sem_take(&temp_sem, K_FOREVER);

		RTDB.temp = data;

		k_sem_give(&temp_sem);

		/* Make thread sleep for the remaining of time before next check */
		if (k_uptime_get() - start_time < input_period)
		{
			/*gets the most updated remaining time to sleep*/
			k_msleep(input_period - (k_uptime_get() - start_time));
		}
		else
		{
			printk("Failed to meet requirement! Temp\n");
		}
	}
}

void ButtonsUpdate(void *argA, void *argB, void *argC)
{
	
	int64_t start_time;
	printk("Button started!\n");
	while (1)
	{
		start_time = k_uptime_get();

		k_sem_take(&button_sem, K_FOREVER);

		RTDB.but[0] = gpio_pin_get(gpio0_dev, buttons_pins[0]);
		RTDB.but[1] = gpio_pin_get(gpio0_dev, buttons_pins[1]);
		RTDB.but[2] = gpio_pin_get(gpio0_dev, buttons_pins[2]);
		RTDB.but[3] = gpio_pin_get(gpio0_dev, buttons_pins[3]);

		k_sem_give(&button_sem);

		/* Make thread sleep for the remaining of time before next check */
		if (k_uptime_get() - start_time < output_period)
		{
			/*gets the most updated remaining time to sleep*/
			k_msleep(input_period - (k_uptime_get() - start_time));
		}
		else
		{
			printk("Failed to meet requirement! Button\n");
		}
	}
}


void LedsUpdate(void *argA, void *argB, void *argC)
{
	int start_time = k_uptime_get();
	printk("LED started!\n");
	while (1)
	{
		start_time = k_uptime_get();

		k_sem_take(&led_sem, K_FOREVER);
		gpio_pin_set(gpio0_dev, leds_pins[0], RTDB.led[0]);
		gpio_pin_set(gpio0_dev, leds_pins[1], RTDB.led[1]);
		gpio_pin_set(gpio0_dev, leds_pins[2], RTDB.led[2]);
		gpio_pin_set(gpio0_dev, leds_pins[3], RTDB.led[3]);
		k_sem_give(&led_sem);

		if (k_uptime_get() - start_time < output_period)
		{
			/*gets the most updated remaining time to sleep*/
			k_msleep(input_period - (k_uptime_get() - start_time));
		}
		else
		{
			printk("Failed to meet requirement! LED\n");
			return;
		}
	}
}

/* Waits for uart to announce the end of the command by raising the flag*/
void Command_Process(void *argA, void *argB, void *argC)
{
	while(1)
	{
		k_sem_take(&command_sem, K_FOREVER);
		cmdProcessor();
	}

}

static void uart_cb(const struct device *dev, struct uart_event *evt, void *user_data)
{
	switch (evt->type) {

	case UART_RX_RDY:
		newCmdChar(evt->data.rx.buf[evt->data.rx.offset]);
		printk("%c",evt->data.rx.buf[evt->data.rx.offset]);
		if(evt->data.rx.buf[evt->data.rx.offset]=='!')
		{
			printk("\n");
			k_sem_give(&command_sem);
		}
		/*
		if(evt->data.rx.len == 1){
			if(evt->data.rx.buf[evt->data.rx.offset] == '1')
				RTDB.led[0]=!RTDB.led[0];
			else if (evt->data.rx.buf[evt->data.rx.offset] == '2')
				RTDB.led[1]=!RTDB.led[1];
			else if (evt->data.rx.buf[evt->data.rx.offset] == '3')
				RTDB.led[2]=!RTDB.led[2];
			else if (evt->data.rx.buf[evt->data.rx.offset] == '4')
				RTDB.led[3]=!RTDB.led[3];						
			}*/

	break;
	case UART_RX_DISABLED:
		uart_rx_enable(dev ,rx_buf,sizeof rx_buf,RECEIVE_TIMEOUT);
		break;
		
	default:
		break;
	}
}