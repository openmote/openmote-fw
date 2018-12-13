/**
 * @file       main.cpp
 * @author     Pere Tuset-Peiro (peretuset@openmote.com)
 * @version    v0.1
 * @date       May, 2015
 * @brief
 *
 * @copyright  Copyright 2015, OpenMote Technologies, S.L.
 *             This file is licensed under the GNU General Public License v2.
 */

/*================================ include ==================================*/

#include "FreeRTOS.h"
#include "task.h"

#include "board.h"

#include "Scheduler.h"
#include "Task.h"

/*================================ define ===================================*/

#define GREEN_LED_TASK_PRIORITY             ( tskIDLE_PRIORITY + 1 )
#define UART_TASK_PRIORITY                  ( tskIDLE_PRIORITY + 0 )

#define UART_BAUDRATE                       ( 115200 )

/*================================ typedef ==================================*/

/*=============================== prototypes ================================*/

static void prvGreenLedTask(void *pvParameters);
static void prvUartTask(void *pvParameters);

/*=============================== variables =================================*/

uint8_t uart_buffer[] = {'O','p','e','n','M','o','t','e','-','C','C','2','5','3','8','\r','\n'};
uint8_t* uart_ptr = uart_buffer;
uint8_t uart_len  = sizeof(uart_buffer);

/*================================= public ==================================*/

int main (void)
{
    // Initialize board
    board.init();

    // Enable the UART peripheral and the serial driver
    uart.enable(UART_BAUDRATE);

    // Create two FreeRTOS tasks
    xTaskCreate(prvGreenLedTask, (const char *) "Green", 128, NULL, GREEN_LED_TASK_PRIORITY, NULL);
    xTaskCreate(prvUartTask, (const char *) "Uart", 128, NULL, UART_TASK_PRIORITY, NULL);

    // Start the scheduler
    Scheduler::run();
}

/*=============================== protected =================================*/

/*================================ private ==================================*/

static void prvUartTask(void *pvParameters)
{
    // Forever
    while(true)
    {
        uart_ptr = uart_buffer;
        uart_len = sizeof(uart_buffer);

        // Turn on red LED
        led_red.on();

        // Print buffer via UART
        uart.writeByte(uart_ptr, uart_len);

        // Turn off red LED
        led_red.off();

        // Delay for 250 ms
        Task::delay(250);
    }
}

static void prvGreenLedTask(void *pvParameters)
{
    // Forever
    while(true)
    {
        // Turn off green LED for 950 ms
        led_green.off();
        Task::delay(950);

        // Turn on green LED for 50 ms
        led_green.on();
        Task::delay(50);
    }
}