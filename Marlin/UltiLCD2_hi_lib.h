#ifndef ULTI_LCD2_HI_LIB_H
#define ULTI_LCD2_HI_LIB_H

#include "preferences.h"
#include "UltiLCD2_low_lib.h"
#include "UltiLCD2_gfx.h"
#include "UltiLCD2_menu_utils.h"

//Arduino IDE compatibility, lacks the eeprom_read_float function
#pragma weak eeprom_read_float
float eeprom_read_float(const float* addr);
#pragma weak eeprom_update_float
void eeprom_update_float(const float* addr, float f);

void lcd_tripple_menu(const char* left, const char* right, const char* bottom);
void lcd_basic_screen();
void lcd_info_screen(menuFunc_t cancelMenu, menuFunc_t callbackOnCancel = NULL, const char* cancelButtonText = NULL);
void lcd_question_screen(menuFunc_t optionAMenu, menuFunc_t callbackOnA, const char* AButtonText, menuFunc_t optionBMenu, menuFunc_t callbackOnB, const char* BButtonText);
// void lcd_scroll_menu(const char* menuNameP, int8_t entryCount, entryNameCallback_t entryNameCallback, entryDetailsCallback_t entryDetailsCallback);
void lcd_scroll_menu(const char* menuNameP, int8_t entryCount, scrollDrawCallback_t entryDrawCallback, entryDetailsCallback_t entryDetailsCallback);

void lcd_progressbar(uint8_t progress);
void lcd_draw_scroll_entry(uint8_t offsetY, char * buffer, uint8_t flags);

void lcd_menu_edit_setting();

bool check_heater_timeout();
bool check_preheat();

#if EXTRUDERS > 1
void lcd_select_nozzle(menuFunc_t callbackOnSelect = 0, menuFunc_t callbackOnAbort = 0);
#endif // EXTRUDERS

extern uint8_t heater_timeout;
extern uint16_t backup_temperature[EXTRUDERS];

extern const char* lcd_setting_name;
extern const char* lcd_setting_postfix;
extern void* lcd_setting_ptr;
extern uint8_t lcd_setting_type;
extern int16_t lcd_setting_min;
extern int16_t lcd_setting_max;
extern int16_t lcd_setting_start_value;
extern int16_t lcd_setting_encoder_value;

extern menuFunc_t postMenuCheck;
extern uint8_t minProgress;

extern uint16_t lineEntryPos;
extern int8_t   lineEntryWait;

void line_entry_pos_update (uint16_t maxStep, uint8_t lineEntryStep = LINE_ENTRY_STEP);
inline void line_entry_pos_reset () { lineEntryPos = lineEntryWait = 0; }
//
char* line_entry_scroll_string_transform (char* str, uint8_t lineEntryStep = LINE_ENTRY_STEP);
void  line_entry_scroll_string_restore   (char* str);
void  line_entry_fixed_string_transform  (char* str);
void  line_entry_fixed_string_restore    (char* str);
//
void lcd_lib_draw_string_scroll_center (uint8_t y, char* str, uint8_t step = LINE_ENTRY_STEP_SOFT);
void lcd_lib_draw_string_scroll_left   (uint8_t y, char* str, uint8_t step = LINE_ENTRY_STEP_SOFT);

// setting is set off if value = 0
#define LCD_SETTINGS_TYPE_OFF_BY_0   (1<<7)
// setting is set off if value is on maximum
// if LCD_SETTINGS_TYPE_OFF_BY_0 is set value is set to 0 if maximum is exceeded
#define LCD_SETTINGS_TYPE_OFF_ON_MAX (1<<6)
// bitfield of type-number
#define LCD_SETTINGS_TYPE_BITS 0x0F

#define LCD_SETTINGS_TYPE_IS_SET(var,type) (((var) & (type)) != 0)

#define LCD_SETTINGS_ENCODER_ACCELERATION 4
#define LCD_SETTINGS_ENCODER_ACCELERATION_FLOAT 32

#define LCD_EDIT_SETTING_FUNCTION(menuFunc) menu.add_menu(menu_t(menuFunc, 0, LCD_SETTINGS_ENCODER_ACCELERATION))

template <class T> inline
void LCD_EDIT_SETTING_P(T& _setting, const char* _name, const char* _postfix, int16_t _min, int16_t _max, uint8_t type = 0)
{
    menu.add_menu(menu_t(lcd_menu_edit_setting, 0, LCD_SETTINGS_ENCODER_ACCELERATION));
    lcd_setting_name = _name;
    lcd_setting_postfix = _postfix;
    lcd_setting_ptr = &_setting;
    lcd_setting_type = sizeof(_setting) | type;
    lcd_setting_start_value = lcd_setting_encoder_value = _setting;
    lcd_setting_min = _min;
    lcd_setting_max = _max;
}
template <class T> inline
void LCD_EDIT_SETTING_BYTE_PERCENT_P(T& _setting, const char* _name, const char* _postfix, int16_t _min, int16_t _max, uint8_t type = 0)
{
    menu.add_menu(menu_t(lcd_menu_edit_setting, 0, LCD_SETTINGS_ENCODER_ACCELERATION));
    lcd_setting_name = _name;
    lcd_setting_postfix = _postfix;
    lcd_setting_ptr = &_setting;
    lcd_setting_type = 5 | type;
    lcd_setting_start_value = lcd_setting_encoder_value = int(_setting) * 100 / 255;
    lcd_setting_min = _min;
    lcd_setting_max = _max;
}
template <class T> inline
void LCD_EDIT_SETTING_FLOAT001_P(T& _setting, const char* _name, const char* _postfix, float _min, float _max, uint8_t type = 0)
{
    menu.add_menu(menu_t(lcd_menu_edit_setting, 0, LCD_SETTINGS_ENCODER_ACCELERATION_FLOAT));
    lcd_setting_name = _name;
    lcd_setting_postfix = _postfix;
    lcd_setting_ptr = &_setting;
    lcd_setting_type = 3 | type;
    lcd_setting_start_value = lcd_setting_encoder_value = (_setting) * 100.0 + 0.5;
    lcd_setting_min = (_min) * 100;
    lcd_setting_max = (_max) * 100;
}
template <class T> FORCE_INLINE
void LCD_EDIT_SETTING_FLOAT1_P(T& _setting, const char* _name, const char* _postfix, float _min, float _max, uint8_t type = 0)
{
    menu.add_menu(menu_t(lcd_menu_edit_setting, 0, LCD_SETTINGS_ENCODER_ACCELERATION));
    lcd_setting_name = _name;
    lcd_setting_postfix = _postfix;
    lcd_setting_ptr = &(_setting);
    lcd_setting_type = 8 | type;
    lcd_setting_start_value = lcd_setting_encoder_value = (_setting) + 0.5;
    lcd_setting_min = (_min) + 0.5;
    lcd_setting_max = (_max) + 0.5;
}
#define LCD_EDIT_SETTING_FLOAT100(_setting, _name, _postfix, _min, _max) do { \
            menu.add_menu(menu_t(lcd_menu_edit_setting, 0, LCD_SETTINGS_ENCODER_ACCELERATION_FLOAT)); \
            lcd_setting_name = PSTR(_name); \
            lcd_setting_postfix = PSTR("00" _postfix); \
            lcd_setting_ptr = &(_setting); \
            lcd_setting_type = 7; \
            lcd_setting_start_value = lcd_setting_encoder_value = (_setting) / 100 + 0.5; \
            lcd_setting_min = (_min) / 100 + 0.5; \
            lcd_setting_max = (_max) / 100 + 0.5; \
        } while(0)
#define LCD_EDIT_SETTING_SPEED_P(_setting, _name, _postfix, _min, _max) do { \
            menu.add_menu(menu_t(lcd_menu_edit_setting, 0, LCD_SETTINGS_ENCODER_ACCELERATION)); \
            lcd_setting_name = _name; \
            lcd_setting_postfix = _postfix; \
            lcd_setting_ptr = &(_setting); \
            lcd_setting_type = 6; \
            lcd_setting_start_value = lcd_setting_encoder_value = (_setting) / 60 + 0.5; \
            lcd_setting_min = (_min) / 60 + 0.5; \
            lcd_setting_max = (_max) / 60 + 0.5; \
        } while(0)

#define LCD_EDIT_SETTING(_setting, _name, _postfix, _min, _max)              LCD_EDIT_SETTING_P             (_setting, PSTR(_name), PSTR(_postfix), _min, _max)
#define LCD_EDIT_SETTING_BYTE_PERCENT(_setting, _name, _postfix, _min, _max) LCD_EDIT_SETTING_BYTE_PERCENT_P(_setting, PSTR(_name), PSTR(_postfix), _min, _max)
#define LCD_EDIT_SETTING_FLOAT001(_setting, _name, _postfix, _min, _max)     LCD_EDIT_SETTING_FLOAT001_P    (_setting, PSTR(_name), PSTR(_postfix), _min, _max)
#define LCD_EDIT_SETTING_FLOAT1(_setting, _name, _postfix, _min, _max)       LCD_EDIT_SETTING_FLOAT1_P      (_setting, PSTR(_name), PSTR(_postfix), _min, _max)
#define LCD_EDIT_SETTING_SPEED(_setting, _name, _postfix, _min, _max)        LCD_EDIT_SETTING_SPEED_P       (_setting, PSTR(_name), PSTR(_postfix), _min, _max)

//If we have a heated bed, then the heated bed menu entries have a size of 1, else they have a size of 0.
#if TEMP_SENSOR_BED != 0
#define BED_MENU_OFFSET 1
#else
#define BED_MENU_OFFSET 0
#endif

#define BOTTOM_MENU_YPOS 54

#define EQUALF(f1, f2) (fabs(f2-f1)<=0.01f)
#define NEQUALF(f1, f2) (fabs(f2-f1)>0.01f)

#endif//ULTI_LCD2_HI_LIB_H
