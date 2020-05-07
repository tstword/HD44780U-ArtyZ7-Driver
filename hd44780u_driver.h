#ifndef HD44780U_DRIVER_H
#define HD44780U_DRIVER_H

#include <stdint.h>

// Macros indicating 8-bit or 4-bit instruction mode
#define HD44780U_DEV_MODE_4BIT 0x04
#define HD44780U_DEV_MODE_8BIT 0x08

// Macros required for entry instruction
#define HD44780U_LCD_ENTRY_CTL    0x04

#define HD44780U_LCD_INCREMENT    0x02
#define HD44780U_LCD_DECREMENT    0x00

#define HD44780U_LCD_UPDATE_DISP  0x01
#define HD44780U_LCD_UPDATE_CURS  0x00

// Macros required for display instruction
#define HD44780U_LCD_DISPLAY_CTL  0x08

#define HD44780U_LCD_DISPLAY_ON   0x04
#define HD44780U_LCD_DISPLAY_OFF  0x00

#define HD44780U_LCD_CURSOR_ON    0x02
#define HD44780U_LCD_CURSOR_OFF   0x00

#define HD44780U_LCD_BLINKING_ON  0x01
#define HD44780U_LCD_BLINKING_OFF 0x00

// Macros required for function instruction
#define HD44780U_LCD_FUNC_CTL     0x20

#define HD44780U_LCD_DATA_8BIT    0x10
#define HD44780U_LCD_DATA_4BIT    0x00

#define HD44780U_LCD_LINE_SINGLE  0x00
#define HD44780U_LCD_LINE_DOUBLE  0x08

#define HD44780U_LCD_FONT_5x10    0x04
#define HD44780U_LCD_FONT_5x8     0x00

// Macros required for shift instruction
#define HD44780U_LCD_SHIFT_CTL    0x10

#define HD44780U_LCD_DISP_SHIFT   0x08
#define HD44780U_LCD_CURS_SHIFT   0x00

#define HD44780U_LCD_SHIFT_RIGHT  0x04
#define HD44780U_LCD_SHIFT_LEFT   0x00

// Macros required for setting DRAM address
#define HD44780U_LCD_DRAM_CTL     0x80

// Macros required for row / col limit
#define HD44780U_LCD_1R_COL_MAXLEN  0x50
#define HD44780U_LCD_2R_COL_MAXLEN 	0x28
#define HD44780U_LCD_ROW_MAXLEN    	0x02

// Macros utilized for reporting errors
#define HD44780U_ERR_NONE	          0x00
#define HD44780U_ERR_NOT_INITIALIZED  0x01
#define HD44780U_ERR_NOT_RUNNING      0x02
#define HD44780U_ERR_INV_DEV_MODE 	  0x03
#define HD44780U_ERR_INV_CELL_NUM	  0x04
#define HD44780U_ERR_INV_CURSOR       0x05
#define HD44780U_ERR_MSG_TOO_LONG     0x06
#define HD44780U_ERR_NO_SPACE         0x07

struct hd44780u_dev {
    volatile uint32_t *gpio_base;

    uint8_t curr_row;
    uint8_t curr_col;
    uint8_t cell_rows;
    uint8_t cell_cols;

    uint8_t data_pins[8];
    uint8_t lcd_mode;
    uint8_t enable_pin;
    uint8_t select_pin;

    uint8_t initialized;
    uint8_t running;
    uint8_t error_code;
};

int hd44780u_init(struct hd44780u_dev *device, uint32_t gpio_addr, uint8_t rs, uint8_t en, uint8_t *data_pins, uint8_t lcd_mode);
int hd44780u_begin(struct hd44780u_dev *device, uint8_t rows, uint8_t columns);

int hd44780u_clear_display(struct hd44780u_dev *device);

int hd44780u_shift_view_left(struct hd44780u_dev *device);
int hd44780u_shift_view_right(struct hd44780u_dev *device);
int hd44780u_reset_view(struct hd44780u_dev *device);

int hd44780u_set_cursor(struct hd44780u_dev *device, uint8_t row, uint8_t column);
int hd44780u_write_message(struct hd44780u_dev *device, const char *message);
int hd44780u_write_char(struct hd44780u_dev *device, char ch);
int hd44780u_write_int(struct hd44780u_dev *device, int value);

#endif
