# Hitachi LCD-II Software Driver

> A software driver written in C for the Hitachi LCD-II set of devices targeted for the Arty Z7 family of devices.

[![License](http://img.shields.io/:license-mit-blue.svg?style=flat-square)](http://badges.mit-license.org)

---
## Installation

- Clone this repo to your local machine using `https://github.com/tstword/HD44780U-ArtyZ7-Driver`

## Using The Source Code

### Prerequisites
- Simply copy the header and source files into your Vitis or SDK project and include the path to the header

### Example Program

- The following is an example program that displays the message "Hello World" on the first line and the message "Writing: 101" on the second line:

```C
#include "xil_printf.h"
#include "hd44780u_driver.h"

/* Comment to make the LCD operate in 8-bit mode */
#define MODE_4BIT

#define D0 0x7	/* d0 pin connected to IO 7 */
#define D1 0x6	/* d1 pin connected to IO 6 */
#define D2 0x5	/* d2 pin connected to IO 5 */
#define D3 0x4	/* d3 pin connected to IO 4 */
#define D4 0x3	/* d4 pin connected to IO 3 */
#define D5 0x2  /* d5 pin connected to IO 2 */
#define D6 0x1  /* d6 pin connected to IO 1 */
#define D7 0x0  /* d7 pin connected to IO 0 */

#define EN 0x8	/* en pin connected to IO 8 */
#define RS 0x9  /* rs pin connected to IO 9 */

int main(void)
{
    /* Structure required for function calls */
    struct hd44780u_dev lcd_device;
    int init_result, begin_result;

    /* Initialize the LCD structure (8-bit mode or 4-bit mode) */
#ifdef MODE_4BIT
    init_result = hd44780u_init_4bit(&lcd_device, 0x41200000, RS, EN, D4, D5, D6, D7);
#else
    init_result = hd44780u_init_8bit(&lcd_device, 0x41200000, RS, EN, D0, D1, D2, D3, D4, D5, D6, D7);
#endif
    if(init_result == -1) {
        xil_printf("Failed to initialized LCD structure: %s\r\n", hd44780u_error_msg(&lcd_device));
        return -1;
    }

    /* Start LCD with 2 rows and 16 columns */
    begin_result = hd44780u_begin(&lcd_device, 2, 16);
    if(begin_result == -1) {
        xil_printf("Failed to start LCD device: %s\r\n", hd44780u_error_msg(&lcd_device));
        return -1;
    }

    /* Start writing to the LCD */
    hd44780u_write_message(&lcd_device, "Hello World!");
    hd44780u_set_cursor(&lcd_device, 1, 0);
    hd44780u_write_message(&lcd_device, "Writing: ");
    hd44780u_write_int(&lcd_device, 101);

    return 0;
}
```

<img src="https://drive.google.com/uc?export=view&id=1n-HHJz0yQ2-7-PcVBxnbQBA1bHT_3XC1">
