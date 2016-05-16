#ifndef ULTILCD2_DEBUG_H
#define ULTILCD2_DEBUG_H
#ifdef DEBUG_MODE

#define  DEBUG_STRING_SIZE 6
const char MSGP_DEBUG_STRING[] PROGMEM = {"debug\0"};

extern int16_t debug_ram_pos;

static void lcd_menu_debug_storage(const char* message_P, int16_t &address, int16_t address_min, int16_t address_max,
                                   uint8_t  (*get_byte) (const uint8_t*  address),
                                   uint16_t (*get_word) (const uint16_t* address),
                                   float    (*get_float)(const float*    address)
                                  );

void lcd_menu_debug_eeprom();
void lcd_menu_debug_ram();
void debug_ram_find_var();

#ifdef DEBUG_VAR_SIZE

extern uint16_t debug_var_end;
extern char     debug_var[DEBUG_STRING_SIZE + DEBUG_VAR_SIZE];

void debug_init();

void debug_add_byte (uint8_t  var);
void debug_add_word (uint16_t var);
void debug_add_float(float    var);

#endif // DEBUG_VAR_SIZE

#endif // DEBUG_MODE
#endif // ULTILCD2_DEBUG_H
