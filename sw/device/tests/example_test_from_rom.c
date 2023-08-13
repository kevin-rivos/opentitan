// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include "sw/device/lib/testing/pinmux_testutils.h"
#include "sw/device/lib/testing/test_framework/check.h"
#include "sw/device/lib/testing/test_framework/status.h"
#include "sw/ip/pinmux/dif/dif_pinmux.h"
#include "sw/ip/uart/dif/dif_uart.h"
#include "sw/lib/sw/device/arch/device.h"
#include "sw/lib/sw/device/base/mmio.h"
#include "sw/lib/sw/device/runtime/hart.h"
#include "sw/lib/sw/device/runtime/log.h"
#include "sw/lib/sw/device/runtime/print.h"

#include "hw/top_darjeeling/sw/autogen/top_darjeeling.h"  // Generated.

static dif_pinmux_t pinmux;
static dif_uart_t uart0;

bool rom_test_main(void) {
  // We need to set the test status as "in test" to indicate to the test code
  // has been reached, even though this test is also in the "boot ROM".
  test_status_set(kTestStatusInTest);
  CHECK_DIF_OK(dif_pinmux_init(
      mmio_region_from_addr(TOP_DARJEELING_PINMUX_AON_BASE_ADDR), &pinmux));
  pinmux_testutils_init(&pinmux);

  // We need to initialize the UART regardless if we LOG any messages, since
  // Verilator and FPGA platforms use the UART to communicate the test results.
  if (kDeviceType != kDeviceSimDV) {
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

  /**
   * Place test code here.
   */

  /**
   * Return true if the test succeeds. Return false if it should fail.
   */
  return true;
}
