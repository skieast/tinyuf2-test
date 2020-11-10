/* 
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "board_api.h"
#include "uf2.h"
#include "tusb.h"

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+
//#define USE_DFU_BUTTON    1

// Enter DFU magic
#define DBL_TAP_MAGIC             0xf01669ef

// Skip double tap delay detction
#define DBL_TAP_MAGIC_QUICK_BOOT  0xf02669ef

// timeout for double tap detection
#define DBL_TAP_DELAY             500

uint8_t const RGB_USB_UNMOUNTED[] = { 0xff, 0x00, 0x00 }; // Red
uint8_t const RGB_USB_MOUNTED[]   = { 0x00, 0xff, 0x00 }; // Green
uint8_t const RGB_WRITING[]       = { 0xcc, 0x66, 0x00 };
uint8_t const RGB_DOUBLE_TAP[]    = { 0x80, 0x00, 0xff }; // Purple
uint8_t const RGB_UNKNOWN[]       = { 0x00, 0x00, 0x88 }; // for debug
uint8_t const RGB_OFF[]           = { 0x00, 0x00, 0x00 };

//--------------------------------------------------------------------+
//
//--------------------------------------------------------------------+

// defined by linker script
extern uint32_t _board_dfu_dbl_tap[];
static volatile uint32_t _timer_count = 0;

// return true if start DFU mode, else App mode
static bool check_dfu_mode(void)
{
  // Check if app is valid
  if ( !board_app_valid() ) return true;

#if USE_DFU_DOUBLE_TAP
//  TU_LOG1_HEX(_board_dfu_dbl_tap);
//  TU_LOG1_HEX(_board_dfu_dbl_tap[0]);

  // App want to reboot quickly
  if (_board_dfu_dbl_tap[0] == DBL_TAP_MAGIC_QUICK_BOOT)
  {
    _board_dfu_dbl_tap[0] = 0;
    return false;
  }

  if (_board_dfu_dbl_tap[0] == DBL_TAP_MAGIC)
  {
    // Double tap occurred
    _board_dfu_dbl_tap[0] = 0;
    return true;
  }

  // Register our first reset for double reset detection
  _board_dfu_dbl_tap[0] = DBL_TAP_MAGIC;
  _timer_count = 0;

  // Turn on LED/RGB for visual indicator
  board_led_write(0xff);
  board_rgb_write(RGB_DOUBLE_TAP);

  // delay a fraction of second if Reset pin is tap during this delay --> we will enter dfu
  board_timer_start(10);
  while(_timer_count < DBL_TAP_DELAY/10) {}
  board_timer_stop();

  // Turn off indicator
  board_led_write(0x00);

  _board_dfu_dbl_tap[0] = 0;
#endif

  return false;
}

int main(void)
{
  board_init();

  // if not DFU mode, jump to App
  if ( !check_dfu_mode() )
  {
    board_app_jump();
    while(1) {}
  }

  board_dfu_init();
  board_flash_init();
  uf2_init();

  tusb_init();

  indicator_set(STATE_BOOTLOADER_STARTED);

#if USE_SCREEN
  screen_init();
  screen_draw_drag();
#endif

#if CFG_TUSB_OS == OPT_OS_NONE
  while(1)
  {
    tud_task();
  }
#endif
}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is plugged and configured
void tud_mount_cb(void)
{
  indicator_set(STATE_USB_PLUGGED);
}

// Invoked when device is unplugged
void tud_umount_cb(void)
{
  indicator_set(STATE_USB_UNPLUGGED);
}

//--------------------------------------------------------------------+
// USB HID
//--------------------------------------------------------------------+

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
{
  // TODO not Implemented
  (void) report_id;
  (void) report_type;
  (void) buffer;
  (void) reqlen;

  return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
  // This example doesn't use multiple report and report ID
  (void) report_id;
  (void) report_type;
  (void) buffer;
  (void) bufsize;
}

//--------------------------------------------------------------------+
// Indicator
//--------------------------------------------------------------------+

static uint32_t _indicator_state = STATE_BOOTLOADER_STARTED;

void indicator_set(uint32_t state)
{
  _indicator_state = state;
  switch(state)
  {
    case STATE_BOOTLOADER_STARTED:
    case STATE_USB_UNPLUGGED:
      board_rgb_write(RGB_USB_UNMOUNTED);
    break;

    case STATE_USB_PLUGGED:
      board_rgb_write(RGB_USB_MOUNTED);
    break;

    case STATE_WRITING_STARTED:
      board_timer_start(25);
    break;

    case STATE_WRITING_FINISHED:
      board_timer_stop();
      board_rgb_write(RGB_WRITING);
    break;

    default:
      board_rgb_write(RGB_UNKNOWN);
    break;
  }
}

void board_timer_handler(void)
{
  _timer_count++;

  if ( _indicator_state == STATE_WRITING_STARTED )
  {
    // fast blink LED if available
    board_led_write(_timer_count & 0x01);

    // blink RGB if available
    board_rgb_write((_timer_count & 0x01) ? RGB_WRITING : RGB_OFF);
  }
}

//--------------------------------------------------------------------+
// Logger newlib retarget
//--------------------------------------------------------------------+

// Enable only with LOG is enabled (Note: ESP32-S2 has built-in support already)
#if CFG_TUSB_DEBUG && (CFG_TUSB_MCU != OPT_MCU_ESP32S2)

#if defined(LOGGER_RTT)
#include "SEGGER_RTT.h"
#endif

TU_ATTR_USED int _write (int fhdl, const void *buf, size_t count)
{
  (void) fhdl;

#if defined(LOGGER_RTT)
  SEGGER_RTT_Write(0, (char*) buf, (int) count);
  return count;
#else
  return board_uart_write(buf, count);
#endif
}

#endif
