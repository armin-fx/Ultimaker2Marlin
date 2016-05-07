#ifndef ULTILCD2_TEXT_H
#define ULTILCD2_TEXT_H

//const char MSGP_[] PROGMEM = {""};
const char MSGP_NEWLINE[]       PROGMEM = {"\n"};
const char MSGP_SLASH[]         PROGMEM = {"/"};
const char MSGP_BRACKED_OPEN[]  PROGMEM = {"("};
const char MSGP_BRACKED_CLOSE[] PROGMEM = {")"};
const char MSGP_EQUAL[]         PROGMEM = {"="};
const char MSGP_UNDERLINE[]     PROGMEM = {"_"};
const char MSGP_CLICK_TO_RETURN[]    PROGMEM = {"Click to return"};
const char MSGP_PAUSE[]              PROGMEM = {"pause"};
const char MSGP_RESUME[]             PROGMEM = {"resume"};
const char MSGP_STORE_SETTINGS[]     PROGMEM = {"Store settings"};
const char MSGP_READING_CARD[]       PROGMEM = {"Reading card..."};
const char MSGP_START_PID_AUTOTUNE[] PROGMEM = {"Start PID autotune"};
const char MSGP_BUILDPLATE[]                         PROGMEM = {"Buildplate"};
const char MSGP_HEATED_BUILDPLATE[]                  PROGMEM = {"Heated buildplate"};
const char MSGP_BUILDPLATE_TEMPERATURE[]             PROGMEM = {"Buildplate Temp."};
const char MSGP_BUILDPLATE_TEMPERATURE_FIRST_LAYER[] PROGMEM = {"Temp. first layer"};
const char MSGP_BUILDPLATE_TEMPERATURE_NEXT[]        PROGMEM = {"Buildpl. Temp. next"};
const char MSGP_EXTRUDER[]           PROGMEM = {"Extruder"};
const char MSGP_SWAP_EXTRUDERS[]     PROGMEM = {"Swap Extruders"};
const char MSGP_EXTRUDER_CHANGE[]    PROGMEM = {"Extruder change"};
const char MSGP_CONTROL_OPTIONS[]    PROGMEM = {"Control options"};
const char MSGP_ACCELERATION[]       PROGMEM = {"Acceleration"};
const char MSGP_ACCELERATION_SHORT[] PROGMEM = {"Accel"};
const char MSGP_MAX_XY_JERK[]        PROGMEM = {"Max. X/Y Jerk"};
const char MSGP_XY_JERK[]            PROGMEM = {"X/Y Jerk"};
const char MSGP_TOGGLE_LED[]         PROGMEM = {"Toggle LED"};
const char MSGP_PID_SETTINGS[]       PROGMEM = {"PID settings"};
const char MSGP_STORE_PID_SETTINGS[] PROGMEM = {"Store PID settings"};
const char MSGP_PROPORTIONAL_COEFFICIENT[] PROGMEM = {"Proportional coeff."};
const char MSGP_INTEGRAL_COEFFICIENT[]     PROGMEM = {"Integral coefficient"};
const char MSGP_DERIVATIVE_COEFFICIENT[]   PROGMEM = {"Derivative coeff."};
const char MSGP_PID_KP[] PROGMEM = {"Kp"};
const char MSGP_PID_KI[] PROGMEM = {"Ki"};
const char MSGP_PID_KD[] PROGMEM = {"Kd"};
const char MSGP_RETRACT[]             PROGMEM = {"Retract"};
const char MSGP_RETRACT_LENGTH[]      PROGMEM = {"Retract length"};
const char MSGP_RETRACT_SPEED[]       PROGMEM = {"Retract speed"};
const char MSGP_RETRACT_LENGTH_END[]  PROGMEM = {"End of print length"};
const char MSGP_FILAMENT_GRAB_COUNT[] PROGMEM = {"Filament grab count"};
const char MSGP_RETRACT_LENGTH_MIN[]  PROGMEM = {"Retract length min"};
const char MSGP_GOTO_PAGE_1[] PROGMEM = {"Goto page 1"};
const char MSGP_GOTO_PAGE_2[] PROGMEM = {"Goto page 2"};
const char MSGP_FAN_SPEED[]       PROGMEM = {"Fan speed"};
const char MSGP_MATERIAL_FLOW[]   PROGMEM = {"Material flow"};
const char MSGP_MATERIAL_FLOW_1[] PROGMEM = {"Material flow 1"};
const char MSGP_MATERIAL_FLOW_2[] PROGMEM = {"Material flow 2"};
const char MSGP_TEMPERATURE[]     PROGMEM = {"Temperature"};
const char MSGP_OFF_BY_0[] PROGMEM = {"off"};

//const char MSGP_UNIT_[] PROGMEM = {""};
const char MSGP_UNIT_SPEED[]        PROGMEM = {"mm/s"};
const char MSGP_UNIT_SPEED2[]       PROGMEM = {"mm/sec"};
const char MSGP_UNIT_FLOW_VOLUME[]  PROGMEM = {"mm\x1D/s"};
const char MSGP_UNIT_MM[]           PROGMEM = {"mm"};
const char MSGP_UNIT_PERCENT[]      PROGMEM = {"%"};
const char MSGP_UNIT_CELSIUS[]      PROGMEM = {"C"};
const char MSGP_UNIT_CELSIUS_LIST[] PROGMEM = {"C "};
const char MSGP_UNIT_CELSIUS_FROM[] PROGMEM = {"C/"};

//const char MSGP_MENU_[] PROGMEM = {""};
const char MSGP_MENU_PRINT[]       PROGMEM = {"PRINT"};
const char MSGP_MENU_MATERIAL[]    PROGMEM = {"MATERIAL"};
const char MSGP_MENU_TUNE[]        PROGMEM = {"TUNE"};
const char MSGP_MENU_RETURN[]      PROGMEM = {"RETURN"};
const char MSGP_MENU_BACK[]        PROGMEM = {"BACK"};
const char MSGP_MENU_ADVANCED[]    PROGMEM = {"ADVANCED"};
const char MSGP_MENU_MAINTENANCE[] PROGMEM = {"MAINTENANCE"};
const char MSGP_MENU_SETTINGS[]    PROGMEM = {"SETTINGS"};
const char MSGP_MENU_CHANGE[]      PROGMEM = {"CHANGE"};
const char MSGP_MENU_PREHEAT[]     PROGMEM = {"PREHEAT"};
const char MSGP_MENU_MOVE[]        PROGMEM = {"MOVE"};
const char MSGP_MENU_READY[]       PROGMEM = {"READY"};
const char MSGP_MENU_START[]       PROGMEM = {"START"};
const char MSGP_MENU_CONTINUE[]    PROGMEM = {"CONTINUE"};
const char MSGP_MENU_CANCEL[]      PROGMEM = {"CANCEL"};
const char MSGP_MENU_ABORT[]       PROGMEM = {"ABORT"};
const char MSGP_MENU_STORE[]       PROGMEM = {"STORE"};
const char MSGP_MENU_DONE[]        PROGMEM = {"DONE"};
const char MSGP_MENU_YES[]         PROGMEM = {"YES"};
const char MSGP_MENU_NO[]          PROGMEM = {"NO"};
const char MSGP_MENU_PRIMARY[]     PROGMEM = {"PRIMARY"};
const char MSGP_MENU_SECOND[]      PROGMEM = {"SECOND"};
const char MSGP_MENU_AUTO[]        PROGMEM = {"AUTO"};
const char MSGP_MENU_BUILDPLATE[]  PROGMEM = {"BUILD-|PLATE"};

//const char MSGP_STORE_[] PROGMEM = {""};
const char MSGP_STORE_ENTRY_MATERIAL[]              PROGMEM = {"[material]"};
const char MSGP_STORE_NAME[]                        PROGMEM = {"name"};
const char MSGP_STORE_TEMPERATURE[]                 PROGMEM = {"temperature"};
const char MSGP_STORE_TEMPERATURE_BED[]             PROGMEM = {"bed_temperature"};
const char MSGP_STORE_TEMPERATURE_BED_FIRST_LAYER[] PROGMEM = {"bed_temp_first_layer"};
const char MSGP_STORE_FAN_SPEED[]                   PROGMEM = {"fan_speed"};
const char MSGP_STORE_FLOW[]                        PROGMEM = {"flow"};
const char MSGP_STORE_DIAMETER[]                    PROGMEM = {"diameter"};
const char MSGP_STORE_CHANGE_TEMPERATURE[]          PROGMEM = {"change_temp"};
const char MSGP_STORE_CHANGE_WAIT[]                 PROGMEM = {"change_wait"};

//const char MSGP_ENTRY_[] PROGMEM = {""};
const char MSGP_ENTRY_RETURN[]  PROGMEM = {"< RETURN"};
const char MSGP_ENTRY_UNKNOWN[] PROGMEM = {"???"};
const char MSGP_ENTRY_SWITCH[]  PROGMEM = {">"};

//const char MSGP_CMD_[] PROGMEM = {""};
const char MSGP_CMD_COMMENT_ULTIGCODE[] PROGMEM = {";FLAVOR:UltiGCode"};
const char MSGP_CMD_MOVE_FAST_TO_XY[]   PROGMEM = {"G1 F12000 X%i Y%i"};
const char MSGP_CMD_MOVE_TO_XY[]        PROGMEM = {"G1 F%i X%i Y%i"};
const char MSGP_CMD_MOVE_TO_XYZ[]       PROGMEM = {"G1 F%i Z%i X%i Y%i"};
const char MSGP_CMD_HOME_ALL[] PROGMEM = {"G28"};
const char MSGP_CMD_HOME_X[]   PROGMEM = {"G28 X"};
const char MSGP_CMD_HOME_Y[]   PROGMEM = {"G28 Y"};
const char MSGP_CMD_HOME_XY[]  PROGMEM = {"G28 X Y"};
const char MSGP_CMD_HOME_Z[]   PROGMEM = {"G28 Z"};

//const char MSGP_MATERIAL_[] PROGMEM = {""};
const char MSGP_MATERIAL_PLA[]  PROGMEM = {"PLA"};
const char MSGP_MATERIAL_ABS[]  PROGMEM = {"ABS"};
const char MSGP_MATERIAL_CPE[]  PROGMEM = {"CPE"};
const char MSGP_MATERIAL_UPET[] PROGMEM = {"UPET"};


#endif
