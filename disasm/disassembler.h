#include <iostream>
#include <string.h>
#include <stdlib.h>

using namespace std;
#ifndef __DISASSEMBLER__


#define __DISASSEMBLER__ 1


/*
 *
 *  Copyright (C) 2010-2011 Amr Thabet <amr.thabet@student.alx.edu.eg>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to Amr Thabet 
 *  amr.thabet@student.alx.edu.eg
 *
 */


//int imm_to_dec(string); 
extern string reg32[8];
#ifdef __DISASM__
//int dis_entries;
//FLAGTABLE*  FlagTable;
//extern int dis_entries;
//extern FLAGTABLE FlagTable[512*7];

extern string reg16[8];
extern string  reg8[8];
extern string   seg[6];
extern string rm_sizes[3];
extern string numbers[10];
extern string prefixes[3];
#endif
#endif       
