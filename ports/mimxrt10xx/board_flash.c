/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Ha Thach for Adafruit Industries
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

#include "board_api.h"
#include "fsl_flexspi.h"
#include "fsl_cache.h"

#include "tusb.h" // for logging

#define FLASH_CACHE_SIZE          4096
#define FLASH_CACHE_INVALID_ADDR  0xffffffff

// FLASH
#define NO_CACHE        0xffffffff

#define SECTOR_SIZE 0x1000 /* 4K */
#define FLASH_PAGE_SIZE 256
#define FILESYSTEM_BLOCK_SIZE 256

static uint32_t _flash_page_addr = NO_CACHE;
static uint8_t  _flash_cache[SECTOR_SIZE] __attribute__((aligned(4)));

extern void     flexspi_nor_flash_init(FLEXSPI_Type *base);
extern status_t flexspi_nor_flash_erase_sector(FLEXSPI_Type *base, uint32_t address);
extern status_t flexspi_nor_flash_page_program(FLEXSPI_Type *base, uint32_t dstAddr, uint32_t *src, uint32_t size);
extern status_t flexspi_nor_get_vendor_id(FLEXSPI_Type *base, uint8_t *vendorId);
extern status_t flexspi_nor_enable_quad_mode(FLEXSPI_Type *base);

void board_flash_init(void)
{
  flexspi_nor_flash_init(FLEXSPI);
}

uint32_t board_flash_size(void)
{
  return 1024*1024;
}

void board_flash_read(uint32_t addr, void* buffer, uint32_t len)
{
  // Must write out anything in cache before trying to read.
  //board_flash_flush();

  memcpy(buffer, (uint8_t*) addr, len);
}

void board_flash_flush(void)
{
  status_t status;

  if ( _flash_page_addr == NO_CACHE ) return;

  TU_LOG1("Erase and Write at address = 0x%08lX\r\n",_flash_page_addr);

  // Skip if data is the same
  if ( memcmp(_flash_cache, (void*) _flash_page_addr, SECTOR_SIZE) != 0 )
  {
    uint32_t const sector_addr = (_flash_page_addr - FlexSPI_AMBA_BASE);

    __disable_irq();
    status = flexspi_nor_flash_erase_sector(FLEXSPI, sector_addr);
    __enable_irq();

    SCB_InvalidateDCache_by_Addr((uint32_t *)sector_addr, SECTOR_SIZE);

    if ( status != kStatus_Success )
    {
      TU_LOG1("Erase failed: status = %ld!\r\n", status);
      return;
    }

    for ( int i = 0; i < SECTOR_SIZE / FLASH_PAGE_SIZE; ++i )
    {
      uint32_t const page_addr = sector_addr + i * FLASH_PAGE_SIZE;
      void* page_data =  _flash_cache + i * FLASH_PAGE_SIZE;

      __disable_irq();
      status = flexspi_nor_flash_page_program(FLEXSPI, page_addr, (uint32_t*) page_data, FLASH_PAGE_SIZE);
      __enable_irq();

      if ( status != kStatus_Success )
      {
        TU_LOG1("Page program failed: status = %ld!\r\n", status);
        return;
      }
    }

    SCB_InvalidateDCache_by_Addr((uint32_t *)sector_addr, SECTOR_SIZE);
  }

  _flash_page_addr = NO_CACHE;
}

void board_flash_write (uint32_t addr, void const *src, uint32_t len)
{
  uint32_t const page_addr = addr & ~(SECTOR_SIZE - 1);

  if ( page_addr != _flash_page_addr )
  {
    // Write out anything in cache before overwriting it.
    board_flash_flush();

    _flash_page_addr = page_addr;

    // Copy the current contents of the entire page into the cache.
    memcpy(_flash_cache, (void*) page_addr, SECTOR_SIZE);
  }

  // Overwrite part or all of the page cache with the src data.
  memcpy(_flash_cache + (addr & (SECTOR_SIZE - 1)), src, len);
}
