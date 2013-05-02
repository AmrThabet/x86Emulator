#define __DISASM__ 1

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
#include "../x86emu.h"
#include "disassembler.h"
#define REGISTERS_LENGTH 8
#define RM_SIZES_LENGTH  3
#define SEGEMENTS_LENGTH 6
#define PREFIXES_LENGTH  3

// ------------------------------------------------------------------------------------------------------------//
// -----                                                                                                  -----//
// -----                                                                                                  -----//
// -----                                     The Disassembler                                             -----//
// -----                                                                                                  -----//
// -----                                                                                                  -----//
// ------------------------------------------------------------------------------------------------------------//
DISASM_INSTRUCTION * System::disasm(DISASM_INSTRUCTION * bIns, char * ins_bytes)
{
    // DISASM_INSTRUCTION* bIns=(DISASM_INSTRUCTION*)malloc(sizeof(DISASM_INSTRUCTION));
    memset(bIns, 0, sizeof(DISASM_INSTRUCTION));
    hde32_disasm(ins_bytes, (hde32s *) &bIns->hde);

    unsigned int n; // represent the temp place to save flags
    // Finding the opcode
    for (unsigned int i = (unsigned int)(bIns->hde.opcode & 0xFF); i < dis_entries; i++)
	{
        if ((bIns->hde.opcode == 0x0F) && (i < 500))
		{
            i = 500 - 1;
            continue;
        }
        if (!(bIns->hde.opcode == 0x0F))
		{
            if ((FlagTable[i].opcode == (unsigned int)(bIns->hde.opcode & 0xFF)) && !(FlagTable[i].flags & OP_0F))
			{
                if (FlagTable[i].reg > 8 & i == bIns->hde.opcode)
				{
                    i = FlagTable[i].reg - 1;
                    continue;
                }
                if (FlagTable[i].flags & OP_GROUP)
				{
                    if (FlagTable[i].flags & OP_FPU)
					{
                        // cout <<(int*)bIns->hde.modrm_reg <<"          "<< (int*)FlagTable[i].reg << "\n";
                        if ((bIns->hde.modrm < 0xC0) && (FlagTable[i].reg == bIns->hde.modrm_reg)) {
                            // cout << (int*)bIns->hde.modrm_reg << "\n";
                            bIns->entry = i;
                            goto Opcode_Found;
                        } else if ((bIns->hde.modrm >= 0xC0) && (FlagTable[i].reg == bIns->hde.modrm)) {
                            bIns->entry = i;
							
                            goto Opcode_Found;
                        }
                    } else if (FlagTable[i].reg == bIns->hde.modrm_reg) {
                        bIns->entry = i;
                        goto Opcode_Found;
                    }
                } else {
                    bIns->entry = i;
					
                    goto Opcode_Found;
                }
            }
        } else {
            if ((FlagTable[i].opcode == bIns->hde.opcode2) && (FlagTable[i].flags & OP_0F)) {
                if (FlagTable[i].flags & OP_GROUP) {
                    if (FlagTable[i].reg == bIns->hde.modrm_reg) {
                        bIns->entry = i;
                        goto Opcode_Found;
                    }
                } else {
                    bIns->entry = i;
                    goto Opcode_Found;
                }
            }
        }
    }
    bIns->emu_func = undefined_opcode;
    bIns->flags    = INS_UNDEFINED;
    return bIns;
Opcode_Found:
    bIns->opcode   = &FlagTable[bIns->entry].mnemonics;
    bIns->emu_func = FlagTable[bIns->entry].emu_func;
	bIns->category = FlagTable[bIns->entry].category;
    n              = FlagTable[bIns->entry].flags;
    // ------------------------------------------------
    // FPU Instructions:
	
    if (n & OP_FPU)
	{
        bIns->flags = n;
        if (n & FPU_DEST_STi)
		{
            bIns->ndest = (n & 0xf);
        }
		else if (n & FPU_SRC_STi)
		{
            bIns->nsrc = (n & 0xf);
        }

	// ------------------------------------------------
	// Setting the flags
    }
	else
	{
        // opcode R/M , R
        if (n & OP_RM_R)
		{
            if (bIns->hde.modrm_mod == 3)
			{
                bIns->flags |= DEST_REG | SRC_REG;
                bIns->ndest  = bIns->hde.modrm_rm;
                bIns->nsrc   = bIns->hde.modrm_reg;
            } else {
                bIns->flags |= DEST_RM | SRC_REG;
                bIns->nsrc   = bIns->hde.modrm_reg;
                // ndest ==0 because it depends on the modrm resolver
            }

            // opcode R , R/M
        } else if (n & OP_R_RM)
		{
            if (bIns->hde.modrm_mod == 3)
			{
                bIns->flags |= DEST_REG | SRC_REG;
                bIns->nsrc   = bIns->hde.modrm_rm;
                bIns->ndest  = bIns->hde.modrm_reg;
            }
			else
			{
                bIns->flags |= DEST_REG | SRC_RM;
                bIns->ndest  = bIns->hde.modrm_reg;
                // nsrc ==0 because it depends on the modrm resolver
            }

            // opcode r/m, imm
        } else if (n & OP_RM_IMM)
		{
            if (bIns->hde.modrm_mod == 3)
			{
                bIns->flags |= DEST_REG | SRC_IMM;
                // int x = (n & 0xFF)                   //take only one byte (that have the registers)
                bIns->ndest = bIns->hde.modrm_rm;
            }
			else
			{
                bIns->flags |= DEST_RM | SRC_IMM;
            }
            if (bIns->hde.flags & F_IMM8)
			{
                bIns->flags |= SRC_BITS8;
                bIns->nsrc   = bIns->hde.imm.imm8;
            }
			else if (bIns->hde.flags & F_IMM16)
			{
                bIns->flags |= SRC_BITS16;
                bIns->nsrc   = bIns->hde.imm.imm16;
            }
			else
			{
                bIns->flags |= SRC_BITS32; // surely the whole instruction will be like this
                bIns->nsrc   = bIns->hde.imm.imm32;
            }

            // opcode reg,imm
        } else if (n & OP_R_IMM) {
            bIns->flags |= DEST_REG | SRC_IMM;
            unsigned int x = (n & 0xFF); // take only one byte (that have the registers
            for (unsigned int i = 0; i < 8; i++) {
                x = x >> 1;
                if (x == 1) {
                    bIns->ndest = i + 1;
                    break;
                }
            }
            if (bIns->hde.flags & F_IMM8) {
                bIns->flags |= SRC_BITS8;
                bIns->nsrc   = bIns->hde.imm.imm8;
            } else if (bIns->hde.flags & F_IMM16) {
                bIns->flags |= SRC_BITS16;
                bIns->nsrc   = bIns->hde.imm.imm16;
            } else {
                bIns->flags |= SRC_BITS32 | DEST_BITS32; // surely the whole instruction will be like this
                bIns->nsrc   = bIns->hde.imm.imm32;
            }

            // opcode reg
        } else if (n & OP_REG_ONLY) {
            bIns->flags |= DEST_REG | SRC_NOSRC;
            unsigned int x = (n & 0xFF); // take only one byte (that have the registers
            for (unsigned int i = 0; i < 8; i++) {
                x = x >> 1;
                if (x == 1) {
                    bIns->ndest = i + 1;
                    break;
                }
            }

            // opcode r/m
        } else if (n & OP_RM_ONLY) {
            if (bIns->hde.modrm_mod == 3) {
                bIns->flags |= DEST_REG | SRC_NOSRC;
                bIns->ndest  = bIns->hde.modrm_rm;
            } else {
                bIns->flags |= DEST_RM | SRC_NOSRC;
            }

            // opcode imm
        } else if (n & OP_IMM_ONLY) {
            bIns->flags |= DEST_IMM | SRC_NOSRC;
            if (bIns->hde.flags & F_IMM8) {
                bIns->flags |= DEST_BITS8;
                bIns->ndest  = bIns->hde.imm.imm8;
            } else if (bIns->hde.flags & F_IMM16) {
                bIns->flags |= DEST_BITS16;
                bIns->ndest  = bIns->hde.imm.imm16;
            } else {
                bIns->flags |= DEST_BITS32 | SRC_BITS32; // surely the whole instruction will be like this
                bIns->ndest  = bIns->hde.imm.imm32;
            }
        }
        // not the operand size (32 /16/8)
        if (n & OP_BITS32) { // || ((bIns->hde.flags & F_PREFIX_67) && bIns->hde.opcode>=0xA0 && bIns->hde.opcode<=0xA3)
            if ((bIns->hde.flags & F_PREFIX_66)) {
                bIns->flags |= DEST_BITS16;
            }
            // bits8 could be settled from imm8 ****
            if (!(bIns->flags & DEST_BITS8) && !(bIns->flags & DEST_BITS16)) {
                bIns->flags |= DEST_BITS32;
            }
            if (!(bIns->flags & SRC_BITS8) && !(bIns->flags & SRC_BITS16)) {
                bIns->flags |= SRC_BITS32;
            }
        } else if (n & OP_BITS8) {
            bIns->flags |= DEST_BITS8 | SRC_BITS8;
        } else if ((bIns->hde.flags & F_PREFIX_66)) { // || ((bIns->hde.flags & F_PREFIX_67) && bIns->hde.opcode>=0xA0 && bIns->hde.opcode<=0xA3)
            bIns->flags |= DEST_BITS16;
        }
    }
    // ----------------------------------------------------------
    // ModRM Resolver

    // if there's a sib so there's a modrm :)
    if (bIns->hde.flags & F_SIB) {
        if (bIns->hde.sib_index != 4) { // be sure it's not none (at index=4-->none)
            bIns->modrm.items[bIns->modrm.length] = bIns->hde.sib_index;
            bIns->modrm.flags[bIns->modrm.length] = RM_REG;
            if (bIns->hde.sib_scale == 1) { // caculate the scale (exx*2/exx*4)
                bIns->modrm.flags[bIns->modrm.length] |= RM_MUL2;
            } else if (bIns->hde.sib_scale == 2) {
                bIns->modrm.flags[bIns->modrm.length] |= RM_MUL4;
            } else if (bIns->hde.sib_scale == 3) {
                bIns->modrm.flags[bIns->modrm.length] |= RM_MUL8;
            }
            bIns->modrm.length++;
        }
        if ((bIns->hde.sib_base != 5) || (bIns->hde.modrm_mod != 0)) { // not exx*2 +disp32
            bIns->modrm.items[bIns->modrm.length] = bIns->hde.sib_base;
            bIns->modrm.flags[bIns->modrm.length] = RM_REG;
            bIns->modrm.length++;
        }
    } else if (bIns->hde.flags & F_MODRM)
	{
        if (bIns->hde.flags & F_PREFIX_67) { // 16 bit addressing
            if (((bIns->hde.modrm_mod != 0) || (bIns->hde.modrm_rm != 6))) { // not disp16 only
                unsigned int reg1 = 0, reg2 = 0, nregs = 0;
                if (bIns->hde.modrm_rm == 0) {
                    reg1  = 3;
                    reg2  = 6;
                    nregs = 2;
                } else if (bIns->hde.modrm_rm == 1) {
                    reg1  = 3;
                    reg2  = 7;
                    nregs = 2;
                } else if (bIns->hde.modrm_rm == 2) {
                    reg1  = 5;
                    reg2  = 6;
                    nregs = 2;
                } else if (bIns->hde.modrm_rm == 3) {
                    reg1  = 5;
                    reg2  = 7;
                    nregs = 2;
                } else if (bIns->hde.modrm_rm == 4) {
                    reg1  = 6;
                    reg2  = 0;
                    nregs = 1;
                } else if (bIns->hde.modrm_rm == 6) {
                    reg1  = 7;
                    reg2  = 0;
                    nregs = 1;
                } else if (bIns->hde.modrm_rm == 7) {
                    reg1  = 3;
                    reg2  = 0;
                    nregs = 1;
                }
                bIns->modrm.items[bIns->modrm.length] = reg1;
                bIns->modrm.flags[bIns->modrm.length] = RM_REG | RM_ADDR16;
                bIns->modrm.length++;
                if (nregs == 2) {
                    bIns->modrm.items[bIns->modrm.length] = reg2;
                    bIns->modrm.flags[bIns->modrm.length] = RM_REG | RM_ADDR16;
                    bIns->modrm.length++;
                }
            }
        } else {
            if (((bIns->hde.modrm_mod != 0) || (bIns->hde.modrm_rm != 5)) && (bIns->hde.modrm_mod != 3)) { // not disp32 only
                bIns->modrm.items[bIns->modrm.length] = bIns->hde.modrm_rm;
                bIns->modrm.flags[bIns->modrm.length] = RM_REG;
                bIns->modrm.length++;
            }
        }
    }
    // testing the displacement
    if (bIns->hde.flags & F_DISP8) {
        bIns->modrm.items[bIns->modrm.length]  = bIns->hde.disp.disp8;
        bIns->modrm.flags[bIns->modrm.length]  = RM_DISP;
        bIns->modrm.flags[bIns->modrm.length] |= RM_DISP8;
        bIns->modrm.length++;
    } else if (bIns->hde.flags & F_DISP16) {
        bIns->modrm.items[bIns->modrm.length]  = bIns->hde.disp.disp16;
        bIns->modrm.flags[bIns->modrm.length]  = RM_DISP;
        bIns->modrm.flags[bIns->modrm.length] |= RM_DISP16;
        bIns->modrm.length++;
    } else if (bIns->hde.flags & F_DISP32) {
        bIns->modrm.items[bIns->modrm.length]  = bIns->hde.disp.disp32;
        bIns->modrm.flags[bIns->modrm.length]  = RM_DISP;
        bIns->modrm.flags[bIns->modrm.length] |= RM_DISP32;
        bIns->modrm.length++;
    }

    // ---------------------------------------------------------------------------
    // Special Cases

    // mov
    if ((bIns->hde.opcode >= 0xa0) && (bIns->hde.opcode <= 0xa3))
	{
        bIns->modrm.length   = 1;
        bIns->modrm.flags[0] = RM_DISP | RM_DISP32;
        bIns->modrm.items[0] = bIns->hde.imm.imm32;
    
    } // shl ecx,1
	else if ((bIns->hde.opcode >= 0xD0) && (bIns->hde.opcode <= 0xD1))
	{
        bIns->nsrc = 1;
    } 
	else if ((bIns->hde.opcode >= 0x90) && (bIns->hde.opcode <= 0x97))
	{
        bIns->nsrc   = 0;
        bIns->flags |= SRC_REG | SRC_BITS32;
        bIns->flags &= ~SRC_NOSRC;
        
    } // shr Ev,cl
	else if ((bIns->hde.opcode == 0xD2) || (bIns->hde.opcode == 0xD3))
	{
        bIns->flags |= SRC_REG | SRC_BITS8;
        bIns->nsrc   = 1;       // cl
        
    } // for add & and & or reg32,imm8 or reg16,imm8 with negative imm
	else if (bIns->flags & SRC_BITS8 && bIns->flags & SRC_IMM && (bIns->nsrc > 0x80)) {
        if (bIns->flags & DEST_BITS32) {
            bIns->nsrc  += 0xFFFFFF00;
            bIns->flags &= ~SRC_BITS8;
            bIns->flags |= SRC_BITS32;
        } else if (bIns->flags & DEST_BITS16) {
            bIns->nsrc  += 0xFF00;
            bIns->flags &= ~SRC_BITS8;
            bIns->flags |= SRC_BITS16;
        }
       
    } // enter imm16,imm8
	else if (bIns->hde.opcode == 0xC8) {
        bIns->flags = DEST_BITS16 | SRC_BITS8 | DEST_IMM | SRC_IMM;
        unsigned short x  = (unsigned short) ins_bytes[1];
        unsigned char  x2 = (unsigned char) ins_bytes[3];
        bIns->ndest = (DWORD) x;
        bIns->nsrc  = (DWORD) x2;
        
    } // imul reg,RM,imm
	else if ((bIns->hde.opcode == 0x69) || (bIns->hde.opcode == 0x6B))
	{
        if (bIns->hde.flags & F_IMM8) {
            bIns->other = bIns->hde.imm.imm8;
        } else if (bIns->hde.flags & F_IMM16) {
            bIns->other = bIns->hde.imm.imm16;
        } else if (bIns->hde.flags & F_IMM32) {
            bIns->other = bIns->hde.imm.imm32;
        }
    }
	else if ((bIns->hde.opcode == 0xC2) || (bIns->hde.opcode == 0xCA))
	{
		bIns->flags &= ~DEST_BITS32;
		bIns->flags |= DEST_BITS16;
	}

    return bIns;
}

// ===================================================================================================================================
DISASM_INSTRUCTION * System::disasm(DISASM_INSTRUCTION * bIns, char * ins_bytes, string & str)
{
    disasm(bIns, ins_bytes);
    unsigned int n = 0;
    str = "";
    // ---------------------------------------------------------------------------
    // FPU
    if (bIns->flags & OP_FPU)
	{
		//we need to implement convert fn instructions into fxxx while using wait
		//if (bIns->hde.flags 
		
        str.append(bIns->opcode->c_str());
        str.append(" ");
        if (bIns->flags & FPU_NULL) {
            return bIns;
        }

        // DEST
        if (bIns->flags & FPU_DEST_ST) {
            str.append("st(0)");
        } else if (bIns->flags & FPU_DEST_STi) {
            unsigned char buff[10] = {0};
            unsigned int  n = sprintf((char*)&buff, "%x", bIns->ndest - 1);
            str.append("st(").append((char*)&buff).append(")");
        } else if (bIns->flags & FPU_DEST_RM) {
            if (bIns->flags & FPU_BITS32) {
                str.append("dword ptr ");
            } else if (bIns->flags & FPU_BITS16) {
                str.append("word ptr ");
            } else if (bIns->flags & FPU_BITS64) {
                str.append("qword ptr ");
            } else if (bIns->flags & FPU_BITS80) {
                str.append("tbyte ptr ");
            }
            str.append("[");
            for (unsigned int i = 0; i < bIns->modrm.length; i++) {
                if (bIns->modrm.flags[i] & RM_REG) {
                    if (i > 0) {
                        str.append(" + ");
                    }
                    if (bIns->modrm.flags[i] & RM_ADDR16) {
                        str.append(reg16[bIns->modrm.items[i]]);
                    } else {
                        str.append(reg32[bIns->modrm.items[i]]);
                    }
                    if (bIns->modrm.flags[i] & RM_MUL2) {
                        str.append("*2");
                    }
                    if (bIns->modrm.flags[i] & RM_MUL4) {
                        str.append("*4");
                    }
                    if (bIns->modrm.flags[i] & RM_MUL8) {
                        str.append("*8");
                    }
                } else if (bIns->modrm.flags[i] & RM_DISP) {
                    string s;
                    if (bIns->modrm.flags[i] & RM_DISP8 && (bIns->modrm.items[i] > 0x80) && (i > 0)) {
                        str.append(" - ");
                        unsigned char buff[10] = {0}; // 1 for safty :)
                        unsigned int  x = 0x100 - bIns->modrm.items[i];
                        n = sprintf((char*)&buff, "%x", x);
                        s = (char*)&buff;
                    } else {
                        if (i > 0) {
                            str.append(" + ");
                        }
                        unsigned char buff[10] = {0}; // 1 for safty :)
                        n = sprintf((char*)&buff, "%x", bIns->modrm.items[i]);
                        s = (char*)&buff;
                    }

                    if ((bIns->modrm.flags[i] & RM_DISP8) && (n > 2)) {
                        s = s.substr(s.size() - 2, s.size());
                    } else if ((bIns->modrm.flags[i] & RM_DISP16) && (n > 4)) {
                        s = s.substr(s.size() - 4, s.size());
                    }
                    str.append(s);
                    str.append("h"); // ecause it's a hexdecimal number
                }
            }
            str.append("]");
        }
        if (bIns->flags & FPU_DEST_ONLY) {
            return bIns;
        }
        // -------
        // SRC
        str.append(", ");
        if (bIns->flags & FPU_SRC_ST)
		{
            str.append("st[0]");
        } 
		else if (bIns->flags & FPU_SRC_STi)
		{
            unsigned char buff[10] = {0};
            unsigned int  n = sprintf((char*)&buff, "%x", bIns->nsrc - 1);
            str.append("st[").append((char*)&buff).append("]");
        }
        return bIns;
        // ---------------------------------------------------------------------------
        // Creating the String
        // stosw & lodsw & so on
    } else {
        if (bIns->flags & INS_UNDEFINED) {
            str = "undefined";
            return bIns;
        }
        if (bIns->hde.flags & F_PREFIX_REPNZ) {
            str.append("repne ");
        }
        if (bIns->hde.flags & F_PREFIX_REPX) {
            str.append("rep ");
        }
        if (bIns->hde.flags & F_PREFIX_LOCK) {
            str.append("lock ");
        }
        str.append(bIns->opcode->c_str());
        str.append(" ");
		if ((str.substr(str.size() - 3, str.size()).compare("sd ") == 0) && bIns->hde.flags & F_PREFIX_66) {
            str = str.substr(0, str.size() - 2);
            str.append("w ");
        }
        if ((bIns->hde.opcode == 0xE3) && (bIns->hde.flags & F_PREFIX_67)) {
            str = "jcxz ";
        }
        // ---------------------------------------------------------------------------
        // Destination

        if (bIns->flags & DEST_REG) {
            if (bIns->flags & DEST_BITS8) {
                str.append(reg8[bIns->ndest]);
            } else if (bIns->flags & DEST_BITS16) {
                str.append(reg16[bIns->ndest]);
            } else if (bIns->flags & DEST_BITS32) {
                str.append(reg32[bIns->ndest]);
            }
            str.append(" ");
        } else if (bIns->flags & DEST_IMM) {
            unsigned char   buff[10] = {0};         // 1 for safty :)
            unsigned int n = sprintf((char*)&buff, "%x", bIns->ndest);
            string s = (char*)&buff;
            if ((bIns->flags & DEST_BITS8) && (n > 2)) {
                s = s.substr(s.size() - 2, s.size());
            } else if ((bIns->flags & DEST_BITS16) && (n > 4)) {
                s = s.substr(s.size() - 4, s.size());
            }
            if (!(compare_array(s, numbers, 10, 1))) {
                s = ((string)("0")).append(s);
            }
            str.append(s);
            str.append("h"); // ecause it's a hexdecimal number
        } else if (bIns->flags & DEST_RM) {
            n = 0;
            if (bIns->flags & DEST_BITS16) {
                n = 1;
            } else if (bIns->flags & DEST_BITS32) {
                n = 2;
            }
            str.append(rm_sizes[n]);
            str.append(" ptr ");
            n = 0;
            if (bIns->hde.flags & F_PREFIX_SEG) {
                if (bIns->hde.p_seg == PREFIX_SEGMENT_CS) {
                    n = 1;
                } else if (bIns->hde.p_seg == PREFIX_SEGMENT_DS) {
                    n = 2;
                } else if (bIns->hde.p_seg == PREFIX_SEGMENT_SS) {
                    n = 3;
                } else if (bIns->hde.p_seg == PREFIX_SEGMENT_ES) {
                    n = 4;
                } else if (bIns->hde.p_seg == PREFIX_SEGMENT_FS) {
                    n = 5;
                } else if (bIns->hde.p_seg == PREFIX_SEGMENT_GS) {
                    n = 6;
                }
                str.append(seg[n - 1]);
                str.append(":");
            }
            str.append("[");
            for (unsigned int i = 0; i < bIns->modrm.length; i++) {
                if (bIns->modrm.flags[i] & RM_REG) {
                    if (i > 0) {
                        str.append(" + ");
                    }
                    if (bIns->modrm.flags[i] & RM_ADDR16) {
                        str.append(reg16[bIns->modrm.items[i]]);
                    } else {
                        str.append(reg32[bIns->modrm.items[i]]);
                    }
                    if (bIns->modrm.flags[i] & RM_MUL2) {
                        str.append("*2");
                    }
                    if (bIns->modrm.flags[i] & RM_MUL4) {
                        str.append("*4");
                    }
                    if (bIns->modrm.flags[i] & RM_MUL8) {
                        str.append("*8");
                    }
                } else if (bIns->modrm.flags[i] & RM_DISP) {
                    string s;
                    if (bIns->modrm.flags[i] & RM_DISP8 && (bIns->modrm.items[i] > 0x80) && (i > 0)) {
                        str.append(" - ");
                        unsigned char buff[10] = {0}; // 1 for safty :)
                        unsigned int  x = 0x100 - bIns->modrm.items[i];
                        n = sprintf((char*)&buff, "%x", x);
                        s = (char*)&buff;
                    } else {
                        if (i > 0) {
                            str.append(" + ");
                        }
                        unsigned char buff[10] = {0}; // 1 for safty :)
                        n = sprintf((char*)&buff, "%x", bIns->modrm.items[i]);
                        s = (char*)&buff;
                    }

                    if ((bIns->modrm.flags[i] & RM_DISP8) && (n > 2)) {
                        s = s.substr(s.size() - 2, s.size());
                    } else if ((bIns->modrm.flags[i] & RM_DISP16) && (n > 4)) {
                        s = s.substr(s.size() - 4, s.size());
                    }
                    str.append(s);
                    str.append("h"); // ecause it's a hexdecimal number
                }
            }
            str.append("]");
        }
        // ---------------------------------------------------------------------------------------------
        // source

        if (!(bIns->flags & SRC_NOSRC) && !(FlagTable[bIns->entry].flags & OP_ANY)) {
            str.append(",");
            if (bIns->flags & SRC_REG) {
                if (bIns->flags & SRC_BITS8 || FlagTable[bIns->entry].flags & OP_SRC8) {
                    str.append(reg8[bIns->nsrc]);
                } else if (bIns->flags & SRC_BITS16 || FlagTable[bIns->entry].flags & OP_SRC16) {
                    str.append(reg16[bIns->nsrc]);
                } else if (bIns->flags & SRC_BITS32) {
                    str.append(reg32[bIns->nsrc]);
                }
                str.append(" ");
            } else if (bIns->flags & SRC_IMM) {
                unsigned char   buff[10] = {0};         // 1 for safty :)
                unsigned int    n = sprintf((char*)&buff, "%x", bIns->nsrc);
                string s = (char*)&buff;
                if ((bIns->flags & SRC_BITS8) && (n > 2)) {
                    s = s.substr(s.size() - 2, s.size());
                } else if ((bIns->flags & SRC_BITS16) && (n > 4)) {
                    s = s.substr(s.size() - 4, s.size());
                }
                if (!(compare_array(s, numbers, 10, 1))) {
					s = string("0").append(s);
                }
                str.append(s);
                str.append("h"); // ecause it's a hexdecimal number
            } else if (bIns->flags & SRC_RM) {
                n = 0;
                if (bIns->flags & SRC_BITS16 || FlagTable[bIns->entry].flags & OP_SRC16) {
                    n = 1;
                } else if (bIns->flags & SRC_BITS32 && !(FlagTable[bIns->entry].flags & OP_SRC8)) {
                    n = 2;
                }
                str.append(rm_sizes[n]);
                str.append(" ptr ");
                n = 0;
                if (bIns->hde.flags & F_PREFIX_SEG) {
                    if (bIns->hde.p_seg == PREFIX_SEGMENT_CS) {
                        n = 1;
                    } else if (bIns->hde.p_seg == PREFIX_SEGMENT_DS) {
                        n = 2;
                    } else if (bIns->hde.p_seg == PREFIX_SEGMENT_SS) {
                        n = 3;
                    } else if (bIns->hde.p_seg == PREFIX_SEGMENT_ES) {
                        n = 4;
                    } else if (bIns->hde.p_seg == PREFIX_SEGMENT_FS) {
                        n = 5;
                    } else if (bIns->hde.p_seg == PREFIX_SEGMENT_GS) {
                        n = 6;
                    }
                    str.append(seg[n - 1]);
                    str.append(":");
                }
                str.append("[");
                for (int i = 0; i < bIns->modrm.length; i++) {
                    if (bIns->modrm.flags[i] & RM_REG) {
                        if (i > 0) {
                            str.append(" + ");
                        }
                        if (bIns->modrm.flags[i] & RM_ADDR16) {
                            str.append(reg16[bIns->modrm.items[i]]);
                        } else {
                            str.append(reg32[bIns->modrm.items[i]]);
                        }
                        if (bIns->modrm.flags[i] & RM_MUL2) {
                            str.append("*2");
                        }
                        if (bIns->modrm.flags[i] & RM_MUL4) {
                            str.append("*4");
                        }
                        if (bIns->modrm.flags[i] & RM_MUL8) {
                            str.append("*8");
                        }
                    } else if (bIns->modrm.flags[i] & RM_DISP) {
                        string s;
                        if (bIns->modrm.flags[i] & RM_DISP8 && (bIns->modrm.items[i] > 0x80) && (i > 0)) {
                            str.append(" - ");
                            unsigned char buff[10] = {0}; // 1 for safty :)
                            unsigned int  x = 0x100 - bIns->modrm.items[i];
                            n = sprintf((char*)&buff, "%x", x);
                            s = (char*)&buff;
                        } else {
                            if (i > 0) {
                                str.append(" + ");
                            }
                            unsigned char buff[10] = {0}; // 1 for safty :)
                            n = sprintf((char*)&buff, "%x", bIns->modrm.items[i]);
                            s = (char*)&buff;
                        }

                        if ((bIns->modrm.flags[i] & RM_DISP8) && (n > 2)) {
                            s = s.substr(s.size() - 2, s.size());
                        } else if ((bIns->modrm.flags[i] & RM_DISP16) && (n > 4)) {
                            s = s.substr(s.size() - 4, s.size());
                        }
                        if (!(compare_array(s, numbers, 10, 1))) {
							s = string("0").append(s);
                        }
                        str.append(s);
                        str.append("h"); // ecause it's a hexdecimal number
                    }
                }
                str.append("]");
            }
        }
        // ------------------------------
        // Imul special case
		if (str.substr(0, 4).compare("imul") == 0)
		{
            unsigned char buff[10] = {0};
            if (bIns->hde.flags & F_IMM8)
			{
                bIns->other = bIns->hde.imm.imm8;
                unsigned int    n = sprintf((char*)&buff, "%x", bIns->other);
                string s = (char*)&buff;
                str.append(",");
                if (n > 2) {
                    s = s.substr(s.size() - 2, s.size());
                }
				str.append(s.append("h"));
            } 
			else if (bIns->hde.flags & F_IMM16)
			{
                bIns->other = bIns->hde.imm.imm16;
                unsigned int    n = sprintf((char*)&buff, "%x", bIns->other);
                string s = (char*)&buff;
                str.append(",");
                if (n > 2)
				{
                    s = s.substr(s.size() - 2, s.size());
                }
				str.append(s.append("h"));
            }
			else if (bIns->hde.flags & F_IMM32)
			{
                bIns->other = bIns->hde.imm.imm32;
                unsigned int    n = sprintf((char*)&buff, "%x", bIns->other);
                string s = (char*)&buff;
                str.append(",");
				str.append(s.append("h"));
            }
        }
    }
    // cout << str << "\n";
    return bIns;
    // cout << str << "\n";
    // bIns->emu_func=undefined_opcode;
    // bIns->flags=INS_INVALID | INS_UNDEFINED;
    // return bIns;
}
