/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_threadx.c
  * @author  MCD Application Team
  * @brief   ThreadX applicative file
  ******************************************************************************
    * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "app_threadx.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "app_touchgfx.h"

extern void touchgfx_signal_vsync(void);
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
#define VSYNC_STACK_SIZE    1024
#define VSYNC_TICK_TICKS    (TX_TIMER_TICKS_PER_SECOND / 60)    /* ~16 ms */
#define SENSOR_STACK_SIZE   1024
#define SENSOR_TICK_TICKS   (TX_TIMER_TICKS_PER_SECOND / 30)    /* ~33 ms = 30 Hz */
#define SENSOR_Q_DEPTH      16

static TX_THREAD VsyncThread;
static TX_THREAD SensorThread;

/* Global queue of simulated mV readings.  Each slot is one ULONG.
 * Model::tick() drains this on every VSYNC. */
TX_QUEUE sensor_q;
static ULONG sensor_q_storage[SENSOR_Q_DEPTH];

static VOID Vsync_Task(ULONG arg)
{
    (void)arg;
    /* 60 Hz VSYNC source for TouchGFX -- the framework's render thread
     * is waitForVSync-blocked on the framework's internal queue and
     * wakes each time we drop a token in via signalVSync(). */
    for (;;) {
        touchgfx_signal_vsync();
        tx_thread_sleep(VSYNC_TICK_TICKS);
    }
}

static VOID Sensor_Task(ULONG arg)
{
    (void)arg;
    /* Simulated 0..3300 mV sensor.  Triangle ramp 0 -> 3300 -> 0 with
     * a small jitter so the waveform looks alive.  Push one sample
     * per ~33 ms (30 Hz) into the queue; the View redraws at the next
     * VSYNC and the user sees a slow sweep across the strip. */
    ULONG mv     = 0;
    LONG  dir    = 60;
    ULONG seed   = 0x1234ABCDu;
    for (;;) {
        seed = seed * 1664525u + 1013904223u;
        const LONG jitter = (LONG)((seed >> 24) & 0x7F) - 64;   /* +/- 64 */

        LONG next = (LONG)mv + dir + (jitter / 8);
        if (next >= 3300) { next = 3300; dir = -60; }
        if (next <= 0)    { next = 0;    dir = +60; }
        mv = (ULONG)next;

        (void)tx_queue_send(&sensor_q, &mv, TX_NO_WAIT);
        tx_thread_sleep(SENSOR_TICK_TICKS);
    }
}
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/**
  * @brief  Application ThreadX Initialization.
  * @param memory_ptr: memory pointer
  * @retval int
  */
UINT App_ThreadX_Init(VOID *memory_ptr)
{
  UINT ret = TX_SUCCESS;
  /* USER CODE BEGIN App_ThreadX_MEM_POOL */
  TX_BYTE_POOL *pool = (TX_BYTE_POOL*)memory_ptr;
  /* USER CODE END App_ThreadX_MEM_POOL */
  /* USER CODE BEGIN App_ThreadX_Init */
  /* Spawn the TouchGFX render thread (generated in app_touchgfx.c). */
  ret = MX_TouchGFX_Init(memory_ptr);
  if (ret != TX_SUCCESS) {
    return ret;
  }

  /* Spawn the 60 Hz VSYNC tick thread.  Priority 4 (one higher than the
   * TouchGFX thread at 5) so the tick wakes promptly. */
  CHAR *vsync_stack = NULL;
  if (tx_byte_allocate(pool, (VOID**)&vsync_stack,
                       VSYNC_STACK_SIZE, TX_NO_WAIT) != TX_SUCCESS) {
    return TX_POOL_ERROR;
  }
  if (tx_thread_create(&VsyncThread, (CHAR*)"VSYNC", Vsync_Task, 0,
                       vsync_stack, VSYNC_STACK_SIZE,
                       4, 4, TX_NO_TIME_SLICE, TX_AUTO_START) != TX_SUCCESS) {
    return TX_THREAD_ERROR;
  }

  /* Create the sensor queue + producer thread (S16.3 MVP demo). */
  if (tx_queue_create(&sensor_q, (CHAR*)"sensor_q",
                      TX_1_ULONG,
                      sensor_q_storage,
                      sizeof(sensor_q_storage)) != TX_SUCCESS) {
    return TX_QUEUE_ERROR;
  }

  CHAR *sensor_stack = NULL;
  if (tx_byte_allocate(pool, (VOID**)&sensor_stack,
                       SENSOR_STACK_SIZE, TX_NO_WAIT) != TX_SUCCESS) {
    return TX_POOL_ERROR;
  }
  if (tx_thread_create(&SensorThread, (CHAR*)"SENSOR", Sensor_Task, 0,
                       sensor_stack, SENSOR_STACK_SIZE,
                       6, 6, TX_NO_TIME_SLICE, TX_AUTO_START) != TX_SUCCESS) {
    return TX_THREAD_ERROR;
  }
  /* USER CODE END App_ThreadX_Init */

  return ret;
}

  /**
  * @brief  Function that implements the kernel's initialization.
  * @param  None
  * @retval None
  */
void MX_ThreadX_Init(void)
{
  /* USER CODE BEGIN Before_Kernel_Start */

  /* USER CODE END Before_Kernel_Start */

  tx_kernel_enter();

  /* USER CODE BEGIN Kernel_Start_Error */

  /* USER CODE END Kernel_Start_Error */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
