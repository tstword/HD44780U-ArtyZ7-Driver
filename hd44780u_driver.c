#include "hd44780u_driver.h"
#include "sleep.h"

static inline void hd44780u_enable_pulse(struct hd44780u_dev *device) {
    *(device->gpio_base) &= ~(1 << device->enable_pin);
    usleep(1);
    *(device->gpio_base) |= (1 << device->enable_pin);
    usleep(1);
    *(device->gpio_base) &= ~(1 << device->enable_pin);
    usleep(42);
}

static void hd44780u_write_4bit(struct hd44780u_dev *device, uint8_t data) {
    for(int i = 0; i < 4; ++i) {
        *(device->gpio_base) &= ~(1 << device->data_pins[i]);
        *(device->gpio_base) |= ((data & 0x1) << device->data_pins[i]);
        data >>= 1;
    }
    hd44780u_enable_pulse(device);
}

static void hd44780u_write_8bit(struct hd44780u_dev *device, uint8_t data) {
    for(int i = 0; i < 8; ++i) {
        *(device->gpio_base) &= ~(1 << device->data_pins[i]);
        *(device->gpio_base) |= ((data & 0x1) << device->data_pins[i]);
        data >>= 1;
    }
    hd44780u_enable_pulse(device);
}

static inline void hd44780u_send_command(struct hd44780u_dev *device, uint8_t data, uint8_t rs) {
    *(device->gpio_base) &= ~(1 << device->select_pin);
    *(device->gpio_base) |= (rs & 0x1) << device->select_pin;

    if(device->lcd_mode == HD44780U_DEV_MODE_4BIT) {
        hd44780u_write_4bit(device, data >> 4);
        hd44780u_write_4bit(device, data);
    }
    else if(device->lcd_mode == HD44780U_DEV_MODE_8BIT){
        hd44780u_write_8bit(device, data);
    }
}

int hd44780u_clear_display(struct hd44780u_dev *device) {
    if(!device->running || !device->initialized) {
        device->error_code = HD44780U_ERR_NOT_RUNNING;
        return -1;
    }

    device->curr_col = 0;
    device->curr_row = 0;

    hd44780u_send_command(device, 0b00000001, 0);
    usleep(1520);

    return 0;
}

int hd44780u_begin(struct hd44780u_dev *device, uint8_t rows, uint8_t columns) {
    uint8_t function_flags = HD44780U_LCD_FUNC_CTL | HD44780U_LCD_LINE_SINGLE
            | HD44780U_LCD_FONT_5x8;
    
    uint8_t display_flags = HD44780U_LCD_DISPLAY_CTL | HD44780U_LCD_DISPLAY_ON |
                            HD44780U_LCD_CURSOR_OFF | HD44780U_LCD_BLINKING_ON;
    
    uint8_t entry_flags = HD44780U_LCD_ENTRY_CTL | HD44780U_LCD_INCREMENT |
                            HD44780U_LCD_UPDATE_CURS;

    if(!device->initialized) {
        device->error_code =  HD44780U_ERR_NOT_INITIALIZED;
        return -1;
    }

    if(rows > HD44780U_LCD_ROW_MAXLEN
            || (rows == 1 && columns > HD44780U_LCD_1R_COL_MAXLEN)
            || (rows == 2 && columns > HD44780U_LCD_2R_COL_MAXLEN)) {
        device->error_code = HD44780U_ERR_INV_CELL_NUM;
        return -1;
    }

    // Set row and columns for device
    device->curr_row = 0;
    device->curr_col = 0;
    device->cell_rows = rows;
    device->cell_cols = columns;

    if(rows == 2) function_flags |= HD44780U_LCD_LINE_DOUBLE;

    // Wait for 50 ms for voltage from v2.7 to rise
    usleep(50000);

    if(device->lcd_mode == HD44780U_DEV_MODE_4BIT) {
        // 1st write (wait 5ms after)
        hd44780u_write_4bit(device, 0b0011);
        usleep(5000);

        // 2nd write (wait 150 microseconds after)
        hd44780u_write_4bit(device, 0b0011);
        usleep(150);

        // 3rd write
        hd44780u_write_4bit(device, 0b0011);
        usleep(42);

        // Set in 4-bit mode
        hd44780u_write_4bit(device, 0b0010);

        // Function set (number of lines + font + 4-bit mode)
        hd44780u_send_command(device, function_flags | HD44780U_LCD_DATA_4BIT, 0);
    }
    else if(device->lcd_mode == HD44780U_DEV_MODE_8BIT) {
        // 1st write (wait 5ms after)
        hd44780u_write_8bit(device, 0b00110000);
        usleep(5000);

        // 2nd write (wait 150 microseconds after)
        hd44780u_write_8bit(device, 0b00110000);
        usleep(150);

        // 3rd write
        hd44780u_write_8bit(device, 0b00110000);
        usleep(42);

        // Set in 8-bit mode
        hd44780u_write_4bit(device, 0b0010);

        // Function set (number of lines + font + 8-bit mode)
        hd44780u_send_command(device, function_flags | HD44780U_LCD_DATA_8BIT, 0);
    }
    else {
        device->error_code = HD44780U_ERR_INV_DEV_MODE;
        return -1;
    }

    // Turn on the display
    hd44780u_send_command(device, display_flags, 0);

    // Clear the display
    hd44780u_send_command(device, 0b00000001, 0);
    usleep(1520);

    // Entry mode set    
    hd44780u_send_command(device, entry_flags, 0);

    device->running = 1;

    return 0;
}

int hd44780u_init(struct hd44780u_dev *device, uint32_t gpio_addr, uint8_t rs, uint8_t en, uint8_t *data_pins, uint8_t lcd_mode) {
    uint32_t gpio_tri = 0xFFFFFFFF;

    // Verify 8-bit or 4-bit mode
    if(lcd_mode != HD44780U_DEV_MODE_4BIT && lcd_mode != HD44780U_DEV_MODE_8BIT) {
        device->initialized = 0;
        device->error_code = HD44780U_ERR_INV_DEV_MODE;
        return -1;
    }
    device->lcd_mode = lcd_mode;

    for(int i = 0; i < lcd_mode; ++i) {
        device->data_pins[i] = data_pins[i];
        gpio_tri ^= (1 << data_pins[i]);
    }

    device->enable_pin = en;
    device->select_pin = rs;
    gpio_tri ^= (1 << rs) | (1 << en);

    device->gpio_base = (volatile uint32_t *)gpio_addr;
    *(device->gpio_base) = 0;
    *(device->gpio_base + 1) = gpio_tri;

    device->initialized = 1;

    device->error_code = HD44780U_ERR_NONE;

    return 0;
}

int hd44780u_write_int(struct hd44780u_dev *device, int value) {
    uint8_t digit_stack[10];    // 4-byte values can store at max 10 digits
    uint8_t neg = (value >> 31) & 0x1;
    int8_t stack_size = 0;

    do {
        if(neg)
            digit_stack[stack_size++] = -(value % 10);
        else
            digit_stack[stack_size++] = value % 10;
        value /= 10;
    } while(value != 0 && stack_size < 10);

    if(neg && hd44780u_write_char(device, '-') == -1)
        return -1;

    for(--stack_size; stack_size >= 0; --stack_size) {
        if(hd44780u_write_char(device, digit_stack[stack_size] + '0') == -1)
            return -1;
    }

    return 0;
}

int hd44780u_write_char(struct hd44780u_dev *device, char ch) {
    if(device->curr_row >= device->cell_rows - 1 && device->curr_col >= device->cell_cols) {
        device->error_code = HD44780U_ERR_NO_SPACE;
        return -1;
    }

    hd44780u_send_command(device, (uint8_t)ch, 1);
    ++device->curr_col;

    if(device->curr_col >= device->cell_cols && device->curr_row + 1 < device->cell_rows)
        hd44780u_set_cursor(device, device->curr_row + 1, 0);
    
    return 0;
}

int hd44780u_write_message(struct hd44780u_dev *device, const char *message) {
    if(!device->running || !device->initialized) {
        device->error_code = HD44780U_ERR_NOT_RUNNING;
        return -1;
    }

    for(; *message != '\0'; ++message) {
        if(hd44780u_write_char(device, *message) == -1)
            return -1;
    }

    return 0;
}

int hd44780u_shift_view_left(struct hd44780u_dev *device) {
    hd44780u_send_command(device, HD44780U_LCD_SHIFT_CTL |
            HD44780U_LCD_DISP_SHIFT |
            HD44780U_LCD_SHIFT_RIGHT, 0);
    return 0;
}

int hd44780u_shift_view_right(struct hd44780u_dev *device) {
    hd44780u_send_command(device, HD44780U_LCD_SHIFT_CTL |
        HD44780U_LCD_DISP_SHIFT |
        HD44780U_LCD_SHIFT_LEFT, 0);
    return 0;
}

int hd44780u_reset_view(struct hd44780u_dev *device) {
    // Utilize home command (DRAM => 0, shift = 0)
    hd44780u_send_command(device, 0b00000010, 0);
    hd44780u_set_cursor(device, device->curr_row, device->curr_col);

    return 0;
}

int hd44780u_set_cursor(struct hd44780u_dev *device, uint8_t row, uint8_t column) {
    if(row >= HD44780U_LCD_ROW_MAXLEN || column >= HD44780U_LCD_1R_COL_MAXLEN) {
        device->error_code = HD44780U_ERR_INV_CELL_NUM;
        return -1;
    }

    if(row >= device->cell_rows || column >= device->cell_cols) {
        device->error_code = HD44780U_ERR_INV_CURSOR;
        return -1;
    }

    // Better way of setting the cursor
    if(device->cell_rows == 2) {
        if(row == 1)
            hd44780u_send_command(device, HD44780U_LCD_DRAM_CTL | (column + 0x40), 0);
        else
            hd44780u_send_command(device, HD44780U_LCD_DRAM_CTL | (column + 0x00), 0);
    }
    else
        hd44780u_send_command(device, HD44780U_LCD_DRAM_CTL | (column + 0x00), 0);

    device->curr_row = row;
    device->curr_col = column;

    return 0;
}
