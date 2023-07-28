//A simple file for getting the hand of C

#include <stdint.h>
#include <stddef.h>
#include <math.h>

//This is an example of an alias to a function pointer. In this case, stmdev_write_ptr
//is a pointer to a function that has 4 parameters and returns on int32_t type
typedef int32_t(*stmdev_write_ptr)(void*, uint8_t, const uint8_t*, uint16_t); 

stmdev_write_ptr yo = 12;