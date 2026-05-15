/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : TouchGFXHAL.cpp
  ******************************************************************************
  * This file was created by TouchGFX Generator 4.26.1. This file is only
  * generated once! Delete this file from your project and re-generate code
  * using STM32CubeMX or change this file manually to update it.
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

#include <TouchGFXHAL.hpp>

/* USER CODE BEGIN TouchGFXHAL.cpp */
#include <touchgfx/hal/OSWrappers.hpp>
#include <touchgfx/canvas_widget_renderer/CanvasWidgetRenderer.hpp>

/* Canvas Widget Renderer scratch buffer.  Any Circle/Line/Arc widget
 * needs a globally-registered render buffer or its draw() silently
 * returns false and nothing appears.  3 KB is plenty for our two
 * thin-stroked circles. */
static uint8_t cwr_buffer[3072];

extern "C" {
#include "ili9488.h"

/* Synthesised VSYNC for parallel-bus displays (no LTDC).  Called from a
 * ThreadX tick thread at ~60 Hz; drops a token into the framework's
 * vsync queue so the TouchGFX render task wakes to draw a frame. */
void touchgfx_signal_vsync(void)
{
    touchgfx::OSWrappers::signalVSync();
}
}

using namespace touchgfx;

void TouchGFXHAL::initialize()
{
    /* Bring the ILI9488 panel up BEFORE the framework's first render.
     * The order is: PB8 backlight enable -> reset+init the controller ->
     * configure LD2 (PB7) as a "flush happened" indicator -> let the
     * framework finish initialising and draw the first frame. */
    HAL_GPIO_WritePin(LCD_BL_GPIO_Port, LCD_BL_Pin, GPIO_PIN_SET);
    ili9488_init();

    GPIO_InitTypeDef ld = {0};
    __HAL_RCC_GPIOB_CLK_ENABLE();
    ld.Pin   = GPIO_PIN_7;
    ld.Mode  = GPIO_MODE_OUTPUT_PP;
    ld.Pull  = GPIO_NOPULL;
    ld.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &ld);

    /* Wire the Canvas Widget Renderer's scratch buffer so Circle/Line
     * widgets can actually draw. */
    touchgfx::CanvasWidgetRenderer::setupBuffer(cwr_buffer, sizeof(cwr_buffer));

    TouchGFXGeneratedHAL::initialize();
}

/**
 * Gets the frame buffer address used by the TFT controller.
 *
 * @return The address of the frame buffer currently being displayed on the TFT.
 */
uint16_t* TouchGFXHAL::getTFTFrameBuffer() const
{
    // Calling parent implementation of getTFTFrameBuffer().
    //
    // To overwrite the generated implementation, omit the call to the parent function
    // and implement the needed functionality here.

    return TouchGFXGeneratedHAL::getTFTFrameBuffer();
}

/**
 * Sets the frame buffer address used by the TFT controller.
 *
 * @param [in] address New frame buffer address.
 */
void TouchGFXHAL::setTFTFrameBuffer(uint16_t* address)
{
    // Calling parent implementation of setTFTFrameBuffer(uint16_t* address).
    //
    // To overwrite the generated implementation, omit the call to the parent function
    // and implement the needed functionality here.

    TouchGFXGeneratedHAL::setTFTFrameBuffer(address);
}

/**
 * This function is called whenever the framework has performed a partial draw.
 *
 * @param rect The area of the screen that has been drawn, expressed in absolute coordinates.
 *
 * @see flushFrameBuffer().
 */
void TouchGFXHAL::flushFrameBuffer(const touchgfx::Rect& rect)
{
    /* Push the dirty rect from the SRAM framebuffer out to the ILI9488 panel
     * over the FMC 8080 bus.  Assumes TouchGFX is configured for the same
     * orientation as the panel (e.g. 480x320 landscape -- match in Designer).
     * The framebuffer stride is bytes-per-row of the framework's view. */
    /* Diagnostic: LD2 (yellow PB7) toggles per flush so we can see from
     * outside whether the render thread is alive. */
    HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_7);

    const uint16_t* fb = static_cast<uint16_t*>(getTFTFrameBuffer());
    const uint32_t stride_px = lcd().framebufferStride() / 2;

    ili9488_set_window(rect.x, rect.y,
                       rect.x + rect.width  - 1,
                       rect.y + rect.height - 1);

    for (int row = 0; row < rect.height; ++row) {
        const uint16_t* p = fb + (rect.y + row) * stride_px + rect.x;
        ili9488_push_pixels(p, rect.width);
    }

    HAL::flushFrameBuffer(rect);
}

bool TouchGFXHAL::blockCopy(void* RESTRICT dest, const void* RESTRICT src, uint32_t numBytes)
{
    return TouchGFXGeneratedHAL::blockCopy(dest, src, numBytes);
}

/**
 * Configures the interrupts relevant for TouchGFX. This primarily entails setting
 * the interrupt priorities for the DMA and LCD interrupts.
 */
void TouchGFXHAL::configureInterrupts()
{
    // Calling parent implementation of configureInterrupts().
    //
    // To overwrite the generated implementation, omit the call to the parent function
    // and implement the needed functionality here.

    TouchGFXGeneratedHAL::configureInterrupts();
}

/**
 * Used for enabling interrupts set in configureInterrupts()
 */
void TouchGFXHAL::enableInterrupts()
{
    // Calling parent implementation of enableInterrupts().
    //
    // To overwrite the generated implementation, omit the call to the parent function
    // and implement the needed functionality here.

    TouchGFXGeneratedHAL::enableInterrupts();
}

/**
 * Used for disabling interrupts set in configureInterrupts()
 */
void TouchGFXHAL::disableInterrupts()
{
    // Calling parent implementation of disableInterrupts().
    //
    // To overwrite the generated implementation, omit the call to the parent function
    // and implement the needed functionality here.

    TouchGFXGeneratedHAL::disableInterrupts();
}

/**
 * Configure the LCD controller to fire interrupts at VSYNC. Called automatically
 * once TouchGFX initialization has completed.
 */
void TouchGFXHAL::enableLCDControllerInterrupt()
{
    // Calling parent implementation of enableLCDControllerInterrupt().
    //
    // To overwrite the generated implementation, omit the call to the parent function
    // and implement the needed functionality here.

    TouchGFXGeneratedHAL::enableLCDControllerInterrupt();
}

bool TouchGFXHAL::beginFrame()
{
    return TouchGFXGeneratedHAL::beginFrame();
}

void TouchGFXHAL::endFrame()
{
    TouchGFXGeneratedHAL::endFrame();
}

/* USER CODE END TouchGFXHAL.cpp */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
