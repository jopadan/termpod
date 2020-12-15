#ifndef _CCITT32_CRC_H
#define _CCITT32_CRC_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

extern uint32_t crctab[256];
extern uint32_t ccitt32_updcrc(uint32_t crcinit, unsigned char* data, size_t count);

#endif
