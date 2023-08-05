// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include <stdint.h>

#include "sw/device/lib/testing/test_framework/check.h"
#include "sw/device/lib/testing/test_framework/status.h"
#include "sw/ip/pinmux/test/utils/pinmux_testutils.h"
#include "sw/ip/uart/dif/dif_uart.h"
#include "sw/lib/sw/device/arch/device.h"
#include "sw/lib/sw/device/runtime/hart.h"
#include "sw/lib/sw/device/runtime/log.h"
#include "sw/lib/sw/device/runtime/print.h"

#include "hw/top_darjeeling/sw/autogen/top_darjeeling.h"

static dif_uart_t uart0;
static dif_pinmux_t pinmux;

enum {
  kSramStart = TOP_DARJEELING_SRAM_CTRL_MAIN_RAM_BASE_ADDR,
  kSramEnd = TOP_DARJEELING_SRAM_CTRL_MAIN_RAM_BASE_ADDR +
             TOP_DARJEELING_SRAM_CTRL_MAIN_RAM_SIZE_BYTES,
};

void sram_main(void) {
  if (kDeviceType != kDeviceSimDV) {
    // Configure the pinmux.
    CHECK_DIF_OK(dif_pinmux_init(
        mmio_region_from_addr(TOP_DARJEELING_PINMUX_AON_BASE_ADDR), &pinmux));
    pinmux_testutils_init(&pinmux);

    // Initialize UART.
    CHECK_DIF_OK(dif_uart_init(
        mmio_region_from_addr(TOP_DARJEELING_UART0_BASE_ADDR), &uart0));
    CHECK(kUartBaudrate <= UINT32_MAX, "kUartBaudrate must fit in uint32_t");
    CHECK(kClockFreqPeripheralHz <= UINT32_MAX,
          "kClockFreqPeripheralHz must fit in uint32_t");
    CHECK_DIF_OK(dif_uart_configure(
        &uart0, (dif_uart_config_t){
                    .baudrate = (uint32_t)kUartBaudrate,
                    .clk_freq_hz = (uint32_t)kClockFreqPeripheralHz,
                    .parity_enable = kDifToggleDisabled,
                    .parity = kDifUartParityEven,
                    .tx_enable = kDifToggleEnabled,
                    .rx_enable = kDifToggleEnabled,
                }));
    base_uart_stdout(&uart0);
  }

  // Read the program counter and check that we are executing from SRAM.
  uint32_t pc = 0;
  asm("auipc %[pc], 0;" : [pc] "=r"(pc));
  LOG_INFO("PC: %p, SRAM: [%p, %p)", pc, kSramStart, kSramEnd);
  CHECK(pc >= kSramStart && pc < kSramEnd, "PC is outside the main SRAM");
  if (kDeviceType == kDeviceSimDV) {
    test_status_set(kTestStatusPassed);
  }
}

// Reference functions that the debugger may wish to call. This prevents the
// compiler from dropping them as unused functions and has the side benefit of
// preventing their includes from appearing unused.
void debugger_support_functions(void) { (void)icache_invalidate; }
