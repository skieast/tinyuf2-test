/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Ha Thach (tinyusb.org) for Adafruit Industries
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
 */
#pragma once

//--------------------------------------------------------------------+
// Button
//--------------------------------------------------------------------+

// Enter UF2 mode if GPIO is pressed while 2nd stage bootloader indicator
// is on e.g RGB = Purple. If it is GPIO0, user should not hold this while
// reset since that will instead run the 1st stage ROM bootloader
#define PIN_BUTTON_UF2       0

//--------------------------------------------------------------------+
// TFT Display ST7789
//--------------------------------------------------------------------+
#undef  CONFIG_LCD_TYPE_AUTO
#define CONFIG_LCD_TYPE_ST7789V

#define PIN_DISPLAY_MISO    -1              //No MISO connected to display.
#define PIN_DISPLAY_MOSI    35
#define PIN_DISPLAY_SCK     36
#define PIN_DISPLAY_CS      34
#define PIN_DISPLAY_DC      37
#define PIN_DISPLAY_RST     38
#define PIN_DISPLAY_BL      33
#define PIN_POWER           14

#define DISPLAY_BL_STATE        1  // GPIO state to enable back light
#define DISPLAY_WIDTH           240
#define DISPLAY_HEIGHT          135
#define DISPLAY_MADCTL          0x36     //Display Memory Access Control ST7789
#define DISPLAY_ROTATION        1
//--------------------------------------------------------------------+
// USB UF2
//--------------------------------------------------------------------+

#define USB_VID           0x303A            // Espressif VID
#define USB_PID           0x0002            // Default PID (not used for any production boards as this will conflict)
#define USB_MANUFACTURER  "Espressif"
#define USB_PRODUCT       "TTGO_T8_S2_ST7789"

#define UF2_PRODUCT_NAME  USB_MANUFACTURER " " USB_PRODUCT
#define UF2_BOARD_ID      "LilyGO-TTGO-T8-S2-ST7789"
#define UF2_VOLUME_LABEL  "TTGOS2BOOT"
#define UF2_INDEX_URL     "http://www.lilygo.cn/prod_view.aspx?TypeId=50033&Id=1321&FId=t3:50033:3"

