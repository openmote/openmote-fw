/**
 * @file       main.cpp
 * @author     Pere Tuset-Peiro (peretuset@openmote.com)
 * @version    v0.1
 * @date       May, 2016
 * @brief
 *
 * @copyright  Copyright 2016, OpenMote Technologies, S.L.
 *             This file is licensed under the GNU General Public License v2.
 */

/*================================ include ==================================*/

#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

#include "board.h"
#include "platform_types.h"

#include "Scheduler.h"
#include "Semaphore.h"
#include "Task.h"

/*================================ define ===================================*/

#define GREEN_LED_TASK_PRIORITY         	( tskIDLE_PRIORITY + 0 )
#define RADIO_TASK_PRIORITY              	( tskIDLE_PRIORITY + 2 )
#define TEMPERATURE_TASK_PRIORITY       	( tskIDLE_PRIORITY + 1 )

#define UART_BAUDRATE                       ( 115200 )
#define SPI_BAUDRATE                        ( 8000000 )

/*================================ typedef ==================================*/

/*=============================== prototypes ================================*/

static void prvGreenLedTask(void *pvParameters);
static void prvTemperatureTask(void *pvParameters);
static void prvRadioTask(void *pvParameters);

static void rf09Callback(void);
static void rf24Callback(void);

/*=============================== variables =================================*/

static PlainCallback rf09Callback_(rf09Callback);
static PlainCallback rf24Callback_(rf24Callback);

static uint8_t uart_buffer[32];
static uint8_t uart_len;

/*================================= public ==================================*/

int main(void) {
    // Initialize the board
    board.init();

    // Enable the UART interface
    uart.enable(UART_BAUDRATE);

    // Enable the SPI interface
    spi.enable(SPI_BAUDRATE);

    // Enable the I2C interface
    i2c.enable();

    // Enable general interrupts
    board.enableInterrupts();

    // Create the FreeRTOS tasks
    xTaskCreate(prvGreenLedTask, (const char *) "GreenLed", 128, NULL, GREEN_LED_TASK_PRIORITY, NULL);
    xTaskCreate(prvTemperatureTask, (const char *) "Temperature", 128, NULL, TEMPERATURE_TASK_PRIORITY, NULL);
    xTaskCreate(prvRadioTask, (const char *) "Radio", 128, NULL, RADIO_TASK_PRIORITY, NULL);

    // Start the scheduler
    Scheduler::run();
}

/*================================ private ==================================*/

static void prvTemperatureTask(void *pvParameters)
{
    uint16_t temperature;
    uint16_t humidity;

    if (si7006.isPresent() == true)
    {
        si7006.enable();

        while (true)
        {
            led_orange.on();

            si7006.readTemperature();
            temperature = si7006.getTemperatureRaw();

            si7006.readHumidity();
            humidity = si7006.getHumidityRaw();

            uart_buffer[0] = (uint8_t)((temperature & 0x00FF) >> 0);
            uart_buffer[1] = (uint8_t)((temperature & 0xFF00) >> 8);
            uart_buffer[2] = (uint8_t)((humidity & 0x00FF) >> 0);
            uart_buffer[3] = (uint8_t)((humidity & 0xFF00) >> 8);
            uart_len = 4;

            uart.writeByte(uart_buffer, uart_len);

            led_orange.off();

            Task::delay(1000);
        }
    }
}

static void prvRadioTask(void *pvParameters)
{
	bool status; 

    // Set radio callbacks
    at86rf215.setCallbacks(&rf09Callback_, &rf24Callback_);
    at86rf215.enableInterrupts();

    // Forever
    while (true)
    {
        // Turn AT86RF215 radio on
        at86rf215.enable();

        status = at86rf215.check();
        if (status) {
        	led_yellow.on();
        } else {
        	led_yellow.off();
        }

        // Turn AT86RF215 radio off
        at86rf215.sleep();

        Task::delay(50);

        led_yellow.off();

        Task::delay(950);
    }
}

static void prvGreenLedTask(void *pvParameters)
{
    // Forever
    while (true)
    {
        // Turn off green LED for 1000 ms
        led_green.off();
        Task::delay(1000);

        // Turn on green LED for 1000 ms
        led_green.on();
        Task::delay(1000);
    }
}
/*================================ private ==================================*/

static void rf09Callback(void)
{
    led_red.toggle();
}

static void rf24Callback(void)
{
}