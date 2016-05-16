#ifndef HELPER_H
#define HELPER_H
#include <avr/pgmspace.h>

char* strcpy_2P (char* destination, const char* source1, const char* source2);

inline uint8_t  ram_read_byte  (const uint8_t*  address)  { return *address; }
inline uint16_t ram_read_word  (const uint16_t* address)  { return *address; }
inline uint32_t ram_read_dword (const uint32_t* address)  { return *address; }
inline float    ram_read_float (const float*    address)  { return *address; }
//
inline void ram_write_byte  (uint8_t*  address, uint8_t  value) { *address = value; }
inline void ram_write_word  (uint16_t* address, uint16_t value) { *address = value; }
inline void ram_write_dword (uint32_t* address, uint32_t value) { *address = value; }
inline void ram_write_float (float*    address, float    value) { *address = value; }

#endif // HELPER_H 
