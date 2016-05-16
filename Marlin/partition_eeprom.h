#ifndef PARTITION_EEPROM_H
#define PARTITION_EEPROM_H

#include <inttypes.h>

// TODO later implementation will regard a black list of addresses
//      then it will move the eeprom partition around the black list when needed

uint16_t xorshift16 (uint16_t value);
uint32_t xorshift32 (uint32_t value);
uint64_t xorshift64 (uint32_t value);

#define MAGIC_CONSTANT_16 0x517C

inline uint8_t  get_hash8_16  (uint16_t value)  { return (uint8_t)  xorshift16 (value + MAGIC_CONSTANT_16); }
inline uint16_t get_hash16_16 (uint16_t value)  { return (uint16_t) xorshift16 (value + MAGIC_CONSTANT_16); }

void copy_eeprom (void* target, void* source, uint16_t size);

// check for base address with hash value
bool check_eeprom_base_address (void* base);
// copy base address to target
inline
void copy_eeprom_base_address  (void* base, void* source)  { copy_eeprom (base, source, 3); }
// set base address and create the hash value
void write_eeprom_base_address (void* base, void* partition);

#define EEPROM_PARTITION_VECTOR_OFFSET 2
#define EEPROM_PARTITION_ENTRY_SIZE    4
#define EEPROM_PARTITION_POSITION_ADDRESS(partition_number) (EEPROM_PARTITION_VECTOR_OFFSET     + (EEPROM_PARTITION_ENTRY_SIZE * (partition_number)))
#define EEPROM_PARTITION_POSITION_SIZE(partition_number)    (EEPROM_PARTITION_VECTOR_OFFSET + 2 + (EEPROM_PARTITION_ENTRY_SIZE * (partition_number)))
#define EEPROM_PARTITION_GET_SIZE_WHEN(entries) (EEPROM_PARTITION_VECTOR_OFFSET + EEPROM_PARTITION_ENTRY_SIZE * (entries))

template <const uint16_t eeprom_base_address>
class eeprom_partition_t
{
public:
    // return true if partition list in good state
    bool is_valid ();

    // number of entries
    uint8_t get_table_size ();
    void    set_table_size (uint8_t size);
    //
    // get address of partition list
    inline
    void*   get_table_address ()  { return (void*) eeprom_read_word ((const uint16_t*) eeprom_base_address); }

    // size of partition number
    uint16_t get_size (uint8_t partition_number);
    //
    // address of partition number
           void  set_address (uint8_t partition_number, void* address, uint16_t size);
    inline void* get_address (uint8_t partition_number, uint16_t position=0)
    {
        uint16_t table_address = (uint16_t) get_table_address();
        uint16_t vector_adress     = eeprom_read_word((const uint16_t*)( (uint16_t)table_address + EEPROM_PARTITION_POSITION_ADDRESS(partition_number)));
        return (void*) (vector_adress + position);
    }

    inline uint8_t  read_byte (uint8_t partition_number, uint16_t position) { return eeprom_read_byte ((const uint8_t*)  get_address(partition_number, position)); }
    inline uint16_t read_word (uint8_t partition_number, uint16_t position) { return eeprom_read_word ((const uint16_t*) get_address(partition_number, position)); }
    inline uint32_t read_dword(uint8_t partition_number, uint16_t position) { return eeprom_read_dword((const uint32_t*) get_address(partition_number, position)); }
    inline float    read_float(uint8_t partition_number, uint16_t position) { return eeprom_read_float((const float*)    get_address(partition_number, position)); }

    inline void update_byte (uint8_t partition_number, uint16_t position, uint8_t  value) { eeprom_update_byte ((uint8_t*)  get_address(partition_number, position), value); }
    inline void update_word (uint8_t partition_number, uint16_t position, uint16_t value) { eeprom_update_word ((uint16_t*) get_address(partition_number, position), value); }
    inline void update_dword(uint8_t partition_number, uint16_t position, uint32_t value) { eeprom_update_dword((uint32_t*) get_address(partition_number, position), value); }
    inline void update_float(uint8_t partition_number, uint16_t position, float    value) { eeprom_update_float((float*)    get_address(partition_number, position), value); }

    inline void write_byte (uint8_t partition_number, uint16_t position, uint8_t  value) { eeprom_write_byte ((uint8_t*)  get_address(partition_number, position), value); }
    inline void write_word (uint8_t partition_number, uint16_t position, uint16_t value) { eeprom_write_word ((uint16_t*) get_address(partition_number, position), value); }
    inline void write_dword(uint8_t partition_number, uint16_t position, uint32_t value) { eeprom_write_dword((uint32_t*) get_address(partition_number, position), value); }
    inline void write_float(uint8_t partition_number, uint16_t position, float    value) { eeprom_write_float((float*)    get_address(partition_number, position), value); }
};


template <const uint16_t eeprom_base_address>
bool eeprom_partition_t<eeprom_base_address>::is_valid ()
{
    uint16_t table_address = (uint16_t) get_table_address();
    uint8_t  table_size    =            get_table_size ();
    if (get_hash8_16 (table_size) != eeprom_read_byte ((uint8_t*) (table_address + 1))) return false;
    for (uint8_t i = 0; i < table_size; ++i)
    {
        if ((uint16_t) get_address(i) + get_size(i) >= E2END) return false;
    }
    return true;
}

template <const uint16_t eeprom_base_address>
uint8_t eeprom_partition_t<eeprom_base_address>::get_table_size ()
{
    return eeprom_read_byte ((uint8_t*) get_table_address());
}

template <const uint16_t eeprom_base_address>
void eeprom_partition_t<eeprom_base_address>::set_table_size (uint8_t size)
{
    uint16_t table_address = (uint16_t) get_table_address();
    eeprom_update_byte ((uint8_t*)  table_address,      size);
    eeprom_update_byte ((uint8_t*) (table_address + 1), get_hash8_16 (size));
}

template <const uint16_t eeprom_base_address>
void eeprom_partition_t<eeprom_base_address>::set_address (uint8_t partition_number, void* address, uint16_t size)
{
    uint16_t table_address = (uint16_t) get_table_address();
    eeprom_update_word ((uint16_t*)(table_address + EEPROM_PARTITION_POSITION_ADDRESS(partition_number)), (uint16_t) address);
    eeprom_update_word ((uint16_t*)(table_address + EEPROM_PARTITION_POSITION_SIZE   (partition_number)),            size);
}

template <const uint16_t eeprom_base_address>
uint16_t eeprom_partition_t<eeprom_base_address>::get_size (uint8_t partition_number)
{
    uint16_t table_address = (uint16_t) get_table_address();
    return eeprom_read_word ((uint16_t*)(table_address + EEPROM_PARTITION_POSITION_SIZE (partition_number)));
}

#endif // PARTITION_EEPROM_H 
