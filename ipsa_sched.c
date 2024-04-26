/*
 * FreeRTOS V202107.00
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 */

#include <stdio.h>
#include <pthread.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"

/* Local includes. */
#include "console.h"
/*Size of the list for a task*/
#define SIZE 50

/* Priorities at which the tasks are created. */
#define mainQUEUE_RECEIVE_TASK_PRIORITY     ( tskIDLE_PRIORITY + 5 )

#define mainQUEUE_SEND_TASK1_PRIORITY       ( tskIDLE_PRIORITY + 1 )
#define mainQUEUE_SEND_TASK2_PRIORITY       ( tskIDLE_PRIORITY + 2 )
#define mainQUEUE_SEND_TASK3_PRIORITY       ( tskIDLE_PRIORITY + 3 )
#define mainQUEUE_SEND_TASK4_PRIORITY       ( tskIDLE_PRIORITY + 4 )


/* The rate at which data is sent to the queue.  The times are converted from
 * milliseconds to ticks using the pdMS_TO_TICKS() macro. */
#define mainTASK1_SEND_FREQUENCY_MS         pdMS_TO_TICKS( 350UL )
#define mainTASK2_SEND_FREQUENCY_MS         pdMS_TO_TICKS( 350UL )
#define mainTASK3_SEND_FREQUENCY_MS         pdMS_TO_TICKS( 600UL )
#define mainTASK4_SEND_FREQUENCY_MS         pdMS_TO_TICKS( 850UL )
#define mainTIMER_SEND_FREQUENCY_MS         pdMS_TO_TICKS( 2400UL )

/* The number of items the queue can hold at once. */
#define mainQUEUE_LENGTH                   ( 5 )

/* The values sent to the queue receive task from the queue send task and the
 * queue send software timer respectively. */
#define mainVALUE_SENT_FROM_TASK1           ( 100UL )
#define mainVALUE_SENT_FROM_TASK2           ( 200UL )
#define mainVALUE_SENT_FROM_TASK3           ( 300UL )
#define mainVALUE_SENT_FROM_TASK4           ( 400UL )
#define mainVALUE_SENT_FROM_TIMER           ( 10UL )

/*-----------------------------------------------------------*/

/*
 * The tasks as described in the comments at the top of this file.
 */
static void prvQueueReceiveTask( void * pvParameters );

static void prvQueueSendTask1( void * pvParameters );
static void prvQueueSendTask2( void * pvParameters );
static void prvQueueSendTask3( void * pvParameters );
static void prvQueueSendTask4( void * pvParameters );

/*
 * The callback function executed when the software timer expires.
 */
static void prvQueueSendTimerCallback( TimerHandle_t xTimerHandle );
/*-----------------------------------------------------------*/

/* The queue used by both tasks. */
static QueueHandle_t xQueue = NULL;

/* A software timer that is started from the tick hook. */
static TimerHandle_t xTimer = NULL;

/*-----------------------------------------------------------*/
void ipsa_sched( void )
{
    const TickType_t xTimerPeriod = mainTIMER_SEND_FREQUENCY_MS;

    /* Create the queue. */
    xQueue = xQueueCreate( mainQUEUE_LENGTH, sizeof( uint32_t ) );

    if( xQueue != NULL )
    {
        /* Start the two tasks as described in the comments at the top of this file. */
        xTaskCreate( prvQueueReceiveTask,"Rx", configMINIMAL_STACK_SIZE, NULL, mainQUEUE_RECEIVE_TASK_PRIORITY, NULL );
        xTaskCreate( prvQueueSendTask1, "T1X", configMINIMAL_STACK_SIZE, NULL, mainQUEUE_SEND_TASK1_PRIORITY, NULL );
        xTaskCreate( prvQueueSendTask2, "T2X", configMINIMAL_STACK_SIZE, NULL, mainQUEUE_SEND_TASK2_PRIORITY, NULL );
        xTaskCreate( prvQueueSendTask3, "T3X", configMINIMAL_STACK_SIZE, NULL, mainQUEUE_SEND_TASK3_PRIORITY, NULL );
        xTaskCreate( prvQueueSendTask4, "T4X", configMINIMAL_STACK_SIZE, NULL, mainQUEUE_SEND_TASK4_PRIORITY, NULL );
        /* Create the software timer, but don't start it yet. */
        xTimer = xTimerCreate( "Timer", xTimerPeriod, pdTRUE, NULL, prvQueueSendTimerCallback ); 

        if( xTimer != NULL )
        {
            xTimerStart( xTimer, 0 );
        }
        vTaskStartScheduler();
    }
    for( ; ; )
    {
    }
}

/*------Timer clock-------------------------------------*/

static void prvQueueSendTimerCallback( TimerHandle_t xTimerHandle )
{
    const uint32_t ulValueToSend = mainVALUE_SENT_FROM_TIMER;
    ( void ) xTimerHandle;
    xQueueSend( xQueue, &ulValueToSend, 0U );
}
/*-----Clock for every task------------------------------------------*/

/*TASK1 - Working*/
static void prvQueueSendTask1( void * pvParameters )
{
    TickType_t xNextWakeTime;
    const TickType_t xBlockTime = mainTASK1_SEND_FREQUENCY_MS;
    const uint32_t ulValueToSend = mainVALUE_SENT_FROM_TASK1;
    ( void ) pvParameters;
    xNextWakeTime = xTaskGetTickCount();
    for( ; ; )
    {
        vTaskDelayUntil( &xNextWakeTime, xBlockTime );
        xQueueSend( xQueue, &ulValueToSend, 0U );
    }
}
/*TASK2 - Conversion*/
static void prvQueueSendTask2( void * pvParameters )
{
    TickType_t xNextWakeTime;
    const TickType_t xBlockTime = mainTASK2_SEND_FREQUENCY_MS;
    const uint32_t ulValueToSend = mainVALUE_SENT_FROM_TASK2;
    ( void ) pvParameters;
    xNextWakeTime = xTaskGetTickCount();
    for( ; ; )
    {
        vTaskDelayUntil( &xNextWakeTime, xBlockTime );
        xQueueSend( xQueue, &ulValueToSend, 0U );
    }
}

/*TASK3 - Multiplication*/
static void prvQueueSendTask3( void * pvParameters )
{
    TickType_t xNextWakeTime;
    const TickType_t xBlockTime = mainTASK3_SEND_FREQUENCY_MS;
    const uint32_t ulValueToSend = mainVALUE_SENT_FROM_TASK3;
    ( void ) pvParameters;
    xNextWakeTime = xTaskGetTickCount();

    for( ; ; )
    {
        vTaskDelayUntil( &xNextWakeTime, xBlockTime );
        xQueueSend( xQueue, &ulValueToSend, 0U );
    }
}

/*TASK4 - Table search*/
static void prvQueueSendTask4( void * pvParameters )
{
    TickType_t xNextWakeTime;
    const TickType_t xBlockTime = mainTASK4_SEND_FREQUENCY_MS;
    const uint32_t ulValueToSend = mainVALUE_SENT_FROM_TASK4;
    ( void ) pvParameters;
    xNextWakeTime = xTaskGetTickCount();
    for( ; ; )
    {
        vTaskDelayUntil( &xNextWakeTime, xBlockTime );
        xQueueSend( xQueue, &ulValueToSend, 0U );
    }
}

/*-----------------------------------------------------------*/

static void prvQueueReceiveTask( void * pvParameters )
{
    uint32_t ulReceivedValue;
    ( void ) pvParameters;
    for( ; ; )
    {
        /* Blocked state. */
        xQueueReceive( xQueue, &ulReceivedValue, portMAX_DELAY );
		/* TASK 1 - Working*/
        if( ulReceivedValue == mainVALUE_SENT_FROM_TASK1)  
        {
            console_print( "Working\n" );
        }
		/* If Val 2 is recieved in queue, TASK 2 - Convert will start*/
        else if( ulReceivedValue == mainVALUE_SENT_FROM_TASK2)
        {
            float F = 86;
			float C;
			C = (F - 32) / 1.8 ;
			printf("Task 2 : The Conversion of %f (Fahrenheit) in (celsius) is %f",F,C);
			printf("\n");
		/* If val 3, TASK 3 - Multiply*/
        }else if( ulReceivedValue == mainVALUE_SENT_FROM_TASK3)
        {   
            long int A=642837192;
			long int B=313193871;
			long int result;
            result = A * B;
			printf("Task 3 : Multiplication of %ld with %ld =  %ld",A,B,result);
			printf("\n");
         
		/* If Val 4, TASK 4 - Table Search*/
        }else if( ulReceivedValue == mainVALUE_SENT_FROM_TASK4)
        {
           int tableau[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51};
		   int n = sizeof(tableau) / sizeof(tableau[0]);
		   int x=37;
		   int low=0;
		   int high=n-1;
		   int check=-1;
		   int m,milieu; 
		   
		   while (low <= high) 
		{
					int milieu = low + (high - low) / 2;
					if (tableau[milieu] == x)
						m = milieu;
						check =0;

					if (tableau[milieu] < x)
						low = milieu + 1;

					else
						high = milieu - 1;
				}
				
				
			if ( check == -1)
				printf("Task 4 : La valeur n'est pas dans le tableau \n");
				
			else
				printf("Task 4 : la valeur est à l'occurence  %d \n", m);
			}
		 
		
		
		
		else if( ulReceivedValue == mainVALUE_SENT_FROM_TIMER)
        {
            console_print( "HyperPeriod reached \n" );
        }
        else 
        {
            console_print( "Unexpected message\n" );
        }
    }
}
/*-----------------------------------------------------------*/
