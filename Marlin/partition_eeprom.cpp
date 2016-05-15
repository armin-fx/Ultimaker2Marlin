#ifndef PARTITION_EEPROM_CPP
#define PARTITION_EEPROM_CPP

#include <inttypes.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>

#include "partition_eeprom.h"


uint16_t xorshift16 (uint16_t value)
{
    value ^= value <<  9;
    value ^= value >>  5;
    value ^= value <<  1;
    value ^= value <<  8;
    return   value;
}
uint32_t xorshift32 (uint32_t value)
{
    value ^= value << 13;
    value ^= value >> 17;
    value ^= value << 5;
    return   value;
}
uint64_t xorshift64 (uint64_t value)
{
    value ^= value << 13;
    value ^= value >> 7;
    value ^= value << 17;
    return   value;
}

void copy_eeprom (void* target, void* source, uint16_t size)
{
    for (uint16_t i = 0; i < size; ++i)
    {
        eeprom_update_byte((uint8_t*)target, eeprom_read_byte((const uint8_t*)source));
        target = (void*) ((uint16_t)target + 1);
        source = (void*) ((uint16_t)source + 1);
    }
}

bool check_eeprom_base_address (void* base)
{
    uint16_t address      = (uint16_t) eeprom_read_word ((uint16_t*)((uint16_t)base));
    uint8_t  address_hash = (uint8_t)  eeprom_read_byte ((uint8_t*) ((uint16_t)base+2));

    if (address      >= E2END)                  return false;
    if (address_hash != get_hash8_16 (address)) return false;
    return true;
}

void write_eeprom_base_address (void* base, void* partition)
{
    eeprom_update_word ((uint16_t*)((uint16_t)base),                 (uint16_t) partition);
    eeprom_update_byte ((uint8_t*) ((uint16_t)base+2), get_hash8_16 ((uint16_t) partition));
}

#endif // PARTITION_EEPROM_CPP

