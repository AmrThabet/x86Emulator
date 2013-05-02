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
// this file for emulating string opcodes (movs,lods,stos,cmps,scas)

// LODS
int op_lods(Thread & thread, DISASM_INSTRUCTION * s) {
    int rep = 1; // for repeat instructions

    if (s->hde.flags & F_PREFIX_REP) {
        rep           = thread.Exx[1]; // ecx
        thread.Exx[1] = 0; // set it to zero
    }
    // now we will loop the read instruction
    for (int i = 0; i < rep; i++) {
        // one byte
        DWORD * ptr;
        EMU_READ_MEM(ptr, thread.Exx[6]); // esi
		if (s->opcode->substr(0, s->opcode->size()).compare("lodsb") == 0) {
            thread.Exx[0]  = (thread.Exx[0] & 0xFFFFFF00) + (*ptr & 0xFF);
            thread.Exx[6] += 1;
        } else if ((s->opcode->substr(0, s->opcode->size()).compare("lodsd") == 0) && (s->hde.flags & F_PREFIX_66)) {
            thread.Exx[0]  = (thread.Exx[0] & 0xFFFF0000) + (*ptr & 0xFFFF);
            thread.Exx[6] += 2;
        } else if ((s->opcode->substr(0, s->opcode->size()).compare("lodsd") == 0) && !(s->hde.flags & F_PREFIX_66)) {
            thread.Exx[0]  = *ptr;
            thread.Exx[6] += 4;
        }
    }
    return 0;
}

// -----------------------------------------------------------------------------------------------------------------
// STOS
int op_stos(Thread & thread, DISASM_INSTRUCTION * s) {
    int rep = 1; // for repeat instructions

    if (s->hde.flags & F_PREFIX_REP) {
        rep           = thread.Exx[1]; // ecx
        thread.Exx[1] = 0; // set it to zero
    }
    // now we will loop the read instruction
    for (int i = 0; i < rep; i++) {
        // one byte
        if (s->opcode->substr(0, s->opcode->size()).compare("stosb") == 0) {
            EMU_WRITE_MEM(thread.Exx[7], 1, (unsigned char *) &thread.Exx[0]);
            thread.Exx[7] += 1;
        } else if ((s->opcode->substr(0, s->opcode->size()).compare("stosd") == 0) && (s->hde.flags & F_PREFIX_66)) {
            EMU_WRITE_MEM(thread.Exx[7], 2, (unsigned char *) &thread.Exx[0]);
            thread.Exx[7] += 2;
        } else if ((s->opcode->substr(0, s->opcode->size()).compare("stosd") == 0) && !(s->hde.flags & F_PREFIX_66)) {
            EMU_WRITE_MEM(thread.Exx[7], 4, (unsigned char *) &thread.Exx[0]);
            thread.Exx[7] += 4;
        }
    }
    return 0;
}

// -----------------------------------------------------------------------------------------------------------------
// MOVS
int op_movs(Thread & thread, DISASM_INSTRUCTION * s) {
    int rep = 1; // for repeat instructions

    if (s->hde.flags & F_PREFIX_REP) {
        rep           = thread.Exx[1]; // ecx
        thread.Exx[1] = 0; // set it to zero
    }
    // now we will loop the read instruction
    for (int i = 0; i < rep; i++) {
        // one byte
        DWORD * ptr;
        EMU_READ_MEM(ptr, thread.Exx[6]); // esi
        if (s->opcode->substr(0, s->opcode->size()).compare("movsb") == 0) {
            unsigned char n = (unsigned char) (*ptr & 0xFF);
            EMU_WRITE_MEM(thread.Exx[7], 1, (unsigned char *) &n);
            thread.Exx[6] += 1;
            thread.Exx[7] += 1;
        } else if ((s->opcode->substr(0, s->opcode->size()).compare("movsd") == 0) && (s->hde.flags & F_PREFIX_66)) {
            unsigned short n = (unsigned short) (*ptr & 0xFFFF);
            EMU_WRITE_MEM(thread.Exx[7], 2, (unsigned char *) &n);
            thread.Exx[6] += 2;
            thread.Exx[7] += 2;
        } else if ((s->opcode->substr(0, s->opcode->size()).compare("movsd") == 0) && !(s->hde.flags & F_PREFIX_66)) {
            DWORD n = *ptr;
            EMU_WRITE_MEM(thread.Exx[7], 4, (unsigned char *) &n);
            thread.Exx[6] += 4;
            thread.Exx[7] += 4;
        }
        // cout << "the result is = "<< (int*)thread.Exx[0]<< "\n";
        // system("PAUSE");
    }
    return 0;
}

// -----------------------------------------------------------------------------------------------------------------
// CMPS
int op_cmps(Thread & thread, DISASM_INSTRUCTION * s) {
    int  rep     = 1; // for repeat instructions
    bool repe    = false;
    bool ins_rep = false;

    if (s->hde.flags & F_PREFIX_REP) {
        rep     = thread.Exx[1];     // ecx
        ins_rep = true;
        // thread.Exx[1]=0;              //set it to zero
    }
    if (s->hde.flags & F_PREFIX_REPX) {
        repe = true;
    }
    // now we will loop the read instruction
    for (int i = 0; i < rep; i++) {
        // one byte
        DWORD * src;
        DWORD * dest;
        EMU_READ_MEM(src,  thread.Exx[6]); // esi
        EMU_READ_MEM(dest, thread.Exx[7]); // edi
        // cout << (int*)*dest<<"\n";
        DWORD result;
        if (s->opcode->substr(0, s->opcode->size()).compare("cmpsb") == 0) {
            result = (*src & 0xFF) - (*dest & 0xFF);
            if (ins_rep) {
                thread.Exx[1]--;
            }
            thread.Exx[6] += 1;
            thread.Exx[7] += 1;
        } else if ((s->opcode->substr(0, s->opcode->size()).compare("cmpsd") == 0) && (s->hde.flags & F_PREFIX_66)) {
            result = (*src & 0xFFFF - *dest & 0xFFFF);
            if (ins_rep) {
                thread.Exx[1]--;
            }
            thread.Exx[6] += 2;
            thread.Exx[7] += 2;
        } else if ((s->opcode->substr(0, s->opcode->size()).compare("cmpsd") == 0) && !(s->hde.flags & F_PREFIX_66)) {
            result = *src - *dest;
            if (ins_rep) {
                thread.Exx[1]--;
            }
            thread.Exx[6] += 4;
            thread.Exx[7] += 4;
        }
        thread.updateflags(*dest, 0, result, UPDATEFLAGS_SUB, s->flags);
        // system("PAUSE");
        if (thread.EFlags & EFLG_ZF && (repe == false)) {
            break;
        }
        if (!(thread.EFlags & EFLG_ZF) && (repe == true)) {
            break;
        }
        // cout << "the result is = "<< (int*)thread.Exx[0]<< "\n";
        // system("PAUSE");
    }
    return 0;
}

// -----------------------------------------------------------------------------------------------------------------
// SCAS
int op_scas(Thread & thread, DISASM_INSTRUCTION * s) {
    int  rep     = 1; // for repeat instructions
    bool ins_rep = false;
    bool repe    = false;

    if (s->hde.flags & F_PREFIX_REP) {
        rep     = thread.Exx[1];     // ecx
        ins_rep = true;
        // thread.Exx[1]=0;              //set it to zero
    }
    if (s->hde.flags & F_PREFIX_REPX) {
        repe = true;
    }
    // now we will loop the read instruction
    for (int i = 0; i < rep; i++) {
        // one byte
        DWORD * dest;
        DWORD * src = (DWORD *) &thread.Exx[0]; // eax
        EMU_READ_MEM(dest, thread.Exx[7]); // edi
        // cout << (int*)*dest<<"\n";
        DWORD result;
        if (s->opcode->substr(0, s->opcode->size()).compare("scasb") == 0) {
            result = (*src & 0xFF) - (*dest & 0xFF);
            if (ins_rep) {
                thread.Exx[1]--;
            }
            thread.Exx[7] += 1;
        } else if ((s->opcode->substr(0, s->opcode->size()).compare("scasd") == 0) && (s->hde.flags & F_PREFIX_66)) {
            result = (*src & 0xFFFF - *dest & 0xFFFF);
            if (ins_rep) {
                thread.Exx[1]--;
            }
            thread.Exx[7] += 2;
        } else if ((s->opcode->substr(0, s->opcode->size()).compare("scasd") == 0) && !(s->hde.flags & F_PREFIX_66)) {
            result = *src - *dest;
            if (ins_rep) {
                thread.Exx[1]--;
            }
            thread.Exx[7] += 4;
        }
        thread.updateflags(*dest, 0, result, UPDATEFLAGS_SUB, s->flags);
        // cout<< (int*)result << "\n";
        // system("PAUSE");
        if (thread.EFlags & EFLG_ZF && (repe == false)) {
            break;
        }
        if (!(thread.EFlags & EFLG_ZF) && (repe == true)) {
            break;
        }
        // cout << "the result is = "<< (int*)thread.Exx[0]<< "\n";
        // system("PAUSE");
    }
    return 0;
}
