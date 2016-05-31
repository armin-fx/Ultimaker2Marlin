#include <avr/pgmspace.h>

#include "Configuration.h"
#ifdef ENABLE_ULTILCD2
#include "Marlin.h"
// #include "cardreader.h"//This code uses the card.longFilename as buffer to store data, to save memory.
#include "temperature.h"
#include "machinesettings.h"
#include "UltiLCD2.h"
#include "UltiLCD2_hi_lib.h"
#include "UltiLCD2_menu_print.h"
#include "UltiLCD2_menu_material.h"
#include "UltiLCD2_menu_maintenance.h"
#include "UltiLCD2_menu_utils.h"
#include "UltiLCD2_text.h"
#include "preferences.h"
#include "helper.h"

struct materialSettings material[EXTRUDERS];
static unsigned long preheat_end_time;

void doCooldown();//TODO
static void lcd_menu_change_material_remove();
static void lcd_menu_change_material_remove_wait_user();
static void lcd_menu_change_material_remove_wait_user_ready();
static void lcd_menu_change_material_insert_wait_user();
static void lcd_menu_change_material_insert_wait_user_ready();
static void lcd_menu_change_material_insert_forward();
static void lcd_menu_change_material_insert();
static void lcd_menu_change_material_select_material();
static void lcd_menu_material_selected();
static void lcd_menu_material_settings();
static void lcd_menu_material_temperature_settings();
static void lcd_menu_material_bed_temperature_settings();
static void lcd_menu_material_settings_store();

static void cancelMaterialInsert()
{
    quickStop();
    //Set E motor power to default.
#if EXTRUDERS > 1 && defined(MOTOR_CURRENT_PWM_E_PIN) && MOTOR_CURRENT_PWM_E_PIN > -1
    digipot_current(2, active_extruder ? motor_current_e2 : motor_current_setting[2]);
#else
    digipot_current(2, motor_current_setting[2]);
#endif
    set_extrude_min_temp(EXTRUDE_MINTEMP);
}

void lcd_material_change_init(bool printing)
{
    if (!printing)
    {
        minProgress = 0;
        // move head to front
        char buffer[32];
        homeHead();
        sprintf_P(buffer, MSGP_CMD_MOVE_TO_XY, int(homing_feedrate[0]), int(AXIS_CENTER_POS(X_AXIS)), int(min_pos[Y_AXIS])+5);
        enquecommand(buffer);
        menu.add_menu(menu_t(lcd_menu_material_main_return));
    }
    preheat_end_time = millis() + (unsigned long)material[active_extruder].change_preheat_wait_time * 1000L;
}

void lcd_menu_material_main_return()
{
    doCooldown();
    homeHead();
    enquecommand_P(PSTR("M84 X Y E"));
    menu.return_to_previous(false);
}

void lcd_menu_material_main()
{
    lcd_tripple_menu(MSGP_MENU_CHANGE, MSGP_MENU_SETTINGS, MSGP_MENU_RETURN);

    if (lcd_lib_button_pressed)
    {
        if (IS_SELECTED_MAIN(0) && !is_command_queued())
        {
            lcd_material_change_init(false);
            menu.add_menu(menu_t(lcd_menu_change_material_preheat));
        }
        else if (IS_SELECTED_MAIN(1))
            menu.add_menu(menu_t(lcd_menu_material_select, SCROLL_MENU_ITEM_POS(0)));
        else if (IS_SELECTED_MAIN(2))
            menu.return_to_previous();
    }

    lcd_lib_update_screen();
}

void lcd_menu_change_material_preheat()
{
    last_user_interaction = millis();
#ifdef USE_CHANGE_TEMPERATURE
    setTargetHotend(material[active_extruder].change_temperature, active_extruder);
#else
    setTargetHotend(material[active_extruder].temperature_default, active_extruder);
#endif
    int16_t temp = degHotend(active_extruder) - 20;
    int16_t target = degTargetHotend(active_extruder) - 20;
    if (temp < 0) temp = 0;

    // draw menu
    char buffer[8];
    uint8_t progress = uint8_t(temp * 125 / target);
    if (progress < minProgress)
        progress = minProgress;
    else
        minProgress = progress;

    lcd_info_screen(lcd_change_to_previous_menu, cancelMaterialInsert);
    lcd_lib_draw_stringP(3, 10, PSTR("Heating nozzle"));
#if EXTRUDERS > 1
    strcpy_P(buffer, PSTR("("));
    int_to_string(active_extruder+1, buffer+1, PSTR(")"));
    lcd_lib_draw_string(3+(15*LCD_CHAR_SPACING), 10, buffer);
#endif
    lcd_lib_draw_stringP(3, 20, PSTR("for material removal"));

    lcd_progressbar(progress);


    // check target temp and waiting time
    if (temp > target - 5 && temp < target + 5)
    {
        if (preheat_end_time < last_user_interaction)
        {
            set_extrude_min_temp(0);

            // plan_set_e_position(0);
            // plan_buffer_line(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], end_of_print_retraction / volume_to_filament_length[active_extruder], retract_feedrate/60.0, active_extruder);

            float old_max_feedrate_e = max_feedrate[E_AXIS];
            float old_retract_acceleration = retract_acceleration;
            float old_max_e_jerk = max_e_jerk;

            max_feedrate[E_AXIS] = float(FILAMENT_FAST_STEPS) / axis_steps_per_unit[E_AXIS];
            retract_acceleration = float(FILAMENT_LONG_ACCELERATION_STEPS) / axis_steps_per_unit[E_AXIS];
            max_e_jerk = FILAMENT_LONG_MOVE_JERK;

            plan_set_e_position(0);
            plan_buffer_line(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], -1.0 / volume_to_filament_length[active_extruder], max_feedrate[E_AXIS], active_extruder);
            plan_buffer_line(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], -FILAMENT_REVERSAL_LENGTH / volume_to_filament_length[active_extruder], max_feedrate[E_AXIS], active_extruder);

            max_feedrate[E_AXIS] = old_max_feedrate_e;
            retract_acceleration = old_retract_acceleration;
            max_e_jerk = old_max_e_jerk;

            menu.replace_menu(menu_t(lcd_menu_change_material_remove), false);
            // temp = target;
            return;
        }
#ifdef USE_CHANGE_TEMPERATURE
        else if (temp > target - 3 && temp < target + 3)
        {
            // show countdown
            strcpy_P(buffer, PSTR("<"));
            int_to_string((preheat_end_time-last_user_interaction)/1000UL, buffer+1, PSTR(">"));
            lcd_lib_draw_string_center(30, buffer);
        }
#endif
    }
    else
    {
#ifdef USE_CHANGE_TEMPERATURE
        preheat_end_time = last_user_interaction + (unsigned long)material[active_extruder].change_preheat_wait_time * 1000UL;
#else
        preheat_end_time = last_user_interaction;
#endif
    }

    lcd_lib_update_screen();
}

static void lcd_menu_change_material_remove()
{
    last_user_interaction = millis();
    lcd_info_screen(lcd_change_to_previous_menu, cancelMaterialInsert);
#if EXTRUDERS > 1
    lcd_lib_draw_stringP(3, 10, MSGP_EXTRUDER);
    char buffer[8];
    strcpy_P(buffer, PSTR("("));
    int_to_string(active_extruder+1, buffer+1, PSTR(")"));
    lcd_lib_draw_string(3+(9*LCD_CHAR_SPACING), 10, buffer);
#endif
    lcd_lib_draw_stringP(3, 20, PSTR("Reversing material"));

    if (!blocks_queued())
    {
        menu.replace_menu(menu_t(lcd_menu_change_material_remove_wait_user, MAIN_MENU_ITEM_POS(0)));
        //Disable the extruder motor so you can pull out the remaining filament.
        disable_e0();
        disable_e1();
        disable_e2();
    #if EXTRUDERS > 1
        last_extruder = 0xFF;
    #endif
    }

    long pos = -st_get_position(E_AXIS);
    long targetPos = lround(FILAMENT_REVERSAL_LENGTH * axis_steps_per_unit[E_AXIS]);
    uint8_t progress = (pos * 125 / targetPos);
    lcd_progressbar(progress);

    lcd_lib_update_screen();
}

static void lcd_menu_change_material_remove_wait_user_ready()
{
    plan_set_e_position(0);
    menu.replace_menu(menu_t(lcd_menu_change_material_insert_wait_user, MAIN_MENU_ITEM_POS(0)));
    check_preheat();
}

static void lcd_change_to_material_main()
{
    // return to material main menu
    lcd_lib_keyclick();
    for (uint8_t i = 0; i < 2; ++i)
    {
        menu.return_to_previous();
    }
}

static void lcd_menu_change_material_remove_wait_user()
{
    LED_GLOW
    setTargetHotend(material[active_extruder].temperature_default, active_extruder);
    lcd_question_screen(NULL, lcd_menu_change_material_remove_wait_user_ready, MSGP_MENU_READY, lcd_change_to_material_main, cancelMaterialInsert, MSGP_MENU_CANCEL);
#if EXTRUDERS > 1
    lcd_lib_draw_stringP(3, 10, MSGP_EXTRUDER);
    char buffer[8];
    strcpy_P(buffer, PSTR("("));
    int_to_string(active_extruder+1, buffer+1, PSTR(")"));
    lcd_lib_draw_string(3+(9*LCD_CHAR_SPACING), 10, buffer);
    lcd_lib_draw_stringP(3, 20, PSTR("Remove material"));
#else
    lcd_lib_draw_string_centerP(20, PSTR("Remove material"));
#endif
    lcd_lib_update_screen();
}

void lcd_menu_insert_material_preheat()
{
    last_user_interaction = millis();
    setTargetHotend(material[active_extruder].temperature_default, active_extruder);
    int16_t temp = degHotend(active_extruder) - 20;
    int16_t target = degTargetHotend(active_extruder) - 20 - 10;
    if (temp < 0) temp = 0;
    if (temp > target && !is_command_queued())
    {
        set_extrude_min_temp(0);
        for(uint8_t e=0; e<EXTRUDERS; e++)
            volume_to_filament_length[e] = 1.0;//Set the extrusion to 1mm per given value, so we can move the filament a set distance.

        menu.replace_menu(menu_t(lcd_menu_change_material_insert_wait_user, MAIN_MENU_ITEM_POS(0)));
        temp = target;
    }

    uint8_t progress = uint8_t(temp * 125 / target);
    if (progress < minProgress)
        progress = minProgress;
    else
        minProgress = progress;

    lcd_info_screen(lcd_change_to_previous_menu, cancelMaterialInsert);
#if EXTRUDERS > 1
    lcd_lib_draw_stringP(3, 10, PSTR("Heating nozzle"));
    char buffer[8];
    strcpy_P(buffer, PSTR("("));
    int_to_string(active_extruder+1, buffer+1, PSTR(")"));
    lcd_lib_draw_string(3+(15*LCD_CHAR_SPACING), 10, buffer);
    lcd_lib_draw_stringP(3, 20, PSTR("for insertion"));
#else
    lcd_lib_draw_stringP(3, 10, PSTR("Heating nozzle for"));
    lcd_lib_draw_stringP(3, 20, PSTR("material insertion"));
#endif

    lcd_progressbar(progress);

    lcd_lib_update_screen();
}

static void lcd_menu_change_material_insert_wait_user()
{
    LED_GLOW

    if (target_temperature[active_extruder] && (printing_state == PRINT_STATE_NORMAL) && (movesplanned() < 2))
    {
        plan_set_e_position(0);
        plan_buffer_line(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], 0.5 / volume_to_filament_length[active_extruder], FILAMENT_INSERT_SPEED, active_extruder);
    }

    lcd_question_screen(NULL, lcd_menu_change_material_insert_wait_user_ready, MSGP_MENU_READY, lcd_change_to_previous_menu, cancelMaterialInsert, MSGP_MENU_CANCEL);
#if EXTRUDERS > 1
    lcd_lib_draw_stringP(3, 10, PSTR("Insert new material"));
    lcd_lib_draw_stringP(3, 20, PSTR("for extruder"));
    char buffer[8];
    strcpy_P(buffer, PSTR("("));
    int_to_string(active_extruder+1, buffer+1, PSTR(")"));
    lcd_lib_draw_string(3+(13*LCD_CHAR_SPACING), 20, buffer);
    lcd_lib_draw_stringP(3, 30, PSTR("from the backside of"));
    lcd_lib_draw_stringP(3, 40, PSTR("your machine"));
#else
    lcd_lib_draw_string_centerP(10, PSTR("Insert new material"));
    lcd_lib_draw_string_centerP(20, PSTR("from the backside of"));
    lcd_lib_draw_string_centerP(30, PSTR("your machine,"));
    lcd_lib_draw_string_centerP(40, PSTR("above the arrow."));
#endif
    lcd_lib_update_screen();

}

static void lcd_menu_change_material_insert_wait_user_ready()
{
    // heat up nozzle (if necessary)
    if (!check_preheat())
    {
        return;
    }

    //Override the max feedrate and acceleration values to get a better insert speed and speedup/slowdown
    float old_max_feedrate_e = max_feedrate[E_AXIS];
    float old_retract_acceleration = retract_acceleration;
    float old_max_e_jerk = max_e_jerk;

    max_feedrate[E_AXIS] = float(FILAMENT_FAST_STEPS) / axis_steps_per_unit[E_AXIS];
    retract_acceleration = float(FILAMENT_LONG_ACCELERATION_STEPS) / axis_steps_per_unit[E_AXIS];
    max_e_jerk = FILAMENT_LONG_MOVE_JERK;

    plan_set_e_position(0);
    plan_buffer_line(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], FILAMENT_FORWARD_LENGTH / volume_to_filament_length[active_extruder], max_feedrate[E_AXIS], active_extruder);

    //Put back origonal values.
    max_feedrate[E_AXIS] = old_max_feedrate_e;
    retract_acceleration = old_retract_acceleration;
    max_e_jerk = old_max_e_jerk;

    menu.replace_menu(menu_t(lcd_menu_change_material_insert_forward));
}

static void lcd_menu_change_material_insert_forward()
{
    last_user_interaction = millis();
    lcd_info_screen(lcd_change_to_previous_menu, cancelMaterialInsert);
#if EXTRUDERS > 1
    lcd_lib_draw_stringP(3, 10, MSGP_EXTRUDER);
    char buffer[8];
    strcpy_P(buffer, PSTR("("));
    int_to_string(active_extruder+1, buffer+1, PSTR(")"));
    lcd_lib_draw_string(3+(9*LCD_CHAR_SPACING), 10, buffer);
#endif
    lcd_lib_draw_stringP(3, 20, PSTR("Forwarding material"));

    if (!blocks_queued())
    {
        lcd_lib_keyclick();
        // led_glow_dir = led_glow = 0;

        //Set the E motor power lower to we skip instead of grind.
#if EXTRUDERS > 1 && defined(MOTOR_CURRENT_PWM_E_PIN) && MOTOR_CURRENT_PWM_E_PIN > -1
        digipot_current(2, active_extruder ? (motor_current_e2*2/3) : (motor_current_setting[2]*2/3));
#else
        digipot_current(2, motor_current_setting[2]*2/3);
#endif
        menu.replace_menu(menu_t(lcd_menu_change_material_insert, MAIN_MENU_ITEM_POS(0)));
    }

    long pos = st_get_position(E_AXIS);
    long targetPos = lround(FILAMENT_FORWARD_LENGTH*axis_steps_per_unit[E_AXIS]);
    uint8_t progress = (pos * 125 / targetPos);
    lcd_progressbar(progress);

    lcd_lib_update_screen();
}

static void materialInsertReady()
{
    plan_set_e_position(0);
    plan_buffer_line(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], (-end_of_print_retraction-retract_length) / volume_to_filament_length[active_extruder], 25*60, active_extruder);
    //Set E motor power to default.
#if EXTRUDERS > 1 && defined(MOTOR_CURRENT_PWM_E_PIN) && MOTOR_CURRENT_PWM_E_PIN > -1
    digipot_current(2, active_extruder ? motor_current_e2 : motor_current_setting[2]);
#else
    digipot_current(2, motor_current_setting[2]);
#endif
    lcd_remove_menu();
    if (!card.sdprinting)
    {
        // cool down nozzle
        for(uint8_t n=0; n<EXTRUDERS; n++)
        {
            setTargetHotend(0, n);
        }
    }
}

static void lcd_menu_change_material_insert()
{
    if (target_temperature[active_extruder])
    {
        LED_GLOW

        lcd_question_screen(lcd_menu_change_material_select_material, materialInsertReady, MSGP_MENU_READY, lcd_change_to_previous_menu, cancelMaterialInsert, MSGP_MENU_CANCEL);
#if EXTRUDERS > 1
        lcd_lib_draw_stringP(3, 20, PSTR("Wait till material"));
        lcd_lib_draw_stringP(3, 30, PSTR("comes out nozzle"));
        char buffer[8];
        strcpy_P(buffer, PSTR("("));
        int_to_string(active_extruder+1, buffer+1, PSTR(")"));
        lcd_lib_draw_string(3+(17*LCD_CHAR_SPACING), 30, buffer);
#else
        lcd_lib_draw_string_centerP(20, PSTR("Wait till material"));
        lcd_lib_draw_string_centerP(30, PSTR("comes out the nozzle"));
#endif

        if (movesplanned() < 2)
        {
            plan_set_e_position(0);
            plan_buffer_line(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], 0.5 / volume_to_filament_length[active_extruder], FILAMENT_INSERT_EXTRUDE_SPEED, active_extruder);
        }
        lcd_lib_update_screen();
    }
    else
    {
        materialInsertReady();
        menu.replace_menu(menu_t(lcd_menu_change_material_select_material));
    }
}

static void lcd_menu_change_material_select_material_callback(uint8_t nr, uint8_t offsetY, uint8_t flags)
{
    char buffer[10];
    eeprom_read_block(buffer, EEPROM_MATERIAL_NAME_OFFSET(nr), MATERIAL_NAME_SIZE);
    buffer[MATERIAL_NAME_SIZE] = '\0';
    lcd_draw_scroll_entry(offsetY, buffer, flags);
}

static void lcd_menu_change_material_select_material_details_callback(uint8_t nr)
{
    char buffer[32]; buffer[0] = '\0';
    char* c = buffer;

    if (led_glow_dir)
    {
        c = float_to_string2(eeprom_read_float(EEPROM_MATERIAL_DIAMETER_OFFSET(nr)), c, MSGP_UNIT_MM);
        while(c < buffer + 10) *c++ = ' ';
        strcpy_P(c, PSTR("Flow:"));
        c += 5;
        c = int_to_string(eeprom_read_word(EEPROM_MATERIAL_FLOW_OFFSET(nr)), c, MSGP_UNIT_PERCENT);
    }else{
        c = int_to_string(eeprom_read_word(EEPROM_MATERIAL_TEMPERATURE_OFFSET(nr)), c, MSGP_UNIT_CELSIUS);
#if TEMP_SENSOR_BED != 0
        *c++ = ' ';
        c = int_to_string(eeprom_read_word(EEPROM_MATERIAL_BED_TEMPERATURE_OFFSET(nr)), c, MSGP_UNIT_CELSIUS);
#endif
        while(c < buffer + 10) *c++ = ' ';
        strcpy_P(c, PSTR("Fan: "));
        c += 5;
        c = int_to_string(eeprom_read_byte(EEPROM_MATERIAL_FAN_SPEED_OFFSET(nr)), c, MSGP_UNIT_PERCENT);
    }
    lcd_lib_draw_string_left(BOTTOM_MENU_YPOS, buffer);
}

static void lcd_menu_change_material_select_material()
{
    uint8_t count = eeprom_read_byte(EEPROM_MATERIAL_COUNT_OFFSET());

    lcd_scroll_menu(MSGP_MENU_MATERIAL, count, lcd_menu_change_material_select_material_callback, lcd_menu_change_material_select_material_details_callback);
    if (lcd_lib_button_pressed)
    {
        lcd_material_set_material(SELECTED_SCROLL_MENU_ITEM(), active_extruder);

        menu.replace_menu(menu_t(lcd_menu_material_selected, MAIN_MENU_ITEM_POS(0)));
    }
    lcd_lib_update_screen();
}

static void lcd_menu_material_export_done()
{
    lcd_lib_encoder_pos = MAIN_MENU_ITEM_POS(0);
    lcd_info_screen(NULL, lcd_change_to_previous_menu, PSTR("Ok"));
    lcd_lib_draw_string_centerP(20, PSTR("Saved materials"));
    lcd_lib_draw_string_centerP(30, PSTR("to the SD card"));
    lcd_lib_draw_string_centerP(40, PSTR("in MATERIAL.TXT"));
    lcd_lib_update_screen();
}


static void lcd_menu_material_export()
{
    char buffer[32];
    
    if (!card.sdInserted)
    {
        lcd_menu_no_sdcard();
        return;
    }
    if (!card.isOk())
    {
        lcd_menu_reading_card();
        return;
    }

    card.setroot();
    strcpy_P(buffer, MSGP_STORE_FILENAME);
    card.openFile(buffer, false);
    uint8_t count = eeprom_read_byte(EEPROM_MATERIAL_COUNT_OFFSET());
    for(uint8_t n=0; n<count; ++n)
    {
        strcpy_2P(buffer, MSGP_STORE_ENTRY_MATERIAL, MSGP_NEWLINE);
        card.write_string(buffer);

        strcpy_2P(buffer, MSGP_STORE_NAME, MSGP_EQUAL);
        char* ptr = buffer + strlen(buffer);
        eeprom_read_block(ptr, EEPROM_MATERIAL_NAME_OFFSET(n), MATERIAL_NAME_SIZE);
        ptr[MATERIAL_NAME_SIZE] = '\0';
        strcat_P(buffer, MSGP_NEWLINE);
        card.write_string(buffer);

        strcpy_2P(buffer, MSGP_STORE_TEMPERATURE, MSGP_EQUAL);
        ptr = buffer + strlen(buffer);
        int_to_string(eeprom_read_word(EEPROM_MATERIAL_TEMPERATURE_OFFSET(n)), ptr, MSGP_NEWLINE);
        card.write_string(buffer);

        for(uint8_t nozzle=0; nozzle<MATERIAL_TEMPERATURE_COUNT; ++nozzle)
        {
            strcpy_2P(buffer, MSGP_STORE_TEMPERATURE, MSGP_UNDERLINE);
            ptr = float_to_string2(nozzleIndexToNozzleSize(nozzle), buffer + strlen(buffer), MSGP_EQUAL);
            if (eeprom_read_word(EEPROM_MATERIAL_EXTRA_TEMPERATURE_OFFSET(n, nozzle)) != 0)
            {
                int_to_string(eeprom_read_word(EEPROM_MATERIAL_EXTRA_TEMPERATURE_OFFSET(n, nozzle)), ptr, MSGP_NEWLINE);
            }
            else // write default temperature to keep compatible with TinkerGnome version
            {
                int_to_string(eeprom_read_word(EEPROM_MATERIAL_TEMPERATURE_OFFSET(n)), ptr, PSTR(" off\n"));
            }
            card.write_string(buffer);
        }

#if TEMP_SENSOR_BED != 0
        strcpy_2P(buffer, MSGP_STORE_TEMPERATURE_BED, MSGP_EQUAL);
        ptr = buffer + strlen(buffer);
        int_to_string(eeprom_read_word(EEPROM_MATERIAL_BED_TEMPERATURE_OFFSET(n)), ptr, MSGP_NEWLINE);
        card.write_string(buffer);

        strcpy_2P(buffer, MSGP_STORE_TEMPERATURE_BED_FIRST_LAYER, MSGP_EQUAL);
        ptr = buffer + strlen(buffer);
        int_to_string(eeprom_read_word(EEPROM_MATERIAL_BED_TEMPERATURE_FIRST_LAYER(n)), ptr, MSGP_NEWLINE);
        card.write_string(buffer);
#endif

        strcpy_2P(buffer, MSGP_STORE_FAN_SPEED, MSGP_EQUAL);
        ptr = buffer + strlen(buffer);
        int_to_string(eeprom_read_byte(EEPROM_MATERIAL_FAN_SPEED_OFFSET(n)), ptr, MSGP_NEWLINE);
        card.write_string(buffer);

        strcpy_2P(buffer, MSGP_STORE_FLOW, MSGP_EQUAL);
        ptr = buffer + strlen(buffer);
        int_to_string(eeprom_read_word(EEPROM_MATERIAL_FLOW_OFFSET(n)), ptr, MSGP_NEWLINE);
        card.write_string(buffer);

        strcpy_2P(buffer, MSGP_STORE_DIAMETER, MSGP_EQUAL);
        ptr = buffer + strlen(buffer);
        float_to_string2(eeprom_read_float(EEPROM_MATERIAL_DIAMETER_OFFSET(n)), ptr, MSGP_NEWLINE);
        card.write_string(buffer);

#ifdef USE_CHANGE_TEMPERATURE
        strcpy_2P(buffer, MSGP_STORE_CHANGE_TEMPERATURE, MSGP_EQUAL);
        ptr = buffer + strlen(buffer);
        float_to_string2(eeprom_read_word(EEPROM_MATERIAL_CHANGE_TEMPERATURE(n)), ptr, MSGP_NEWLINE);
        card.write_string(buffer);

        strcpy_2P(buffer, MSGP_STORE_CHANGE_WAIT, MSGP_EQUAL);
        ptr = buffer + strlen(buffer);
        float_to_string2(eeprom_read_byte(EEPROM_MATERIAL_CHANGE_WAIT_TIME(n)), ptr, MSGP_NEWLINE);
        card.write_string(buffer);
#endif
        strcpy_P(buffer, MSGP_NEWLINE);
        card.write_string(buffer);
    }
    card.closefile();
    menu.replace_menu(menu_t(lcd_menu_material_export_done));
}

static void lcd_menu_material_import_done()
{
    lcd_lib_encoder_pos = MAIN_MENU_ITEM_POS(0);
    lcd_info_screen(NULL, lcd_change_to_previous_menu, PSTR("Ok"));
    lcd_lib_draw_string_centerP(20, PSTR("Loaded materials"));
    lcd_lib_draw_string_centerP(30, PSTR("from the SD card"));
    lcd_lib_update_screen();
}

static void lcd_menu_material_reset()
{
    lcd_material_reset_defaults();
    lcd_lib_encoder_pos = MAIN_MENU_ITEM_POS(0);
    lcd_info_screen(NULL, lcd_change_to_previous_menu, PSTR("Ok"));
    lcd_lib_draw_string_centerP(20, PSTR("Reset materials"));
    lcd_lib_draw_string_centerP(30, PSTR("to factory default"));
    lcd_lib_update_screen();
}

void materialSettingsEEPROM_clear (materialSettingsEEPROM &m)
{
    m.temperature_default = 0;
    for (uint8_t i=0; i<MAX_MATERIAL_TEMPERATURES; ++i) m.temperature[i] = 0;
    m.bed_temperature             = 0;
    m.bed_temperature_first_layer = 0;
    m.fan_speed = 100;
    m.flow      = 100;
    m.diameter  = DEFAULT_FILAMENT_DIAMETER;
    m.name[0]   = '\0';
    m.change_temperature       = 0;
    m.change_preheat_wait_time = 0;
}

void materialSettingsEEPROM_repair (materialSettingsEEPROM &m)
{
    cut_scope (m.temperature_default, 0, HEATER_0_MAXTEMP - 15);
    for (uint8_t i=0; i<MATERIAL_TEMPERATURE_COUNT; ++i)
    {
        cut_scope (m.temperature[i], 0, HEATER_0_MAXTEMP - 15);
        //if (m.temperature[i] == 0) 
        //    m.temperature[i] = m.temperature_default;
    }
#if TEMP_SENSOR_BED != 0
    cut_scope (m.bed_temperature,             0, BED_MAXTEMP - 15);
    cut_scope (m.bed_temperature_first_layer, 0, BED_MAXTEMP - 15);
#else
    cut_min (m.bed_temperature,             0);
    cut_min (m.bed_temperature_first_layer, 0);
#endif
    cut_max   (m.fan_speed, 100);
    cut_scope (m.flow     , 0,  1000);
    cut_scope (m.diameter , 0.1f,  10.0f);
    m.name[MATERIAL_NAME_SIZE] = '\0';
    cut_scope (m.change_temperature, 0, HEATER_0_MAXTEMP - 15);
    cut_min (m.change_preheat_wait_time, 0);
}

void materialSettingsEEPROM_store (materialSettingsEEPROM &m, uint8_t nr)
{
    eeprom_update_block(m.name, EEPROM_MATERIAL_NAME_OFFSET(nr), MATERIAL_NAME_SIZE);
    eeprom_update_word (EEPROM_MATERIAL_TEMPERATURE_OFFSET(nr), m.temperature_default);
    for(uint8_t i=0; i<MATERIAL_TEMPERATURE_COUNT; ++i)
        eeprom_update_word(EEPROM_MATERIAL_EXTRA_TEMPERATURE_OFFSET(nr, i), m.temperature[i]);
    eeprom_update_word (EEPROM_MATERIAL_BED_TEMPERATURE_OFFSET(nr),      m.bed_temperature);
    eeprom_update_word (EEPROM_MATERIAL_BED_TEMPERATURE_FIRST_LAYER(nr), m.bed_temperature_first_layer);
    eeprom_update_byte (EEPROM_MATERIAL_FAN_SPEED_OFFSET(nr),            m.fan_speed);
    eeprom_update_word (EEPROM_MATERIAL_FLOW_OFFSET(nr),                 m.flow);
    eeprom_update_float(EEPROM_MATERIAL_DIAMETER_OFFSET(nr),             m.diameter);
#ifdef USE_CHANGE_TEMPERATURE
    eeprom_update_word (EEPROM_MATERIAL_CHANGE_TEMPERATURE(nr), m.change_temperature);
    eeprom_update_byte (EEPROM_MATERIAL_CHANGE_WAIT_TIME(nr),   m.change_preheat_wait_time);
#endif
}


static void lcd_menu_material_import()
{
    char buffer[32];

    if (!card.sdInserted)
    {
        lcd_menu_no_sdcard();
        return;
    }
    if (!card.isOk())
    {
        lcd_menu_reading_card();
        return;
    }

    card.setroot();
    strcpy_P(buffer, MSGP_STORE_FILENAME);
    card.openFile(buffer, true);
    if (!card.isFileOpen())
    {
        lcd_info_screen(NULL, lcd_change_to_previous_menu);
        lcd_lib_draw_string_centerP(15, PSTR("No export file"));
        lcd_lib_draw_string_centerP(25, PSTR("Found on card."));
        lcd_lib_update_screen();
        return;
    }

    uint8_t count = 0xFF;
    materialSettingsEEPROM materialEEPROM;
    while(card.fgets(buffer, sizeof(buffer)) > 0)
    {
        buffer[sizeof(buffer)-1] = '\0';
        char* c = strchr(buffer, '\n');
        if (c) *c = '\0';

        if(strcmp_P(buffer, MSGP_STORE_ENTRY_MATERIAL) == 0)
        {
            if (count != 0xFF)
            { // verify and store imported material settings
                materialSettingsEEPROM_repair(materialEEPROM);
                materialSettingsEEPROM_store (materialEEPROM, count);
            }
            // reset material settings for next material entry
            materialSettingsEEPROM_clear(materialEEPROM);
            count++;
        }else if (count < EEPROM_MATERIAL_SETTINGS_MAX_COUNT)
        {
            c = strchr(buffer, '=');
            if (c)
            {
                *c++ = '\0';
                if (strcmp_P(buffer, MSGP_STORE_NAME) == 0)
                {
                    strncpy(materialEEPROM.name, c, MATERIAL_NAME_SIZE);
                    materialEEPROM.name[MATERIAL_NAME_SIZE] = '\0';
                }else if (strcmp_P(buffer, MSGP_STORE_TEMPERATURE) == 0)
                {
                    materialEEPROM.temperature_default = strtol(c, NULL, 10);
                }else if (strcmp_P(buffer, MSGP_STORE_TEMPERATURE_BED) == 0)
                {
                    materialEEPROM.bed_temperature = strtol(c, NULL, 10);
                }else if (strcmp_P(buffer, MSGP_STORE_TEMPERATURE_BED_FIRST_LAYER) == 0)
                {
                    materialEEPROM.bed_temperature_first_layer = strtol(c, NULL, 10);
                }else if (strcmp_P(buffer, MSGP_STORE_FAN_SPEED) == 0)
                {
                    materialEEPROM.fan_speed = strtol(c, NULL, 10);
                }else if (strcmp_P(buffer, MSGP_STORE_FLOW) == 0)
                {
                    materialEEPROM.flow = strtol(c, NULL, 10);
                }else if (strcmp_P(buffer, MSGP_STORE_DIAMETER) == 0)
                {
                    materialEEPROM.diameter = strtod(c, NULL);
#ifdef USE_CHANGE_TEMPERATURE
                }else if (strcmp_P(buffer, MSGP_STORE_CHANGE_TEMPERATURE) == 0)
                {
                    materialEEPROM.change_temperature = strtol(c, NULL, 10);
                }else if (strcmp_P(buffer, MSGP_STORE_CHANGE_WAIT) == 0)
                {
                    materialEEPROM.change_preheat_wait_time = strtol(c, NULL, 10);
#endif
                }
                for(uint8_t nozzle=0; nozzle<MATERIAL_TEMPERATURE_COUNT; ++nozzle)
                {
                    char buffer2[32];
                    strcpy_2P(buffer2, MSGP_STORE_TEMPERATURE, MSGP_UNDERLINE);
                    char* ptr = buffer2 + strlen(buffer2);
                    float_to_string2(nozzleIndexToNozzleSize(nozzle), ptr);
                    if (strcmp(buffer, buffer2) == 0)
                    {
                        materialEEPROM.temperature[nozzle] = strtol(c, NULL, 10);

                        ptr = strchr(c, 'o');
                        if (ptr != NULL) if (strcmp_P(ptr, PSTR("off")) == 0)
                            materialEEPROM.temperature[nozzle] = 0;
                    }
                }
            }
        }
    }
    if (count != 0xFF)
    {
        // verify and store imported material settings - last entry
        materialSettingsEEPROM_repair(materialEEPROM);
        materialSettingsEEPROM_store (materialEEPROM, count);
        // update materials count
        eeprom_update_byte(EEPROM_MATERIAL_COUNT_OFFSET(), count + 1);
    }
    card.closefile();

    menu.replace_menu(menu_t(lcd_menu_material_import_done));
}

static void lcd_material_select_callback(uint8_t nr, uint8_t offsetY, uint8_t flags)
{
    uint8_t count = eeprom_read_byte(EEPROM_MATERIAL_COUNT_OFFSET());
    char buffer[32];
    if (nr == 0)
        strcpy_P(buffer, MSGP_ENTRY_RETURN);
    else if (nr == count + 1)
        strcpy_P(buffer, PSTR("Customize"));
    else if (nr == count + 2)
        strcpy_P(buffer, PSTR("Export to SD"));
    else if (nr == count + 3)
        strcpy_P(buffer, PSTR("Import from SD"));
    else if (nr == count + 4)
        strcpy_P(buffer, PSTR("Reset to default"));
    else{
        eeprom_read_block(buffer, EEPROM_MATERIAL_NAME_OFFSET(nr - 1), MATERIAL_NAME_SIZE);
        buffer[MATERIAL_NAME_SIZE] = '\0';
    }
    lcd_draw_scroll_entry(offsetY, buffer, flags);
}

static void lcd_material_select_details_callback(uint8_t nr)
{
    uint8_t count = eeprom_read_byte(EEPROM_MATERIAL_COUNT_OFFSET());
    if (nr == 0)
    {

    }
    else if (nr <= count)
    {
        char buffer[32]; buffer[0] = '\0';
        char* c = buffer;
        nr -= 1;

        if (led_glow_dir)
        {
            c = float_to_string2(eeprom_read_float(EEPROM_MATERIAL_DIAMETER_OFFSET(nr)), c, MSGP_UNIT_MM);
            while(c < buffer + 10) *c++ = ' ';
            strcpy_P(c, PSTR("Flow:"));
            c += 5;
            c = int_to_string(eeprom_read_word(EEPROM_MATERIAL_FLOW_OFFSET(nr)), c, MSGP_UNIT_PERCENT);
        }else{
            c = int_to_string(eeprom_read_word(EEPROM_MATERIAL_TEMPERATURE_OFFSET(nr)), c, MSGP_UNIT_CELSIUS);
#if TEMP_SENSOR_BED != 0
            *c++ = ' ';
            c = int_to_string(eeprom_read_word(EEPROM_MATERIAL_BED_TEMPERATURE_OFFSET(nr)), c, MSGP_UNIT_CELSIUS);
#endif
            while(c < buffer + 10) *c++ = ' ';
            strcpy_P(c, PSTR("Fan: "));
            c += 5;
            c = int_to_string(eeprom_read_byte(EEPROM_MATERIAL_FAN_SPEED_OFFSET(nr)), c, MSGP_UNIT_PERCENT);
        }
        lcd_lib_draw_string_left(BOTTOM_MENU_YPOS, buffer);
    }else if (nr == count + 1)
    {
        lcd_lib_draw_string_centerP(BOTTOM_MENU_YPOS, PSTR("Modify the settings"));
    }else if (nr == count + 2)
    {
        lcd_lib_draw_string_centerP(BOTTOM_MENU_YPOS, PSTR("Saves all materials"));
    }else if (nr == count + 3)
    {
        lcd_lib_draw_string_centerP(BOTTOM_MENU_YPOS, PSTR("Loads all materials"));
    }else if (nr == count + 4)
    {
        lcd_lib_draw_string_centerP(BOTTOM_MENU_YPOS, PSTR("Clear all materials"));
    }
}

void lcd_menu_material_select()
{
    uint8_t count = eeprom_read_byte(EEPROM_MATERIAL_COUNT_OFFSET());

    lcd_scroll_menu(MSGP_MENU_MATERIAL, count + 5, lcd_material_select_callback, lcd_material_select_details_callback);
    if (lcd_lib_button_pressed)
    {
        if (IS_SELECTED_SCROLL(0))
            menu.return_to_previous();
        else if (IS_SELECTED_SCROLL(count + 1))
            menu.add_menu(menu_t(lcd_menu_material_settings));
        else if (IS_SELECTED_SCROLL(count + 2))
            menu.add_menu(menu_t(lcd_menu_material_export));
        else if (IS_SELECTED_SCROLL(count + 3))
            menu.add_menu(menu_t(lcd_menu_material_import));
        else if (IS_SELECTED_SCROLL(count + 4))
            menu.add_menu(menu_t(lcd_menu_material_reset));
        else{
            lcd_material_set_material(SELECTED_SCROLL_MENU_ITEM() - 1, active_extruder);
            menu.replace_menu(menu_t(lcd_menu_material_selected, MAIN_MENU_ITEM_POS(0)));
        }
    }
    lcd_lib_update_screen();
}

static void lcd_menu_material_selected()
{
    lcd_info_screen(NULL, lcd_change_to_previous_menu, PSTR("OK"));
    lcd_lib_draw_string_centerP(20, PSTR("Selected material:"));
    lcd_lib_draw_string_center(30, LCD_CACHE_FILENAME(0));
#if EXTRUDERS > 1
    if (active_extruder == 0)
        lcd_lib_draw_string_centerP(40, PSTR("for extruder 1"));
    else if (active_extruder == 1)
        lcd_lib_draw_string_centerP(40, PSTR("for extruder 2"));
#endif
    lcd_lib_update_screen();
}

static void lcd_material_settings_callback(uint8_t nr, uint8_t offsetY, uint8_t flags)
{
    char buffer[32];
    if (nr == 0)
        strcpy_P(buffer, MSGP_ENTRY_RETURN);
    else if (nr == 1)
        strcpy_P(buffer, MSGP_TEMPERATURE);
#if TEMP_SENSOR_BED != 0
    else if (nr == 2)
        strcpy_P(buffer, MSGP_HEATED_BUILDPLATE);
#endif
    else if (nr == 2 + BED_MENU_OFFSET)
        strcpy_P(buffer, PSTR("Diameter"));
    else if (nr == 3 + BED_MENU_OFFSET)
        strcpy_P(buffer, PSTR("Fan"));
    else if (nr == 4 + BED_MENU_OFFSET)
        strcpy_P(buffer, PSTR("Flow %"));
#ifdef USE_CHANGE_TEMPERATURE
    else if (nr == 5 + BED_MENU_OFFSET)
        strcpy_P(buffer, PSTR("Change temperature"));
    else if (nr == 6 + BED_MENU_OFFSET)
        strcpy_P(buffer, PSTR("Change wait time"));
    else if (nr == 7 + BED_MENU_OFFSET)
        strcpy_P(buffer, PSTR("Store as preset"));
#else
    else if (nr == 5 + BED_MENU_OFFSET)
        strcpy_P(buffer, PSTR("Store as preset"));
#endif
    else
        strcpy_P(buffer, MSGP_ENTRY_UNKNOWN);

    lcd_draw_scroll_entry(offsetY, buffer, flags);
}

static void lcd_material_settings_details_callback(uint8_t nr)
{
    char buffer[21];
    buffer[0] = '\0';
    if (nr == 0)
    {
        return;
    }else if (nr == 1)
    {
        uint8_t count = 0;
        for(uint8_t n=0; n<MATERIAL_TEMPERATURE_COUNT; ++n)
            if (material[active_extruder].temperature[n] != 0)
                ++count;
        if (count == 0)
        {
            int_to_string(material[active_extruder].temperature_default, buffer, MSGP_UNIT_CELSIUS);
        }
        else
        {
            char* c = buffer;
            c = int_to_string(material[active_extruder].temperature_default, c, PSTR("C ("));

            count = (millis() / 1500L) % count;
            for(uint8_t n=0; n<MATERIAL_TEMPERATURE_COUNT; ++n)
            {
                if (count-- == 0)
                {
                    c = float_to_string2(nozzleIndexToNozzleSize(n), c, PSTR("mm: "));
                    c = int_to_string(material[active_extruder].temperature[n], c, PSTR("C)"));
                    break;
                }
            }
        }
#if TEMP_SENSOR_BED != 0
    }else if (nr == 2)
    {
        if (material[active_extruder].bed_temperature_first_layer == 0)
        {
            int_to_string(material[active_extruder].bed_temperature, buffer, MSGP_UNIT_CELSIUS);
        }
        else
        {
            int_to_string(material[active_extruder].bed_temperature, buffer, PSTR("C ("));
            int_to_string(material[active_extruder].bed_temperature_first_layer, buffer+strlen(buffer), PSTR("C)"));
        }
#endif
    }else if (nr == 2 + BED_MENU_OFFSET)
    {
        float_to_string2(material[active_extruder].diameter, buffer, MSGP_UNIT_MM);
    }else if (nr == 3 + BED_MENU_OFFSET)
    {
        int_to_string(material[active_extruder].fan_speed, buffer, MSGP_UNIT_PERCENT);
    }else if (nr == 4 + BED_MENU_OFFSET)
    {
        int_to_string(material[active_extruder].flow, buffer, MSGP_UNIT_PERCENT);
#ifdef USE_CHANGE_TEMPERATURE
    }else if (nr == 5 + BED_MENU_OFFSET)
    {
        int_to_string(material[active_extruder].change_temperature, buffer, MSGP_UNIT_CELSIUS);
    }else if (nr == 6 + BED_MENU_OFFSET)
    {
        int_to_string(material[active_extruder].change_preheat_wait_time, buffer, PSTR("Sec"));
#endif
    }
    lcd_lib_draw_string_left(BOTTOM_MENU_YPOS, buffer);
}

static void lcd_menu_material_settings()
{
#ifdef USE_CHANGE_TEMPERATURE
    lcd_scroll_menu(MSGP_MENU_MATERIAL, 8 + BED_MENU_OFFSET, lcd_material_settings_callback, lcd_material_settings_details_callback);
#else
    lcd_scroll_menu(MSGP_MENU_MATERIAL, 6 + BED_MENU_OFFSET, lcd_material_settings_callback, lcd_material_settings_details_callback);
#endif
    if (lcd_lib_button_pressed)
    {
        if (IS_SELECTED_SCROLL(0))
        {
            lcd_change_to_previous_menu();
            lcd_material_store_current_material();
        }else if (IS_SELECTED_SCROLL(1))
        {
            //LCD_EDIT_SETTING(material[active_extruder].temperature[0], "Temperature", "C", 0, HEATER_0_MAXTEMP - 15);
            menu.add_menu(menu_t(lcd_menu_material_temperature_settings));
        }
#if TEMP_SENSOR_BED != 0
        else if (IS_SELECTED_SCROLL(2))
        {
            //LCD_EDIT_SETTING_P(material[active_extruder].bed_temperature, MSGP_BUILDPLATE_TEMPERATURE, MSGP_UNIT_CELSIUS, 0, BED_MAXTEMP - 15);
            menu.add_menu(menu_t(lcd_menu_material_bed_temperature_settings));
        }
#endif
        else if (IS_SELECTED_SCROLL(2 + BED_MENU_OFFSET))
            LCD_EDIT_SETTING_FLOAT001_P(material[active_extruder].diameter, PSTR("Material Diameter"), MSGP_UNIT_MM, 0, 100);
        else if (IS_SELECTED_SCROLL(3 + BED_MENU_OFFSET))
            LCD_EDIT_SETTING_P(material[active_extruder].fan_speed, MSGP_FAN_SPEED, MSGP_UNIT_PERCENT, 0, 100);
        else if (IS_SELECTED_SCROLL(4 + BED_MENU_OFFSET))
            LCD_EDIT_SETTING_P(material[active_extruder].flow, MSGP_MATERIAL_FLOW, MSGP_UNIT_PERCENT, 1, 1000);
#ifdef USE_CHANGE_TEMPERATURE
        else if (IS_SELECTED_SCROLL(5 + BED_MENU_OFFSET))
            LCD_EDIT_SETTING_P(material[active_extruder].change_temperature, PSTR("Change temperature"), MSGP_UNIT_CELSIUS, 0, get_maxtemp(active_extruder));
        else if (IS_SELECTED_SCROLL(6 + BED_MENU_OFFSET))
            LCD_EDIT_SETTING_P(material[active_extruder].change_preheat_wait_time, PSTR("Change wait time"), PSTR("sec"), 0, 180);
        else if (IS_SELECTED_SCROLL(7 + BED_MENU_OFFSET))
            menu.add_menu(menu_t(lcd_menu_material_settings_store));
#else
        else if (IS_SELECTED_SCROLL(5 + BED_MENU_OFFSET))
            menu.add_menu(menu_t(lcd_menu_material_settings_store));
#endif
    }
    lcd_lib_update_screen();
}

static void lcd_material_temperature_settings_callback(uint8_t nr, uint8_t offsetY, uint8_t flags)
{
    char buffer[21];
    if (nr == 0)
    {
        strcpy_P(buffer, MSGP_ENTRY_RETURN);
    }
    else if (nr == 1)
    {
        strcpy_P(buffer, PSTR("Temperature: default"));
    }
    else
    {
        strcpy_P(buffer, PSTR("Temperature: "));
        float_to_string2(nozzleIndexToNozzleSize(nr - 2), buffer + strlen(buffer));
    }

    lcd_draw_scroll_entry(offsetY, buffer, flags);
}

static void lcd_material_settings_temperature_details_callback(uint8_t nr)
{
    char buffer[21];
    if (nr == 1)
    {
        int_to_string(material[active_extruder].temperature_default, buffer, MSGP_UNIT_CELSIUS);
        lcd_lib_draw_string_left(BOTTOM_MENU_YPOS, buffer);
    }
    else if (nr > 1)
    {
        if (material[active_extruder].temperature[nr - 2] == 0)
        {
            strcpy_P(buffer, PSTR("default ("));
            int_to_string(material[active_extruder].temperature_default, buffer + strlen(buffer), PSTR("C)"));
        }
        else
        {
            int_to_string(material[active_extruder].temperature[nr - 2], buffer, MSGP_UNIT_CELSIUS);
        }
        lcd_lib_draw_string_left(BOTTOM_MENU_YPOS, buffer);
    }
}

static void lcd_menu_material_temperature_settings()
{
    lcd_scroll_menu(MSGP_MENU_MATERIAL, 2 + MATERIAL_TEMPERATURE_COUNT, lcd_material_temperature_settings_callback, lcd_material_settings_temperature_details_callback);
    if (lcd_lib_button_pressed)
    {
        if (IS_SELECTED_SCROLL(0))
        {
            menu.return_to_previous();
        }
        else if (IS_SELECTED_SCROLL(1))
        {
            //menu.return_to_previous();
            LCD_EDIT_SETTING_P(material[active_extruder].temperature_default, MSGP_TEMPERATURE, MSGP_UNIT_CELSIUS, 0, HEATER_0_MAXTEMP - 15);
        }
        else
        {
            uint8_t index = SELECTED_SCROLL_MENU_ITEM() - 2;
            //menu.return_to_previous();
            LCD_EDIT_SETTING_P(material[active_extruder].temperature[index], MSGP_TEMPERATURE, MSGP_UNIT_CELSIUS, 0, HEATER_0_MAXTEMP - 15, LCD_SETTINGS_TYPE_OFF_BY_0);
        }
    }
    lcd_lib_update_screen();
}

#if TEMP_SENSOR_BED != 0

static void lcd_material_bed_temperature_settings_callback(uint8_t nr, uint8_t offsetY, uint8_t flags)
{
    char buffer[21];
    if (nr == 0)
        strcpy_P(buffer, MSGP_ENTRY_RETURN);
    else if (nr == 1)
        strcpy_P(buffer, MSGP_BUILDPLATE_TEMPERATURE);
    else if (nr == 2)
        strcpy_P(buffer, MSGP_BUILDPLATE_TEMPERATURE_FIRST_LAYER);
    else
        strcpy_P(buffer, MSGP_ENTRY_UNKNOWN);

    lcd_draw_scroll_entry(offsetY, buffer, flags);
}

static void lcd_material_settings_bed_temperature_details_callback(uint8_t nr)
{
    char buffer[10];
    if (nr == 1)
    {
        int_to_string(material[active_extruder].bed_temperature, buffer, MSGP_UNIT_CELSIUS);
        lcd_lib_draw_string_left(BOTTOM_MENU_YPOS, buffer);
    }
    if (nr == 2)
    {
        if (material[active_extruder].bed_temperature_first_layer == 0)
            strcpy_P(buffer, MSGP_OFF_BY_0);
        else
            int_to_string(material[active_extruder].bed_temperature_first_layer, buffer, MSGP_UNIT_CELSIUS);
        lcd_lib_draw_string_left(BOTTOM_MENU_YPOS, buffer);
    }
}

static void lcd_menu_material_bed_temperature_settings()
{
    lcd_scroll_menu(MSGP_HEATED_BUILDPLATE, 3, lcd_material_bed_temperature_settings_callback, lcd_material_settings_bed_temperature_details_callback);
    if (lcd_lib_button_pressed)
    {
        if (IS_SELECTED_SCROLL(0))
        {
            menu.return_to_previous();
        }
        else if (IS_SELECTED_SCROLL(1))
        {
            //menu.return_to_previous();
            LCD_EDIT_SETTING_P(material[active_extruder].bed_temperature, MSGP_BUILDPLATE_TEMPERATURE, MSGP_UNIT_CELSIUS, 0, BED_MAXTEMP - 15);
        }
        else if (IS_SELECTED_SCROLL(2))
        {
            //menu.return_to_previous();
            LCD_EDIT_SETTING_P(material[active_extruder].bed_temperature_first_layer, MSGP_BUILDPLATE_TEMPERATURE_FIRST_LAYER, MSGP_UNIT_CELSIUS, 0, BED_MAXTEMP - 15, LCD_SETTINGS_TYPE_OFF_BY_0);
        }
    }
    lcd_lib_update_screen();
}

#endif // TEMP_SENSOR_BED != 0


static void lcd_menu_material_settings_store_callback(uint8_t nr, uint8_t offsetY, uint8_t flags)
{
    uint8_t count = eeprom_read_byte(EEPROM_MATERIAL_COUNT_OFFSET());
    char buffer[32];
    if (nr == 0)
        strcpy_P(buffer, MSGP_ENTRY_RETURN);
    else if (nr > count)
        strcpy_P(buffer, PSTR("New preset"));
    else{
        eeprom_read_block(buffer, EEPROM_MATERIAL_NAME_OFFSET(nr - 1), MATERIAL_NAME_SIZE);
        buffer[MATERIAL_NAME_SIZE] = '\0';
    }
    lcd_draw_scroll_entry(offsetY, buffer, flags);
}

static void lcd_menu_material_settings_store_details_callback(uint8_t nr)
{
}

static void lcd_menu_material_settings_store()
{
    uint8_t count = eeprom_read_byte(EEPROM_MATERIAL_COUNT_OFFSET());
    if (count == EEPROM_MATERIAL_SETTINGS_MAX_COUNT)
        count--;
    lcd_scroll_menu(PSTR("PRESETS"), 2 + count, lcd_menu_material_settings_store_callback, lcd_menu_material_settings_store_details_callback);

    if (lcd_lib_button_pressed)
    {
        if (!IS_SELECTED_SCROLL(0))
        {
            uint8_t idx = SELECTED_SCROLL_MENU_ITEM() - 1;
            if (idx == count)
            {
                char buffer[9];
                strcpy_P(buffer, PSTR("CUSTOM"));
                int_to_string(idx - 1, buffer + 6);
                eeprom_update_block(buffer, EEPROM_MATERIAL_NAME_OFFSET(idx), MATERIAL_NAME_SIZE);
                eeprom_update_byte(EEPROM_MATERIAL_COUNT_OFFSET(), idx + 1);
            }
            lcd_material_store_material(idx);
        }
        lcd_change_to_previous_menu();
    }
    lcd_lib_update_screen();
}

inline void eeprom_material_extra_temperature_clear_unused (uint8_t nr)
{
    for(uint8_t n=MATERIAL_TEMPERATURE_COUNT; n<MAX_MATERIAL_TEMPERATURES; ++n)
    {
        eeprom_update_word(EEPROM_MATERIAL_EXTRA_TEMPERATURE_OFFSET(nr, n), 0);
    }
}

void lcd_material_write_CPE (char* buffer, uint8_t nr)
{
    strcpy_P(buffer, MSGP_MATERIAL_CPE);
    eeprom_update_block(buffer, EEPROM_MATERIAL_NAME_OFFSET(nr), 4);
    eeprom_update_word(EEPROM_MATERIAL_TEMPERATURE_OFFSET(nr), 250);
    eeprom_update_word(EEPROM_MATERIAL_BED_TEMPERATURE_OFFSET(nr),      60);
    eeprom_update_word(EEPROM_MATERIAL_BED_TEMPERATURE_FIRST_LAYER(nr), 70);
    eeprom_update_byte(EEPROM_MATERIAL_FAN_SPEED_OFFSET(nr), 50);
    eeprom_update_word(EEPROM_MATERIAL_FLOW_OFFSET(nr), 100);
    eeprom_update_float(EEPROM_MATERIAL_DIAMETER_OFFSET(nr), DEFAULT_FILAMENT_DIAMETER);
    eeprom_update_word(EEPROM_MATERIAL_EXTRA_TEMPERATURE_OFFSET(nr, 0), 250);//0.4
    eeprom_update_word(EEPROM_MATERIAL_EXTRA_TEMPERATURE_OFFSET(nr, 1), 245);//0.25
    eeprom_update_word(EEPROM_MATERIAL_EXTRA_TEMPERATURE_OFFSET(nr, 2), 255);//0.6
    eeprom_update_word(EEPROM_MATERIAL_EXTRA_TEMPERATURE_OFFSET(nr, 3), 260);//0.8
    eeprom_update_word(EEPROM_MATERIAL_EXTRA_TEMPERATURE_OFFSET(nr, 4), 260);//1.0
    eeprom_material_extra_temperature_clear_unused (nr);

    eeprom_update_word(EEPROM_MATERIAL_CHANGE_TEMPERATURE(nr), 85);
    eeprom_update_byte(EEPROM_MATERIAL_CHANGE_WAIT_TIME(nr), 15);
}

void lcd_material_reset_defaults()
{
    //Fill in the defaults
    char buffer[MATERIAL_NAME_SIZE+1];
    int8_t nr = -1;

#ifndef NO_MATERIAL_PLA
    ++nr;
    strcpy_P(buffer, MSGP_MATERIAL_PLA);
    eeprom_update_block(buffer, EEPROM_MATERIAL_NAME_OFFSET(nr), 4);
    eeprom_update_word(EEPROM_MATERIAL_TEMPERATURE_OFFSET(nr), 210);
    eeprom_update_word(EEPROM_MATERIAL_BED_TEMPERATURE_OFFSET(nr), 60);
    eeprom_update_word(EEPROM_MATERIAL_BED_TEMPERATURE_FIRST_LAYER(nr), 65);
    eeprom_update_byte(EEPROM_MATERIAL_FAN_SPEED_OFFSET(nr), 100);
    eeprom_update_word(EEPROM_MATERIAL_FLOW_OFFSET(nr), 100);
    eeprom_update_float(EEPROM_MATERIAL_DIAMETER_OFFSET(nr), DEFAULT_FILAMENT_DIAMETER);

    eeprom_update_word(EEPROM_MATERIAL_EXTRA_TEMPERATURE_OFFSET(nr, 0), 210);//0.4
    eeprom_update_word(EEPROM_MATERIAL_EXTRA_TEMPERATURE_OFFSET(nr, 1), 195);//0.25
    eeprom_update_word(EEPROM_MATERIAL_EXTRA_TEMPERATURE_OFFSET(nr, 2), 230);//0.6
    eeprom_update_word(EEPROM_MATERIAL_EXTRA_TEMPERATURE_OFFSET(nr, 3), 240);//0.8
    eeprom_update_word(EEPROM_MATERIAL_EXTRA_TEMPERATURE_OFFSET(nr, 4), 240);//1.0
    eeprom_material_extra_temperature_clear_unused (nr);

    eeprom_update_word(EEPROM_MATERIAL_CHANGE_TEMPERATURE(nr), 70);
    eeprom_update_byte(EEPROM_MATERIAL_CHANGE_WAIT_TIME(nr), 30);
#endif // NO_MATERIAL_PLA

#ifndef NO_MATERIAL_ABS
    ++nr;
    strcpy_P(buffer, MSGP_MATERIAL_ABS);
    eeprom_update_block(buffer, EEPROM_MATERIAL_NAME_OFFSET(nr), 4);
    eeprom_update_word(EEPROM_MATERIAL_TEMPERATURE_OFFSET(nr), 250);
    eeprom_update_word(EEPROM_MATERIAL_BED_TEMPERATURE_OFFSET(nr), 90);
    eeprom_update_word(EEPROM_MATERIAL_BED_TEMPERATURE_FIRST_LAYER(nr), 110);
    eeprom_update_byte(EEPROM_MATERIAL_FAN_SPEED_OFFSET(nr), 100);
    eeprom_update_word(EEPROM_MATERIAL_FLOW_OFFSET(nr), 107);
    eeprom_update_float(EEPROM_MATERIAL_DIAMETER_OFFSET(nr), DEFAULT_FILAMENT_DIAMETER);

    eeprom_update_word(EEPROM_MATERIAL_EXTRA_TEMPERATURE_OFFSET(nr, 0), 250);//0.4
    eeprom_update_word(EEPROM_MATERIAL_EXTRA_TEMPERATURE_OFFSET(nr, 1), 245);//0.25
    eeprom_update_word(EEPROM_MATERIAL_EXTRA_TEMPERATURE_OFFSET(nr, 2), 255);//0.6
    eeprom_update_word(EEPROM_MATERIAL_EXTRA_TEMPERATURE_OFFSET(nr, 3), 260);//0.8
    eeprom_update_word(EEPROM_MATERIAL_EXTRA_TEMPERATURE_OFFSET(nr, 4), 260);//1.0
    eeprom_material_extra_temperature_clear_unused (nr);

    eeprom_update_word(EEPROM_MATERIAL_CHANGE_TEMPERATURE(nr), 90);
    eeprom_update_byte(EEPROM_MATERIAL_CHANGE_WAIT_TIME(nr), 30);
#endif // NO_MATERIAL_ABS

#ifndef NO_MATERIAL_CPE
    ++nr;
    lcd_material_write_CPE (buffer, nr);
#endif // NO_MATERIAL_CPE

#ifndef NO_MATERIAL_PC
    ++nr;
    strcpy_P(buffer, PSTR("PC"));
    eeprom_update_block(buffer, EEPROM_MATERIAL_NAME_OFFSET(nr), 3);
    eeprom_update_word(EEPROM_MATERIAL_TEMPERATURE_OFFSET(nr), 260);
    eeprom_update_word(EEPROM_MATERIAL_BED_TEMPERATURE_OFFSET(nr), 110);
    eeprom_update_byte(EEPROM_MATERIAL_FAN_SPEED_OFFSET(nr), 100);
    eeprom_update_word(EEPROM_MATERIAL_FLOW_OFFSET(nr), 100);
    eeprom_update_float(EEPROM_MATERIAL_DIAMETER_OFFSET(nr), DEFAULT_FILAMENT_DIAMETER);

    eeprom_update_word(EEPROM_MATERIAL_EXTRA_TEMPERATURE_OFFSET(nr, 0), 260);//0.4
    eeprom_update_word(EEPROM_MATERIAL_EXTRA_TEMPERATURE_OFFSET(nr, 1), 260);//0.25
    eeprom_update_word(EEPROM_MATERIAL_EXTRA_TEMPERATURE_OFFSET(nr, 2), 260);//0.6
    eeprom_update_word(EEPROM_MATERIAL_EXTRA_TEMPERATURE_OFFSET(nr, 3), 260);//0.8
    eeprom_update_word(EEPROM_MATERIAL_EXTRA_TEMPERATURE_OFFSET(nr, 4), 260);//1.0
    eeprom_material_extra_temperature_clear_unused (nr);

    eeprom_update_word(EEPROM_MATERIAL_CHANGE_TEMPERATURE(nr), 85);
    eeprom_update_byte(EEPROM_MATERIAL_CHANGE_WAIT_TIME(nr), 15);
#endif // NO_MATERIAL_PC

#ifndef NO_MATERIAL_NYLON
    ++nr;
    strcpy_P(buffer, PSTR("Nylon"));
    eeprom_update_block(buffer, EEPROM_MATERIAL_NAME_OFFSET(nr), 6);
    eeprom_update_word(EEPROM_MATERIAL_TEMPERATURE_OFFSET(nr), 250);
    eeprom_update_word(EEPROM_MATERIAL_BED_TEMPERATURE_OFFSET(nr), 60);
    eeprom_update_byte(EEPROM_MATERIAL_FAN_SPEED_OFFSET(nr), 100);
    eeprom_update_word(EEPROM_MATERIAL_FLOW_OFFSET(nr), 100);
    eeprom_update_float(EEPROM_MATERIAL_DIAMETER_OFFSET(nr), 2.85);

    eeprom_update_word(EEPROM_MATERIAL_EXTRA_TEMPERATURE_OFFSET(nr, 0), 250);//0.4
    eeprom_update_word(EEPROM_MATERIAL_EXTRA_TEMPERATURE_OFFSET(nr, 1), 240);//0.25
    eeprom_update_word(EEPROM_MATERIAL_EXTRA_TEMPERATURE_OFFSET(nr, 2), 255);//0.6
    eeprom_update_word(EEPROM_MATERIAL_EXTRA_TEMPERATURE_OFFSET(nr, 3), 260);//0.8
    eeprom_update_word(EEPROM_MATERIAL_EXTRA_TEMPERATURE_OFFSET(nr, 4), 260);//1.0
    eeprom_material_extra_temperature_clear_unused (nr);

    eeprom_update_word(EEPROM_MATERIAL_CHANGE_TEMPERATURE(nr), 85);
    eeprom_update_byte(EEPROM_MATERIAL_CHANGE_WAIT_TIME(nr), 15);
#endif // NO_MATERIAL_NYLON

    eeprom_update_byte(EEPROM_MATERIAL_COUNT_OFFSET(), nr + 1);
}

void lcd_material_set_material(uint8_t nr, uint8_t e)
{
    material[e].temperature_default = eeprom_read_word(EEPROM_MATERIAL_TEMPERATURE_OFFSET(nr));
    set_maxtemp(e, constrain(material[e].temperature_default + 15, HEATER_0_MAXTEMP, min(HEATER_0_MAXTEMP + 15, material[e].temperature_default + 15)));

#if TEMP_SENSOR_BED != 0
    material[e].bed_temperature             = eeprom_read_word(EEPROM_MATERIAL_BED_TEMPERATURE_OFFSET(nr));
    cut_max (material[e].bed_temperature,             BED_MAXTEMP - 15);
    material[e].bed_temperature_first_layer = eeprom_read_word(EEPROM_MATERIAL_BED_TEMPERATURE_FIRST_LAYER(nr));
    cut_max (material[e].bed_temperature_first_layer, BED_MAXTEMP - 15);
#endif
    material[e].flow = eeprom_read_word(EEPROM_MATERIAL_FLOW_OFFSET(nr));

    material[e].fan_speed = eeprom_read_byte(EEPROM_MATERIAL_FAN_SPEED_OFFSET(nr));
    material[e].diameter = eeprom_read_float(EEPROM_MATERIAL_DIAMETER_OFFSET(nr));

    eeprom_read_block(material[e].name, EEPROM_MATERIAL_NAME_OFFSET(nr), MATERIAL_NAME_SIZE);
    material[e].name[MATERIAL_NAME_SIZE] = '\0';
    strcpy(LCD_CACHE_FILENAME(0), material[e].name);
    for(uint8_t n=0; n<MAX_MATERIAL_TEMPERATURES; ++n)
    {
        material[e].temperature[n] = eeprom_read_word(EEPROM_MATERIAL_EXTRA_TEMPERATURE_OFFSET(nr, n));
//        set_maxtemp(e, constrain(material[e].temperature[n] + 15, HEATER_0_MAXTEMP, min(max(get_maxtemp(e), HEATER_0_MAXTEMP + 15), material[e].temperature[n] + 15)));
        cut_max (material[e].temperature[n], get_maxtemp(e) - 15);
    }

    material[e].change_temperature = eeprom_read_word(EEPROM_MATERIAL_CHANGE_TEMPERATURE(nr));
    material[e].change_preheat_wait_time = eeprom_read_byte(EEPROM_MATERIAL_CHANGE_WAIT_TIME(nr));
    if ((material[e].change_temperature < 10) || (material[e].change_temperature > (get_maxtemp(e) - 15)))
        material[e].change_temperature = material[e].temperature_default;

    lcd_material_store_current_material();
}

void lcd_material_store_material(uint8_t nr)
{
    eeprom_update_word(EEPROM_MATERIAL_TEMPERATURE_OFFSET(nr), material[active_extruder].temperature_default);
#if TEMP_SENSOR_BED != 0
    eeprom_update_word(EEPROM_MATERIAL_BED_TEMPERATURE_OFFSET(nr),      material[active_extruder].bed_temperature);
    eeprom_update_word(EEPROM_MATERIAL_BED_TEMPERATURE_FIRST_LAYER(nr), material[active_extruder].bed_temperature_first_layer);
#endif
    eeprom_update_word(EEPROM_MATERIAL_FLOW_OFFSET(nr), material[active_extruder].flow);

    eeprom_update_byte(EEPROM_MATERIAL_FAN_SPEED_OFFSET(nr), material[active_extruder].fan_speed);
    eeprom_update_float(EEPROM_MATERIAL_DIAMETER_OFFSET(nr), material[active_extruder].diameter);
    for(uint8_t n=0; n<MAX_MATERIAL_TEMPERATURES; ++n)
        eeprom_update_word(EEPROM_MATERIAL_EXTRA_TEMPERATURE_OFFSET(nr, n), material[active_extruder].temperature[n]);

    eeprom_update_word(EEPROM_MATERIAL_CHANGE_TEMPERATURE(nr), material[active_extruder].change_temperature);
    eeprom_update_byte(EEPROM_MATERIAL_CHANGE_WAIT_TIME(nr), material[active_extruder].change_preheat_wait_time);
}

void lcd_material_read_current_material()
{
    for(uint8_t e=0; e<EXTRUDERS; ++e)
    {
        material[e].temperature_default = eeprom_read_word(EEPROM_MATERIAL_TEMPERATURE_OFFSET(EEPROM_MATERIAL_SETTINGS_MAX_COUNT+e));
        set_maxtemp(e, constrain(material[e].temperature_default + 15, HEATER_0_MAXTEMP, min(HEATER_0_MAXTEMP + 15, material[e].temperature_default + 15)));
#if TEMP_SENSOR_BED != 0
        material[e].bed_temperature             = eeprom_read_word(EEPROM_MATERIAL_BED_TEMPERATURE_OFFSET     (EEPROM_MATERIAL_SETTINGS_MAX_COUNT+e));
        material[e].bed_temperature_first_layer = eeprom_read_word(EEPROM_MATERIAL_BED_TEMPERATURE_FIRST_LAYER(EEPROM_MATERIAL_SETTINGS_MAX_COUNT+e));
#endif
        material[e].flow = eeprom_read_word(EEPROM_MATERIAL_FLOW_OFFSET(EEPROM_MATERIAL_SETTINGS_MAX_COUNT+e));

        material[e].fan_speed = eeprom_read_byte(EEPROM_MATERIAL_FAN_SPEED_OFFSET(EEPROM_MATERIAL_SETTINGS_MAX_COUNT+e));
        material[e].diameter = eeprom_read_float(EEPROM_MATERIAL_DIAMETER_OFFSET(EEPROM_MATERIAL_SETTINGS_MAX_COUNT+e));
        for(uint8_t n=0; n<MAX_MATERIAL_TEMPERATURES; ++n)
        {
            material[e].temperature[n] = eeprom_read_word(EEPROM_MATERIAL_EXTRA_TEMPERATURE_OFFSET(EEPROM_MATERIAL_SETTINGS_MAX_COUNT+e, n));
            // set_maxtemp(e, constrain(material[e].temperature[n] + 15, HEATER_0_MAXTEMP, min(HEATER_0_MAXTEMP + 15, material[e].temperature[n] + 15)));
        }

        eeprom_read_block(material[e].name, EEPROM_MATERIAL_NAME_OFFSET(EEPROM_MATERIAL_SETTINGS_MAX_COUNT+e), MATERIAL_NAME_SIZE);
        material[e].name[MATERIAL_NAME_SIZE] = '\0';

        material[e].change_temperature = eeprom_read_word(EEPROM_MATERIAL_CHANGE_TEMPERATURE(EEPROM_MATERIAL_SETTINGS_MAX_COUNT+e));
        material[e].change_preheat_wait_time = eeprom_read_byte(EEPROM_MATERIAL_CHANGE_WAIT_TIME(EEPROM_MATERIAL_SETTINGS_MAX_COUNT+e));
        if ((material[e].change_temperature < 10) || (material[e].change_temperature > (get_maxtemp(e) - 15)))
            material[e].change_temperature = material[e].temperature_default;
    }
}

void lcd_material_store_current_material()
{
    for(uint8_t e=0; e<EXTRUDERS; ++e)
    {
        eeprom_update_word(EEPROM_MATERIAL_TEMPERATURE_OFFSET(EEPROM_MATERIAL_SETTINGS_MAX_COUNT+e), material[e].temperature_default);
        set_maxtemp(e, constrain(material[e].temperature_default + 15, HEATER_0_MAXTEMP, min(HEATER_0_MAXTEMP + 15, material[e].temperature_default + 15)));
#if TEMP_SENSOR_BED != 0
        eeprom_update_word(EEPROM_MATERIAL_BED_TEMPERATURE_OFFSET     (EEPROM_MATERIAL_SETTINGS_MAX_COUNT+e), material[e].bed_temperature);
        eeprom_update_word(EEPROM_MATERIAL_BED_TEMPERATURE_FIRST_LAYER(EEPROM_MATERIAL_SETTINGS_MAX_COUNT+e), material[e].bed_temperature_first_layer);
#endif
        eeprom_update_byte(EEPROM_MATERIAL_FAN_SPEED_OFFSET(EEPROM_MATERIAL_SETTINGS_MAX_COUNT+e), material[e].fan_speed);
        eeprom_update_word(EEPROM_MATERIAL_FLOW_OFFSET(EEPROM_MATERIAL_SETTINGS_MAX_COUNT+e), material[e].flow);
        eeprom_update_float(EEPROM_MATERIAL_DIAMETER_OFFSET(EEPROM_MATERIAL_SETTINGS_MAX_COUNT+e), material[e].diameter);

        for(uint8_t n=0; n<MAX_MATERIAL_TEMPERATURES; ++n)
        {
            eeprom_update_word(EEPROM_MATERIAL_EXTRA_TEMPERATURE_OFFSET(EEPROM_MATERIAL_SETTINGS_MAX_COUNT+e, n), material[e].temperature[n]);
            // set_maxtemp(e, constrain(material[e].temperature[n] + 15, HEATER_0_MAXTEMP, min(max(get_maxtemp(e), HEATER_0_MAXTEMP + 15), material[e].temperature[n] + 15)));
        }

        eeprom_update_block(material[e].name, EEPROM_MATERIAL_NAME_OFFSET(EEPROM_MATERIAL_SETTINGS_MAX_COUNT+e), MATERIAL_NAME_SIZE);

        eeprom_update_word(EEPROM_MATERIAL_CHANGE_TEMPERATURE(EEPROM_MATERIAL_SETTINGS_MAX_COUNT+e), material[e].change_temperature);
        eeprom_update_byte(EEPROM_MATERIAL_CHANGE_WAIT_TIME(EEPROM_MATERIAL_SETTINGS_MAX_COUNT+e), material[e].change_preheat_wait_time);
    }
}

bool lcd_material_verify_material_settings()
{
#ifndef NO_MATERIAL_CPE
    bool hasCPE = false;
#endif // NO_MATERIAL_CPE
    char buffer[MATERIAL_NAME_SIZE+1];

    uint8_t cnt = eeprom_read_byte(EEPROM_MATERIAL_COUNT_OFFSET());
    if (cnt < 2 || cnt > EEPROM_MATERIAL_SETTINGS_MAX_COUNT)
        return false;
    while(cnt > 0)
    {
        cnt --;
        if (eeprom_read_word(EEPROM_MATERIAL_TEMPERATURE_OFFSET(cnt)) > HEATER_0_MAXTEMP)
            return false;
#if TEMP_SENSOR_BED != 0
        if (eeprom_read_word(EEPROM_MATERIAL_BED_TEMPERATURE_OFFSET(cnt))      > BED_MAXTEMP)
            return false;
        if (eeprom_read_word(EEPROM_MATERIAL_BED_TEMPERATURE_FIRST_LAYER(cnt)) > BED_MAXTEMP)
            return false;
#endif
        if (eeprom_read_byte(EEPROM_MATERIAL_FAN_SPEED_OFFSET(cnt)) > 100)
            return false;
        if (eeprom_read_word(EEPROM_MATERIAL_FLOW_OFFSET(cnt)) > 1000)
            return false;
        if (eeprom_read_float(EEPROM_MATERIAL_DIAMETER_OFFSET(cnt)) > 10.0)
            return false;
        if (eeprom_read_float(EEPROM_MATERIAL_DIAMETER_OFFSET(cnt)) < 0.1)
            return false;

        for(uint8_t n=0; n<MATERIAL_TEMPERATURE_COUNT; ++n)
        {
            if (eeprom_read_word(EEPROM_MATERIAL_EXTRA_TEMPERATURE_OFFSET(cnt, n)) > HEATER_0_MAXTEMP)
                return false;
            if (eeprom_read_word(EEPROM_MATERIAL_EXTRA_TEMPERATURE_OFFSET(cnt, n)) == 0)
                if (eeprom_read_word(EEPROM_MATERIAL_TEMPERATURE_OFFSET(cnt)) == 0)
                    return false;
        }

        eeprom_read_block(buffer, EEPROM_MATERIAL_NAME_OFFSET(cnt), MATERIAL_NAME_SIZE);
        buffer[MATERIAL_NAME_SIZE] = '\0';

#ifndef NO_MATERIAL_CPE
        if (strcmp_P(buffer, MSGP_MATERIAL_UPET) == 0)
        {
            strcpy_P(buffer, MSGP_MATERIAL_CPE);
            eeprom_update_block(buffer, EEPROM_MATERIAL_NAME_OFFSET(cnt), 4);
        }
        if (strcmp_P(buffer, MSGP_MATERIAL_CPE) == 0)
        {
            hasCPE = true;
        }
#endif // NO_MATERIAL_CPE

        if (eeprom_read_word(EEPROM_MATERIAL_CHANGE_TEMPERATURE(cnt)) > HEATER_0_MAXTEMP || eeprom_read_word(EEPROM_MATERIAL_CHANGE_TEMPERATURE(cnt)) < 10)
        {
            //Invalid temperature for change temperature.
#ifndef NO_MATERIAL_PLA
            if (strcmp_P(buffer, MSGP_MATERIAL_PLA) == 0)
            {
                eeprom_update_word(EEPROM_MATERIAL_CHANGE_TEMPERATURE(cnt), 70);
                eeprom_update_byte(EEPROM_MATERIAL_CHANGE_WAIT_TIME(cnt), 30);
            } else
#endif // NO_MATERIAL_PLA
#ifndef NO_MATERIAL_ABS
            if (strcmp_P(buffer, MSGP_MATERIAL_ABS) == 0)
            {
                eeprom_update_word(EEPROM_MATERIAL_CHANGE_TEMPERATURE(cnt), 90);
                eeprom_update_byte(EEPROM_MATERIAL_CHANGE_WAIT_TIME(cnt), 30);
            } else
#endif // NO_MATERIAL_ABS
#ifndef NO_MATERIAL_CPE
            if (strcmp_P(buffer, MSGP_MATERIAL_CPE) == 0)
            {
                eeprom_update_word(EEPROM_MATERIAL_CHANGE_TEMPERATURE(cnt), 85);
                eeprom_update_byte(EEPROM_MATERIAL_CHANGE_WAIT_TIME(cnt), 15);
            } else
#endif // NO_MATERIAL_CPE
            {
                eeprom_update_word(EEPROM_MATERIAL_CHANGE_TEMPERATURE(cnt), eeprom_read_word(EEPROM_MATERIAL_TEMPERATURE_OFFSET(cnt)));
                eeprom_update_byte(EEPROM_MATERIAL_CHANGE_WAIT_TIME(cnt), 5);
            }
        }
    }
#ifndef NO_MATERIAL_CPE
    cnt = eeprom_read_byte(EEPROM_MATERIAL_COUNT_OFFSET());
    if (!hasCPE && cnt < EEPROM_MATERIAL_SETTINGS_MAX_COUNT)
    {
        lcd_material_write_CPE (buffer, cnt);
        eeprom_update_byte(EEPROM_MATERIAL_COUNT_OFFSET(), cnt + 1);
    }
#endif // NO_MATERIAL_CPE
    return true;
}

uint8_t nozzleSizeToTemperatureIndex(float nozzle_size)
{
    if (fabs(nozzle_size - 0.25) < 0.1)
        return 1;
    if (fabs(nozzle_size - 0.60) < 0.1)
        return 2;
    if (fabs(nozzle_size - 0.80) < 0.1)
        return 3;
    if (fabs(nozzle_size - 1.00) < 0.1)
        return 4;

    //Default to index 0
    return 0;
}

float nozzleIndexToNozzleSize(uint8_t nozzle_index)
{
    switch(nozzle_index)
    {
    case 0:
        return 0.4;
    case 1:
        return 0.25;
    case 2:
        return 0.6;
    case 3:
        return 0.8;
    case 4:
        return 1.0;
    }
    return 0.0;
}

#endif//ENABLE_ULTILCD2
