#ifndef ULTILCD2_DEBUG_CPP
#define ULTILCD2_DEBUG_CPP
#include "helper.h"
#include "UltiLCD2.h"
#include "UltiLCD2_hi_lib.h"
#include "UltiLCD2_menu_utils.h"
#include "UltiLCD2_debug.h"

#ifdef DEBUG_MODE

int16_t debug_ram_pos = 0;

static void lcd_menu_debug_storage(const char* message_P, int16_t &address, int16_t address_min, int16_t address_max,
                                   uint8_t  (*get_byte) (const uint8_t*  address),
                                   uint16_t (*get_word) (const uint16_t* address),
                                   float    (*get_float)(const float*    address)
                                  )
{
    uint8_t i;
    char buffer[22];
    
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

void lcd_menu_debug_eeprom()
{
    static int16_t eeprom_pos = 0;
    lcd_menu_debug_storage (PSTR("EEPROM-Click to ret."), eeprom_pos, 0, E2END,
                            eeprom_read_byte, eeprom_read_word, eeprom_read_float
                           );
}

void lcd_menu_debug_ram()
{
    lcd_menu_debug_storage (PSTR("RAM   -Click to ret."), debug_ram_pos, 0, RAMEND,
                            ram_read_byte, ram_read_word, ram_read_float
                           );
}

void debug_ram_find_var()
{
    if (debug_ram_pos + DEBUG_STRING_SIZE >= RAMEND) debug_ram_pos = 0;
    for ( ;debug_ram_pos < RAMEND - DEBUG_STRING_SIZE; ++debug_ram_pos)
    {
        if (memcmp_P((const void*)debug_ram_pos, (const void*)MSGP_DEBUG_STRING, DEBUG_STRING_SIZE) == 0)
            return;
    }
    debug_ram_pos = 0;
}

#ifdef DEBUG_VAR_SIZE

uint16_t debug_var_end   = 0;
uint16_t debug_var_count = 0;
char     debug_var[DEBUG_STRING_SIZE + DEBUG_VAR_SIZE];

void debug_init()
{
    strcpy_P (debug_var, MSGP_DEBUG_STRING);
    for (int i = DEBUG_STRING_SIZE; i < DEBUG_STRING_SIZE + DEBUG_VAR_SIZE; ++i)
        debug_var[i] = '\0';
    debug_var_end   = 0;
    debug_var_count = 0;
}

inline bool debug_is_in_space (size_t size)
{
    if (debug_var_end + size >= DEBUG_VAR_SIZE) return false;
    return true;
}

template <typename T>
void debug_add_value (T var)
{
    if (debug_var_count < DEBUG_VAR_BEGIN)
    {
        debug_var_count += sizeof(var);
        return;
    }
    if (debug_is_in_space(sizeof(var)))
    {
        *((typeof(var)*) (debug_var + DEBUG_STRING_SIZE + debug_var_end)) = var;
        debug_var_end += sizeof(var);
    }
}

void debug_add_string(char* str)
{
    do
    {
        debug_add_byte (*str);
    }
    while (*(str++) != '\0');
}

void debug_add_block (void* ptr, size_t size)
{
    char* str = (char*) ptr;
    while (size-- > 0);
    {
        debug_add_byte (*(str++));
    }
}

#endif // DEBUG_VAR_SIZE

#endif // DEBUG_MODE
#endif // ULTILCD2_DEBUG_CPP
