#ifndef HELPER_H
#define HELPER_H
#include <avr/pgmspace.h>

char* strcpy_2P (char* destination, const char* source1, const char* source2)
{
    strcpy_P (destination, source1);
    strcat_P (destination, source2);
    return destination;
}

#endif // HELPER_H 
