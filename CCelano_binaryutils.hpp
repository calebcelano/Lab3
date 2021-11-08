#pragma once
/* Author: Caleb Celano
 * Date: 31 August 2021
 * Description: this is the first lab/mini project and will act as a bitwise op/c refresher
 * Note: I worked with Kat Meza on this lab.
 */

 // Header Files
#include "stdint.h"
#include "stdio.h"

// Header Guards
#ifndef GAURD
#define GAURD

// Function Declarations
void setbit(uint32_t* addr, uint8_t whichbit);
void clearbit(uint32_t* addr, uint8_t whichbit);
void setbits(uint32_t* addr, uint32_t bitmask);
void clearbits(uint32_t* addr, uint32_t bitmask);
void display_binary(uint32_t num);



// End of Guard
#endif
