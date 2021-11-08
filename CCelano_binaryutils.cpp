#include "CCelano_binaryutils.hpp"

//returns nothing, arguments are addr and whichbit, sets bit at position whichbit
void setbit(uint32_t* addr, uint8_t whichbit) {
    //left shift
    *addr |= 1 << whichbit;
}

// returns nothing, arguments are addr and whichbit, only sets bit at position whichbit to 0
void clearbit(uint32_t* addr, uint8_t whichbit) {
    // NOT operator inverts it, AND operator clears the bit
    *addr &= ~(1 << whichbit);
}
// returns nothing, arguments are addr and bitmask, sets only the bits defined in the mask
void setbits(uint32_t* addr, uint32_t bitmask) {
    //left shift bits of bitmask
    *addr |= bitmask;
}
// returns nothing, arguments are addr and bitmask, clears only bits defined in mask
void clearbits(uint32_t* addr, uint32_t bitmask) {
    // AND operator clears the bits in bitmask
    *addr &= ~(bitmask);
}
// returns nothing, argument is num, displays to console binary representation of num
void display_binary(uint32_t num) {
        if (num > 1) {
            //recursion for the MOD-2 method
            display_binary(num / 2);
        }
        // last remainder is most significant digit and first remainder is least
        printf("%d", num % 2);
}