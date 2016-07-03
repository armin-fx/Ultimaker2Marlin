#ifndef PARTITION_EEPROM_H
#define PARTITION_EEPROM_H

#include <inttypes.h>

#include "limiter.h"

// TODO later implementation will regard a black list of addresses
//      then it will move the eeprom partition around the black list when needed

uint16_t xorshift16 (uint16_t value);
uint32_t xorshift32 (uint32_t value);
uint64_t xorshift64 (uint32_t value);

#define MAGIC_CONSTANT_16 (0x517C + 1)

inline uint8_t  get_hash8_16  (uint16_t value)  { return (uint8_t)  xorshift16 (value + MAGIC_CONSTANT_16); }
inline uint16_t get_hash16_16 (uint16_t value)  { return (uint16_t) xorshift16 (value + MAGIC_CONSTANT_16); }

void copy_eeprom (void* target, void* source, uint16_t size);
void move_eeprom (void* target, void* source, uint16_t size);

// return true if both address overlap
bool is_address_overlap (void *address_A, size_t size_A, void *address_B, size_t size_B);

// check for base address with hash value
bool check_eeprom_base_address (void* base);
// copy base address to target
inline
void copy_eeprom_base_address  (void* base, void* source)  { move_eeprom (base, source, 3); }
//
void write_eeprom_base_tripel (void* base, uint16_t partition, uint8_t hash);
// set base address and create the hash value
inline void write_eeprom_base_address (void* base, void* partition)
{   write_eeprom_base_tripel (base, (uint16_t) partition, get_hash8_16 ((uint16_t) partition)); }
// make the base address invalid
inline void scratch_eeprom_base_address (void* base)
{   write_eeprom_base_tripel (base, 0xFFFF, 0xFF); }

#define EEPROM_PARTITION_VECTOR_OFFSET 2
#define EEPROM_PARTITION_ENTRY_SIZE    5
#define EEPROM_PARTITION_POSITION(partition_number)         (EEPROM_PARTITION_VECTOR_OFFSET     + (EEPROM_PARTITION_ENTRY_SIZE * (partition_number)))
#define EEPROM_PARTITION_POSITION_ADDRESS(partition_number) (EEPROM_PARTITION_VECTOR_OFFSET     + (EEPROM_PARTITION_ENTRY_SIZE * (partition_number)))
#define EEPROM_PARTITION_POSITION_SIZE(partition_number)    (EEPROM_PARTITION_VECTOR_OFFSET + 2 + (EEPROM_PARTITION_ENTRY_SIZE * (partition_number)))
#define EEPROM_PARTITION_POSITION_TYPE(partition_number)    (EEPROM_PARTITION_VECTOR_OFFSET + 4 + (EEPROM_PARTITION_ENTRY_SIZE * (partition_number)))
#define EEPROM_PARTITION_GET_SIZE_WHEN(entries) (EEPROM_PARTITION_VECTOR_OFFSET + EEPROM_PARTITION_ENTRY_SIZE * (entries))

template <const uint16_t eeprom_base_address>
class eeprom_partition_t
{
protected:
    uint16_t* blacklist;
private:
    bool is_no_overlap (uint16_t table_address, uint8_t  table_size);
public:
    eeprom_partition_t ()                           { set_blacklist (0); }
    eeprom_partition_t (const uint16_t* _blacklist) { set_blacklist (_blacklist); }

    // set eeprom blacklist in program space (0 = no blacklist)
    void set_blacklist (const uint16_t* _blacklist) { blacklist = (uint16_t*) _blacklist; }
    // return true if address is not in black list
    bool check_with_blacklist (void* address, uint16_t size);

    // return true if partition list in good state
    bool is_valid ();

    // number of entries
    uint8_t get_table_size ();
    void    set_table_size (uint8_t size);
    //
    // get address of partition list
    void*   get_table_address ();
    //
    // get the partition number of a specific partition type
    uint8_t get_partition_number (uint8_t type, uint8_t type_count=0);
    // true if type exists
    bool    exist_type (uint8_t type, uint8_t type_count=0);

    // delete the complete partition list
    void clear();
    // add a partition with specific address at the end
    bool append (uint8_t type, uint16_t size, void* address);
    // add a partition with specific address
    bool insert (uint8_t partition_number, uint8_t type, uint16_t size, void* address);
    // delete a partition
    void remove (uint8_t partition_number);
    // resize a partition
    bool resize (uint8_t partition_number, uint16_t new_size);
    // resize a partition with specific type
    bool resize_type (uint8_t type, uint16_t new_size, uint8_t type_count=0);

    // write partition list entry without checks
    void write_entry (uint8_t partition_number, uint8_t type, uint16_t size, void* address);

    // size of partition number
    uint16_t get_size (uint8_t partition_number);
    // size of partition with specific type
    uint16_t get_size_type (uint8_t type, uint8_t type_count=0);
    // type of partition number
    uint8_t  get_type (uint8_t partition_number);
    // address of partition number
    void*    get_address (uint8_t partition_number, uint16_t position=0);
    // address of partition with specific type
    void*    get_address_type (uint8_t type, uint16_t position=0, uint8_t type_count=0);

    inline uint8_t  read_byte (uint8_t partition_number, uint16_t position) { return eeprom_read_byte ((const uint8_t*)  get_address(partition_number, position)); }
    inline uint16_t read_word (uint8_t partition_number, uint16_t position) { return eeprom_read_word ((const uint16_t*) get_address(partition_number, position)); }
    inline uint32_t read_dword(uint8_t partition_number, uint16_t position) { return eeprom_read_dword((const uint32_t*) get_address(partition_number, position)); }
    inline float    read_float(uint8_t partition_number, uint16_t position) { return eeprom_read_float((const float*)    get_address(partition_number, position)); }

    inline void update_byte (uint8_t partition_number, uint16_t position, uint8_t  value) { eeprom_update_byte ((uint8_t*)  get_address(partition_number, position), value); }
    inline void update_word (uint8_t partition_number, uint16_t position, uint16_t value) { eeprom_update_word ((uint16_t*) get_address(partition_number, position), value); }
    inline void update_dword(uint8_t partition_number, uint16_t position, uint32_t value) { eeprom_update_dword((uint32_t*) get_address(partition_number, position), value); }
    inline void update_float(uint8_t partition_number, uint16_t position, float    value) { eeprom_update_float((float*)    get_address(partition_number, position), value); }

    inline void write_byte  (uint8_t partition_number, uint16_t position, uint8_t  value) { eeprom_write_byte  ((uint8_t*)  get_address(partition_number, position), value); }
    inline void write_word  (uint8_t partition_number, uint16_t position, uint16_t value) { eeprom_write_word  ((uint16_t*) get_address(partition_number, position), value); }
    inline void write_dword (uint8_t partition_number, uint16_t position, uint32_t value) { eeprom_write_dword ((uint32_t*) get_address(partition_number, position), value); }
    inline void write_float (uint8_t partition_number, uint16_t position, float    value) { eeprom_write_float ((float*)    get_address(partition_number, position), value); }

    inline uint8_t  read_byte_type (uint8_t type, uint16_t position, uint8_t type_count=0) { size_t address = (size_t) get_address_type(type, position, type_count); return (address<E2END) ? eeprom_read_byte ((const uint8_t*)  address) : 0; }
    inline uint16_t read_word_type (uint8_t type, uint16_t position, uint8_t type_count=0) { size_t address = (size_t) get_address_type(type, position, type_count); return (address<E2END) ? eeprom_read_word ((const uint16_t*) address) : 0; }
    inline uint32_t read_dword_type(uint8_t type, uint16_t position, uint8_t type_count=0) { size_t address = (size_t) get_address_type(type, position, type_count); return (address<E2END) ? eeprom_read_dword((const uint32_t*) address) : 0; }
    inline float    read_float_type(uint8_t type, uint16_t position, uint8_t type_count=0) { size_t address = (size_t) get_address_type(type, position, type_count); return (address<E2END) ? eeprom_read_float((const float*)    address) : 0; }

    inline void update_byte_type (uint8_t type, uint16_t position, uint8_t  value, uint8_t type_count=0) { size_t address = (size_t) get_address_type(type, position, type_count); if (address<E2END) eeprom_update_byte ((uint8_t*)  address, value); }
    inline void update_word_type (uint8_t type, uint16_t position, uint16_t value, uint8_t type_count=0) { size_t address = (size_t) get_address_type(type, position, type_count); if (address<E2END) eeprom_update_word ((uint16_t*) address, value); }
    inline void update_dword_type(uint8_t type, uint16_t position, uint32_t value, uint8_t type_count=0) { size_t address = (size_t) get_address_type(type, position, type_count); if (address<E2END) eeprom_update_dword((uint32_t*) address, value); }
    inline void update_float_type(uint8_t type, uint16_t position, float    value, uint8_t type_count=0) { size_t address = (size_t) get_address_type(type, position, type_count); if (address<E2END) eeprom_update_float((float*)    address, value); }

    inline void write_byte_type  (uint8_t type, uint16_t position, uint8_t  value, uint8_t type_count=0) { size_t address = (size_t) get_address_type(type, position, type_count); if (address<E2END) eeprom_write_byte  ((uint8_t*)  address, value); }
    inline void write_word_type  (uint8_t type, uint16_t position, uint16_t value, uint8_t type_count=0) { size_t address = (size_t) get_address_type(type, position, type_count); if (address<E2END) eeprom_write_word  ((uint16_t*) address, value); }
    inline void write_dword_type (uint8_t type, uint16_t position, uint32_t value, uint8_t type_count=0) { size_t address = (size_t) get_address_type(type, position, type_count); if (address<E2END) eeprom_write_dword ((uint32_t*) address, value); }
    inline void write_float_type (uint8_t type, uint16_t position, float    value, uint8_t type_count=0) { size_t address = (size_t) get_address_type(type, position, type_count); if (address<E2END) eeprom_write_float ((float*)    address, value); }
};


template <const uint16_t eeprom_base_address>
bool eeprom_partition_t<eeprom_base_address>::is_valid ()
{
    uint16_t table_address = (uint16_t) get_table_address();
    uint8_t  table_size    =            get_table_size ();
    // check hash value
    if (get_hash8_16 (table_size) != eeprom_read_byte ((uint8_t*) (table_address + 1))) return false;
    //
    void*    address;
    uint16_t size;
    for (uint8_t i = 0; i < table_size; ++i)
    {
        address = get_address(i);
        size    = get_size(i);
        // check partition range is in EEPROM range
        if ((uint16_t) address + size >= E2END) return false;
        // check entries with blacklist
        if (! check_with_blacklist (address, size)) return false;
    }
    // check partition entries don't overlap
    if (! is_no_overlap(table_address, table_size)) return false;
    //
    return true;
}

template <const uint16_t eeprom_base_address>
bool eeprom_partition_t<eeprom_base_address>::is_no_overlap (uint16_t table_address, uint8_t  table_size)
{
    if (table_size < 2) return true;

    uint8_t i, k;
    for (i = 0; i<table_size-1; ++i)
    {
        for (k=i+1; k < table_size; ++k)
        {
            if (is_address_overlap (get_address(i), get_size(i),
                                    get_address(k), get_size(k)))
                return false;
        }
    }
    return true;
}

template <const uint16_t eeprom_base_address>
bool eeprom_partition_t<eeprom_base_address>::check_with_blacklist (void* address, uint16_t size)
{
    uint16_t blacklist_address;
    uint16_t blacklist_size;
    uint8_t n = 0;
    do
    {
        blacklist_address = pgm_read_word (blacklist + n++);
        blacklist_size    = pgm_read_word (blacklist + n++);
        if (is_address_overlap(address, size, (void*)blacklist_address, blacklist_size))
            return false;
    }
    while (blacklist_address != 0 || blacklist_size != 0);
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
inline
void*   eeprom_partition_t<eeprom_base_address>::get_table_address ()
{
    return (void*) eeprom_read_word ((const uint16_t*) eeprom_base_address);
}

template <const uint16_t eeprom_base_address>
inline
uint8_t eeprom_partition_t<eeprom_base_address>::get_partition_number (uint8_t type, uint8_t type_count)
{
    uint8_t  table_size    =            get_table_size ();
    uint16_t table_address = (uint16_t) get_table_address();
    uint8_t  partition_type_current;
    uint8_t  partition_type_count_nr = 0;
    for (uint8_t partition_number=0; partition_number<table_size; ++partition_number)
    {
        partition_type_current = eeprom_read_byte((const uint8_t*)( (uint16_t)table_address + EEPROM_PARTITION_POSITION_TYPE(partition_number)));
        if (partition_type_current == type)
        {
            if (partition_type_count_nr == type_count)
            {
                return partition_number;
            }
            else
            {
                ++partition_type_count_nr;
            }
        }
    }
    return table_size;
}

template <const uint16_t eeprom_base_address>
inline
bool eeprom_partition_t<eeprom_base_address>::exist_type (uint8_t type, uint8_t type_count)
{
    if (get_partition_number(type, type_count) == get_table_size()) return false;
    return true;
}

template <const uint16_t eeprom_base_address>
inline
void* eeprom_partition_t<eeprom_base_address>::get_address (uint8_t partition_number, uint16_t position)
{
    uint16_t vector_adress = eeprom_read_word((const uint16_t*)( (uint16_t)get_table_address() + EEPROM_PARTITION_POSITION_ADDRESS(partition_number)));
    return (void*) (vector_adress + position);
}

template <const uint16_t eeprom_base_address>
inline
void* eeprom_partition_t<eeprom_base_address>::get_address_type (uint8_t type, uint16_t position, uint8_t type_count)
{
    uint8_t  table_size       = get_table_size ();
    uint8_t  partition_number = get_partition_number (type, type_count);
    if (partition_number < table_size)
    {
        uint16_t vector_adress = eeprom_read_word((const uint16_t*)( (uint16_t)get_table_address() + EEPROM_PARTITION_POSITION_ADDRESS(partition_number)));
        return (void*) (vector_adress + position);
    }
    return (void*) E2END;
}

template <const uint16_t eeprom_base_address>
uint16_t eeprom_partition_t<eeprom_base_address>::get_size (uint8_t partition_number)
{
    if (partition_number >= get_table_size()) return 0;
    uint16_t table_address = (uint16_t) get_table_address();
    return eeprom_read_word ((uint16_t*)(table_address + EEPROM_PARTITION_POSITION_SIZE (partition_number)));
}

template <const uint16_t eeprom_base_address>
inline
uint16_t eeprom_partition_t<eeprom_base_address>::get_size_type (uint8_t type, uint8_t type_count)
{
    return get_size (get_partition_number (type, type_count));
}

template <const uint16_t eeprom_base_address>
uint8_t eeprom_partition_t<eeprom_base_address>::get_type (uint8_t partition_number)
{
    if (partition_number >= get_table_size()) return 0;
    uint16_t table_address = (uint16_t) get_table_address();
    return eeprom_read_byte ((uint8_t*)(table_address + EEPROM_PARTITION_POSITION_TYPE (partition_number)));
}

template <const uint16_t eeprom_base_address>
void eeprom_partition_t<eeprom_base_address>::write_entry (uint8_t partition_number, uint8_t type, uint16_t size, void* address)
{
    uint16_t table_address = (uint16_t) get_table_address();
    eeprom_update_word ((uint16_t*)(table_address + EEPROM_PARTITION_POSITION_ADDRESS(partition_number)), (uint16_t) address);
    eeprom_update_word ((uint16_t*)(table_address + EEPROM_PARTITION_POSITION_SIZE   (partition_number)),            size);
    eeprom_update_byte ((uint8_t*) (table_address + EEPROM_PARTITION_POSITION_TYPE   (partition_number)),            type);
}

template <const uint16_t eeprom_base_address>
inline
void eeprom_partition_t<eeprom_base_address>::clear()
{
    set_table_size (0);
}

template <const uint16_t eeprom_base_address>
inline
bool eeprom_partition_t<eeprom_base_address>::append (uint8_t type, uint16_t size, void* address)
{
    return insert (get_table_size(), type, size, address);
}

template <const uint16_t eeprom_base_address>
bool eeprom_partition_t<eeprom_base_address>::insert (uint8_t partition_number, uint8_t type, uint16_t size, void* address)
{ // TODO check for black list
    uint8_t table_size = get_table_size ();
    if (partition_number >  table_size) return false;
    if (table_size != 0)
    {
        if (partition_number < table_size)
        {
            move_eeprom ((void*) EEPROM_PARTITION_POSITION(partition_number + 1),
                         (void*) EEPROM_PARTITION_POSITION(partition_number),
                         EEPROM_PARTITION_ENTRY_SIZE * (table_size - partition_number)
                        );
        }
    }
    write_entry    (partition_number, type, size, address);
    set_table_size (table_size + 1);
    return true;
}

template <const uint16_t eeprom_base_address>
void eeprom_partition_t<eeprom_base_address>::remove (uint8_t partition_number)
{
    uint8_t table_size = get_table_size ();
    if (partition_number >= table_size) return;
    //
    if (table_size == 0) return;
    if (table_size > 1)
    {
        if (partition_number < table_size-1)
        {
            move_eeprom ((void*) EEPROM_PARTITION_POSITION(partition_number),
                         (void*) EEPROM_PARTITION_POSITION(partition_number + 1),
                         EEPROM_PARTITION_ENTRY_SIZE * (table_size-1 - partition_number)
                        );
        }
    }
    set_table_size (table_size - 1);
}

template <const uint16_t eeprom_base_address>
bool eeprom_partition_t<eeprom_base_address>::resize (uint8_t partition_number, uint16_t new_size)
{ // TODO check for black list
    if (partition_number >= get_table_size()) return false;
    eeprom_update_word ((uint16_t*)((uint16_t)get_table_address() + EEPROM_PARTITION_POSITION_SIZE (partition_number)), new_size);
    return true;
}

template <const uint16_t eeprom_base_address>
inline
bool eeprom_partition_t<eeprom_base_address>::resize_type (uint8_t type, uint16_t new_size, uint8_t type_count)
{
    return resize (get_partition_number(type, type_count), new_size);
}

#endif // PARTITION_EEPROM_H 
