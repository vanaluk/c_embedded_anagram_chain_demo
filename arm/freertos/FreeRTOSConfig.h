/**
 * @file FreeRTOSConfig.h
 * @brief FreeRTOS configuration for LM3S6965 (Cortex-M3)
 *
 * Minimal configuration for running a single task demo.
 */

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/*-----------------------------------------------------------
 * Application specific definitions.
 *----------------------------------------------------------*/

/* Cortex-M3 specific */
#define configCPU_CLOCK_HZ (12000000UL) /* 12 MHz for LM3S6965 */
#define configTICK_RATE_HZ (1000)       /* 1ms tick */
#define configUSE_16_BIT_TICKS 0        /* 32-bit tick counter */

/* Scheduler */
#define configUSE_PREEMPTION 1
#define configUSE_TIME_SLICING 1
#define configUSE_IDLE_HOOK 0
#define configUSE_TICK_HOOK 0
#define configUSE_DAEMON_TASK_STARTUP_HOOK 0

/* Memory */
#define configMINIMAL_STACK_SIZE (128)    /* Words, not bytes */
#define configTOTAL_HEAP_SIZE (32 * 1024) /* 32KB heap */
#define configMAX_TASK_NAME_LEN (16)
#define configSUPPORT_STATIC_ALLOCATION 0
#define configSUPPORT_DYNAMIC_ALLOCATION 1

/* Task priorities */
#define configMAX_PRIORITIES (5)

/* Features to include */
#define configUSE_MUTEXES 0
#define configUSE_RECURSIVE_MUTEXES 0
#define configUSE_COUNTING_SEMAPHORES 0
#define configUSE_QUEUE_SETS 0
#define configUSE_TASK_NOTIFICATIONS 1
#define configUSE_TRACE_FACILITY 0
#define configUSE_STATS_FORMATTING_FUNCTIONS 0

/* Co-routines (not used) */
#define configUSE_CO_ROUTINES 0

/* Software timer (not used) */
#define configUSE_TIMERS 0

/* API functions to include */
#define INCLUDE_vTaskPrioritySet 0
#define INCLUDE_uxTaskPriorityGet 0
#define INCLUDE_vTaskDelete 1
#define INCLUDE_vTaskSuspend 0
#define INCLUDE_vTaskDelayUntil 0
#define INCLUDE_vTaskDelay 1
#define INCLUDE_xTaskGetSchedulerState 0
#define INCLUDE_xTaskGetCurrentTaskHandle 0
#define INCLUDE_uxTaskGetStackHighWaterMark 0
#define INCLUDE_xTaskGetIdleTaskHandle 0

/* Cortex-M specific interrupt priorities */
#ifdef __NVIC_PRIO_BITS
#define configPRIO_BITS __NVIC_PRIO_BITS
#else
#define configPRIO_BITS 3 /* LM3S6965 has 3 priority bits */
#endif

#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY 0x07
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 0x01

#define configKERNEL_INTERRUPT_PRIORITY \
    (configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))
#define configMAX_SYSCALL_INTERRUPT_PRIORITY \
    (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))

/* Assert handler */
#define configASSERT(x)           \
    if ((x) == 0) {               \
        taskDISABLE_INTERRUPTS(); \
        for (;;);                 \
    }

/* Map FreeRTOS interrupt handlers to standard names */
#define vPortSVCHandler SVC_Handler
#define xPortPendSVHandler PendSV_Handler
#define xPortSysTickHandler SysTick_Handler

#endif /* FREERTOS_CONFIG_H */
