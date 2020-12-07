#ifndef BANK_DATA_H
#define BANK_DATA_H

#include <gb/gb.h>

/**
 * Call set_bkg_data with data stored in banked memory
 * 
 * @param bank bank to read from
 * @param i first tile to write to
 * @param l number of tiles to write
 * @param ptr memory address of tile data within bank
 */
void SetBankedBkgData(UBYTE i, UBYTE l, unsigned char *ptr, UBYTE bank);

/**
 * Call set_sprite_data with data stored in banked memory
 * 
 * @param bank bank to read from
 * @param i first tile to write to
 * @param l number of tiles to write
 * @param ptr memory address of tile data within bank
 */
void SetBankedSpriteData(UBYTE i, UBYTE l, unsigned char *ptr, UBYTE bank);

/**
 * Read UBYTE from banked memory location
 * 
 * @param bank bank to read from
 * @param ptr memory address of data within bank
 * @return value stored in banked location
 */
UBYTE ReadBankedUBYTE(UBYTE bank, unsigned char *ptr);

/**
 * memcpy data from banked memory location
 * 
 * @param bank bank to read from
 * @param to destination to write fetched data
 * @param from memory address of data within bank
 * @param n number of bytes to fetch from bank
 */
void MemcpyBanked(UBYTE bank, void* to, void* from, size_t n);

#endif
