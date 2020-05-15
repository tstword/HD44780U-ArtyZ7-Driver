/*
 * A C software driver compatible with the Hitachi LCD-II device
 *
 * The Hitachi LCD-II driver works with any compatible LCDs. The
 * code below is written for the Arty Z7 family (or rather pair)
 * of devices but will work any ZYNQ device that has an Arduino
 * shield connected with an AXI4 GPIO IP.
 *
 * AXI4 GPIO DETAILS: The software driver modifies the TRI-STATE register
 * of the peripheral. The driver will set the specified pins to write mode
 * and the will not modify the read-write state of the other pins.
 *
 * Note: See https://github.com/tstword/HD44780U-ArtyZ7-Driver for examples
 */

#ifndef HD44780U_DRIVER_H
#define HD44780U_DRIVER_H

#include <stdint.h>

/* Macros indicating 8-bit or 4-bit instruction mode */
#define HD44780U_DEV_MODE_4BIT 0x04
#define HD44780U_DEV_MODE_8BIT 0x08

/* Macros required for entry instruction */
#define HD44780U_LCD_ENTRY_CTL    0x04

#define HD44780U_LCD_INCREMENT    0x02
#define HD44780U_LCD_DECREMENT    0x00

#define HD44780U_LCD_UPDATE_DISP  0x01
#define HD44780U_LCD_UPDATE_CURS  0x00

/* Macros required for display instruction */
#define HD44780U_LCD_DISPLAY_CTL  0x08

#define HD44780U_LCD_DISPLAY_ON   0x04
#define HD44780U_LCD_DISPLAY_OFF  0x00

#define HD44780U_LCD_CURSOR_ON    0x02
#define HD44780U_LCD_CURSOR_OFF   0x00

#define HD44780U_LCD_BLINKING_ON  0x01
#define HD44780U_LCD_BLINKING_OFF 0x00

/* Macros required for function instruction */
#define HD44780U_LCD_FUNC_CTL     0x20

#define HD44780U_LCD_DATA_8BIT    0x10
#define HD44780U_LCD_DATA_4BIT    0x00

#define HD44780U_LCD_LINE_SINGLE  0x00
#define HD44780U_LCD_LINE_DOUBLE  0x08

#define HD44780U_LCD_FONT_5x10    0x04
#define HD44780U_LCD_FONT_5x8     0x00

/* Macros required for shift instruction */
#define HD44780U_LCD_SHIFT_CTL    0x10

#define HD44780U_LCD_DISP_SHIFT   0x08
#define HD44780U_LCD_CURS_SHIFT   0x00

#define HD44780U_LCD_SHIFT_RIGHT  0x04
#define HD44780U_LCD_SHIFT_LEFT   0x00

/* Macros required for setting DRAM address */
#define HD44780U_LCD_DRAM_CTL     0x80

/* Macros required for row / column limit */
#define HD44780U_LCD_1R_COL_MAXLEN  0x50
#define HD44780U_LCD_2R_COL_MAXLEN  0x28
#define HD44780U_LCD_ROW_MAXLEN     0x02

/* Macros utilized for reporting errors */
#define HD44780U_ERR_NONE             0x00
#define HD44780U_ERR_NOT_INITIALIZED  0x01
#define HD44780U_ERR_NOT_RUNNING      0x02
#define HD44780U_ERR_INV_DEV_MODE     0x03
#define HD44780U_ERR_INV_CELL_NUM     0x04
#define HD44780U_ERR_INV_CURSOR       0x05
#define HD44780U_ERR_NO_SPACE         0x06

struct hd44780u_dev {
    volatile uint32_t *gpio_base;	/* Base address for AXI4 GPIO IP attached to Arduino shield */

    uint8_t curr_row;				/* Indicates the current row of the LCD */
    uint8_t curr_col;				/* Indicates the current column of the LCD */
    uint8_t cell_rows;				/* Indicates the total rows of the LCD */
    uint8_t cell_cols;				/* Indicates the total columns of the LCD */

    uint8_t data_pins[8];			/* Reference to data pins (D0-D7) on the Arduino shield */
    uint8_t lcd_mode;				/* Indicates 8-bit (HD44780U_DEV_MODE_8BIT) or 4-bit (HD44780U_DEV_MODE_4BIT) mode */
    uint8_t enable_pin;				/* Reference to the enable pin (EN) on the Arduino shield */
    uint8_t select_pin;				/* Reference to the ready set pin (RS) on the Arduino shield */

    uint8_t initialized;			/* Flag indicating the initialized status of device */
    uint8_t running;				/* Flag indicating whether device is properly reset and running */
    uint8_t error_code;				/* Error code indicating failure (see error codes above) */
};

int hd44780u_init_4bit(struct hd44780u_dev *device, uint32_t gpio_addr, uint8_t rs, uint8_t en,
		uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7);
int hd44780u_init_8bit(struct hd44780u_dev *device, uint32_t gpio_addr, uint8_t rs, uint8_t en,
		uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3, uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7);

int hd44780u_begin(struct hd44780u_dev *device, uint8_t rows, uint8_t columns);

int hd44780u_clear_display(struct hd44780u_dev *device);

int hd44780u_shift_view_left(struct hd44780u_dev *device);
int hd44780u_shift_view_right(struct hd44780u_dev *device);
int hd44780u_reset_view(struct hd44780u_dev *device);

int hd44780u_set_cursor(struct hd44780u_dev *device, uint8_t row, uint8_t column);

int hd44780u_write_message(struct hd44780u_dev *device, const char *message);
int hd44780u_write_char(struct hd44780u_dev *device, char ch);
int hd44780u_write_int(struct hd44780u_dev *device, int value);

const char *hd44780u_error_msg(struct hd44780u_dev *device);

#endif
