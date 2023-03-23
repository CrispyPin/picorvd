#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "pico/binary_info.h"
#include "build/singlewire.pio.h"
#include "hardware/clocks.h"
#include "WCH_Debug.h"
#include "log.h"
#include "gdb_server.h"
#include "utils.h"

const uint SWD_PIN = 14;

extern "C" {
  void stdio_usb_out_chars(const char *buf, int length);
  int  stdio_usb_in_chars (char *buf, int length);

  void uart_write_blocking(uart_inst_t *uart, const uint8_t *src, size_t len);
  void uart_read_blocking (uart_inst_t *uart, uint8_t *dst, size_t len);
};

void ser_put(char c) {
  if (c == '\n') ser_put('\r');
  uart_write_blocking(uart0, (uint8_t*)&c, 1);
}

char ser_get() {
  char r = 0;
  uart_read_blocking(uart0, (uint8_t*)&r, 1);
  return r;
}

void usb_put(char c) {
  if (c == '\n') usb_put('\r');
  stdio_usb_out_chars(&c, 1);
}

char usb_get() {
  char r = 0;
  while (1) {
    int len = stdio_usb_in_chars(&r, 1);
    if (len) return r;
  }
  // should never get here...
  return 0;
}


//------------------------------------------------------------------------------

void usb_printf(const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vprint_to(usb_put, fmt, args);
}

void ser_printf(const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vprint_to(ser_put, fmt, args);
}

//------------------------------------------------------------------------------

unsigned char blink_bin[] = {
  0x6f, 0x00, 0xe0, 0x18, 0x00, 0x00, 0x00, 0x00, 0x20, 0x02, 0x00, 0x00,
  0x20, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x20, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x02, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x20, 0x02, 0x00, 0x00, 0x20, 0x02, 0x00, 0x00,
  0x20, 0x02, 0x00, 0x00, 0x20, 0x02, 0x00, 0x00, 0x20, 0x02, 0x00, 0x00,
  0x20, 0x02, 0x00, 0x00, 0x20, 0x02, 0x00, 0x00, 0x20, 0x02, 0x00, 0x00,
  0x20, 0x02, 0x00, 0x00, 0x20, 0x02, 0x00, 0x00, 0x20, 0x02, 0x00, 0x00,
  0x20, 0x02, 0x00, 0x00, 0x20, 0x02, 0x00, 0x00, 0x20, 0x02, 0x00, 0x00,
  0x20, 0x02, 0x00, 0x00, 0x20, 0x02, 0x00, 0x00, 0x20, 0x02, 0x00, 0x00,
  0x20, 0x02, 0x00, 0x00, 0x20, 0x02, 0x00, 0x00, 0x20, 0x02, 0x00, 0x00,
  0x20, 0x02, 0x00, 0x00, 0x20, 0x02, 0x00, 0x00, 0x20, 0x02, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0xb7, 0xf7, 0x00, 0xe0, 0xd8, 0x43, 0x79, 0x9b,
  0xd8, 0xc3, 0x88, 0xcb, 0x23, 0xa4, 0x07, 0x00, 0x98, 0x43, 0x13, 0x67,
  0x17, 0x00, 0x98, 0xc3, 0xd8, 0x43, 0x05, 0x8b, 0x75, 0xdf, 0x98, 0x43,
  0x79, 0x9b, 0x98, 0xc3, 0x82, 0x80, 0x51, 0x11, 0x37, 0x07, 0x00, 0x01,
  0x06, 0xc4, 0x22, 0xc2, 0x26, 0xc0, 0xb7, 0x17, 0x02, 0x40, 0x05, 0x07,
  0x98, 0xc3, 0x23, 0xa2, 0x07, 0x00, 0x37, 0x27, 0x02, 0x40, 0x85, 0x46,
  0x14, 0xc3, 0x37, 0x07, 0x9f, 0x00, 0x98, 0xc7, 0xb7, 0x06, 0x00, 0x02,
  0x37, 0x17, 0x02, 0x40, 0x1c, 0x43, 0xf5, 0x8f, 0xf5, 0xdf, 0x5c, 0x43,
  0xa1, 0x46, 0xf1, 0x9b, 0x93, 0xe7, 0x27, 0x00, 0x5c, 0xc3, 0xb7, 0x17,
  0x02, 0x40, 0xd8, 0x43, 0x31, 0x8b, 0xe3, 0x1e, 0xd7, 0xfe, 0x98, 0x4f,
  0xb7, 0x06, 0xf1, 0xff, 0xfd, 0x16, 0x13, 0x67, 0x07, 0x02, 0x98, 0xcf,
  0xb7, 0x17, 0x01, 0x40, 0x03, 0xa7, 0x07, 0x40, 0xb7, 0x54, 0x12, 0x00,
  0x93, 0x84, 0x04, 0xf8, 0x41, 0x9b, 0x23, 0xa0, 0xe7, 0x40, 0x03, 0xa7,
  0x07, 0x40, 0x13, 0x67, 0x37, 0x00, 0x23, 0xa0, 0xe7, 0x40, 0x03, 0xa7,
  0x07, 0x40, 0x75, 0x8f, 0x23, 0xa0, 0xe7, 0x40, 0x03, 0xa7, 0x07, 0x40,
  0xb7, 0x06, 0x03, 0x00, 0x55, 0x8f, 0x23, 0xa0, 0xe7, 0x40, 0x37, 0x14,
  0x01, 0x40, 0xc5, 0x47, 0x23, 0x28, 0xf4, 0x40, 0x26, 0x85, 0x2d, 0x3f,
  0xb7, 0x07, 0x11, 0x00, 0x37, 0x25, 0x09, 0x00, 0x23, 0x28, 0xf4, 0x40,
  0x13, 0x05, 0x05, 0x7c, 0x25, 0x37, 0xc5, 0xb7, 0x2a, 0x96, 0xaa, 0x87,
  0x63, 0x93, 0xc7, 0x00, 0x82, 0x80, 0x23, 0x80, 0xb7, 0x00, 0x85, 0x07,
  0xd5, 0xbf, 0x97, 0x01, 0x00, 0x20, 0x93, 0x81, 0x21, 0x67, 0x13, 0x81,
  0x01, 0x00, 0x93, 0x02, 0x00, 0x08, 0x73, 0x90, 0x02, 0x30, 0x93, 0x02,
  0x30, 0x00, 0x73, 0x90, 0x42, 0x80, 0x97, 0x02, 0x00, 0x00, 0x93, 0x82,
  0x62, 0xe5, 0x93, 0xe2, 0x32, 0x00, 0x73, 0x90, 0x52, 0x30, 0xb7, 0x07,
  0x00, 0x20, 0x03, 0xa5, 0x07, 0x00, 0xb7, 0x07, 0x00, 0x20, 0x83, 0xa7,
  0x07, 0x00, 0x13, 0x07, 0xd5, 0xff, 0x13, 0x06, 0x00, 0x00, 0x63, 0xe8,
  0xe7, 0x00, 0x13, 0x86, 0x37, 0x00, 0x33, 0x06, 0xa6, 0x40, 0x13, 0x76,
  0xc6, 0xff, 0x93, 0x05, 0x00, 0x00, 0x59, 0x3f, 0x03, 0x27, 0x40, 0x22,
  0xb7, 0x06, 0x00, 0x20, 0xb7, 0x07, 0x00, 0x20, 0x83, 0xa7, 0x07, 0x00,
  0x83, 0xa6, 0x06, 0x00, 0x63, 0x98, 0xd7, 0x00, 0x93, 0x07, 0x60, 0x0c,
  0x73, 0x90, 0x17, 0x34, 0x73, 0x00, 0x20, 0x30, 0x03, 0x26, 0x07, 0x00,
  0x13, 0x07, 0x47, 0x00, 0x93, 0x87, 0x47, 0x00, 0x23, 0xae, 0xc7, 0xfe,
  0x6f, 0xf0, 0x1f, 0xfe, 0x6f, 0x00, 0x00, 0x00
};
unsigned int blink_bin_len = 548;

//------------------------------------------------------------------------------

int main() {
  stdio_usb_init();

  Log log(ser_put);

  // Enable non-USB serial port on gpio 0/1 for meta-debug output :D
  uart_init(uart0, 3000000);
  gpio_set_function(0, GPIO_FUNC_UART);
  gpio_set_function(1, GPIO_FUNC_UART);  

  // Wait for Minicom to finish connecting before sending clear screen etc
  //sleep_ms(100);

  //SLDebugger sl;
  //sl.init(SWD_PIN, usb_printf);
  //sl.reset();

  char command[256] = {0};
  uint8_t cursor = 0;

  //log("\033[2J");
  log("PicoDebug 0.0.2\n");
  log("<debug log begin>\n");

  /*
  while(1) {
    auto c = usb_get();
    ser_put(c);
  }
  */

  //while(!stdio_usb_connected());
  //log("stdio_usb_connected()\n");

  GDBServer server(usb_get, usb_put, log);
  server.loop();

#if 0
  while(1) {
    usb_printf("> ");
    while(1) {
      auto c = getchar();
      if (c == 8) {
        usb_printf("\b \b");
        cursor--;
        command[cursor] = 0;
      }
      else if (c == '\n' || c == '\r') {
        command[cursor] = 0;
        usb_printf("\n");
        break;
      }
      else {
        usb_printf("%c", c);
        command[cursor++] = c;
      }
    }

    uint32_t time_a = time_us_32();

    if (strcmp(command, "status") == 0)      sl.print_status();
    if (strcmp(command, "dump_flash") == 0)  sl.dump_flash();
    if (strcmp(command, "wipe_chip") == 0)   sl.wipe_chip();
    if (strcmp(command, "write_flash") == 0) sl.write_flash(0x08000000, (uint32_t*)blink_bin, blink_bin_len / 4);
    if (strcmp(command, "unhalt") == 0)      sl.unhalt(/*reset*/ true);
    if (strcmp(command, "halt") == 0)        sl.halt();

    int err = Reg_ABSTRACTCS(sl.get_dbg(ADDR_ABSTRACTCS)).CMDER;
    if (err) {
      printf("CMDERR = %d\n", err);
      sl.clear_err();
    }

    uint32_t time_b = time_us_32();
    usb_printf("Command took %d us\n", time_b - time_a);

    cursor = 0;
    command[cursor] = 0;
  }
#endif
}

//------------------------------------------------------------------------------
