#ifndef ULTILCD2_DEBUG_H
#define ULTILCD2_DEBUG_H
#ifdef DEBUG_MODE


static void lcd_menu_debug_storage(const char* message_P, int16_t &address, int16_t address_min, int16_t address_max,
                                   uint8_t  (*get_byte) (const uint8_t*  address),
                                   uint16_t (*get_word) (const uint16_t* address),
                                   float    (*get_float)(const float*    address)
                                  )
{
    uint8_t i;
    char buffer[22] = {0};
    
    if (lcd_lib_encoder_pos / ENCODER_TICKS_PER_TUNE_VALUE_ITEM != 0)
    {
        address = int(address) + (lcd_lib_encoder_pos / ENCODER_TICKS_PER_TUNE_VALUE_ITEM);
        address = constrain(address, address_min, address_max);
        lcd_lib_encoder_pos = 0;
    }
    if (lcd_lib_button_pressed)
        lcd_change_to_previous_menu();
    
    lcd_lib_clear();
    lcd_lib_draw_string_centerP(BOTTOM_MENU_YPOS, message_P);
    
    strcpy_P (buffer, PSTR("Address: "));
    hex16_to_string(uint16_t(address), buffer + strlen(buffer), PSTR("-"));
    int_to_string  (int     (address), buffer + strlen(buffer));
    lcd_lib_draw_string_left(10, buffer);
    
    
    strcpy_P (buffer, PSTR("String: \""));
    for (i=0; i<8; ++i)
    {
        buffer[9+i] = get_byte ((uint8_t*) (address + i));
    }
    buffer[17] = '\0';
    char* tmp = buffer + strlen(buffer);
    tmp[0] = '\"';
    tmp[1] = '\0';
    lcd_lib_draw_string_left(20, buffer);
    
    strcpy_P (buffer, PSTR("0x"));
    for (i=0; i<8; ++i)
    {
        hex8_to_string(get_byte ((uint8_t*) (address + i)), buffer + strlen(buffer));
    }
    lcd_lib_draw_string_left(30, buffer);
    
    int_to_string   (get_byte ((uint8_t*) address), buffer);
    lcd_lib_draw_string_right(LCD_CHAR_MARGIN_LEFT + 3*LCD_CHAR_SPACING, 40, buffer);
    int_to_string   (get_word ((uint16_t*)address), buffer);
    lcd_lib_draw_string_right(LCD_CHAR_MARGIN_LEFT + 9*LCD_CHAR_SPACING, 40, buffer);
    //
    strcpy_P (buffer, PSTR("- "));
    float_to_string1(get_float((float*)   address), buffer + strlen(buffer), NULL);
    lcd_lib_draw_string(60, 40, buffer);
    
    lcd_lib_update_screen();
}

uint8_t  ram_read_byte  (const uint8_t*  address)  { return *address; }
uint16_t ram_read_word  (const uint16_t* address)  { return *address; }
float    ram_read_float (const float*    address)  { return *address; }

static void lcd_menu_debug_eeprom()
{
    static int16_t eeprom_pos = 0;
    lcd_menu_debug_storage (PSTR("EEPROM-Click to ret."), eeprom_pos, 0, E2END,
                            eeprom_read_byte, eeprom_read_word, eeprom_read_float
                           );
}

static void lcd_menu_debug_ram()
{
    static int16_t ram_pos = 0;
    lcd_menu_debug_storage (PSTR("RAM   -Click to ret."), ram_pos, 0, RAMEND,
                            ram_read_byte, ram_read_word, ram_read_float
                           );
}

#endif // DEBUG_MODE
#endif // ULTILCD2_DEBUG_H
