#include <stdio.h>

#include "pico/stdlib.h"
#include "tusb.h"

#include "PicoSWIO.h"
#include "RVDebug.h"
#include "WCHFlash.h"
#include "SoftBreak.h"
#include "Console.h"
#include "GDBServer.h"  
#include "debug_defines.h"

const int PIN_SWIO = 28;
const int PIN_UART_TX = 0;
const int PIN_UART_RX = 1;
const int ch32v003_flash_size = 16*1024;

//------------------------------------------------------------------------------

int main() {
  // Enable non-USB serial port on gpio 0/1 for meta-debug output :D
  stdio_uart_init_full(uart0, 1000000, PIN_UART_TX, PIN_UART_RX);

  printf("\n\n\n");
  printf("//==============================================================================\n");
  printf("// PicoDebug 0.0.2\n");

  printf("// Starting PicoSWIO\n");
  PicoSWIO* swio = new PicoSWIO();
  swio->reset(PIN_SWIO);

  printf("// Starting RVDebug\n");
  RVDebug* rvd = new RVDebug(swio);
  rvd->reset();

  printf("// Starting WCHFlash\n");
  WCHFlash* flash = new WCHFlash(rvd, ch32v003_flash_size);
  flash->reset();

  printf("// Starting SoftBreak\n");
  SoftBreak* soft = new SoftBreak(rvd, flash);
  soft->reset();

  printf("// Starting GDBServer\n");
  GDBServer* gdb = new GDBServer(rvd, flash, soft);
  gdb->reset();

  printf("// Starting USB\n");
  tud_init(BOARD_TUD_RHPORT);

  printf("// Starting Console\n");
  Console* console = new Console(rvd, flash, soft);

  printf("// Everything up and running!\n");

  console->reset();

  while (1) {
    //----------------------------------------
    // Update TinyUSB

    tud_task();

    //----------------------------------------
    // Update GDB stub

    bool connected = tud_cdc_n_connected(0);
    bool usb_ie = tud_cdc_n_available(0); // this "available" check is required for some reason
    char usb_in = 0;
    char usb_out = 0;
    bool usb_oe = 0;

    if (usb_ie) {
      tud_cdc_n_read(0, &usb_in, 1);
    }
    gdb->update(connected, usb_ie, usb_in, usb_oe, usb_out);

    if (usb_oe) {
      tud_cdc_n_write(0, &usb_out, 1);
      tud_cdc_n_write_flush(0);
    }

    //----------------------------------------
    // Update uart console

    bool ser_ie = uart_is_readable(uart0);
    char ser_in = 0;

    if (ser_ie) {
      uart_read_blocking(uart0, (uint8_t*)&ser_in, 1);
    }
    console->update(ser_ie, ser_in);
  }


  return 0;
}

//------------------------------------------------------------------------------
