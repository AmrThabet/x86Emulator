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

// this file for emulating the jmps and push &pop to stack

// PUSH & POP
int op_push(Thread & thread, DISASM_INSTRUCTION * s) {
    int dest = 0;

    if (s->flags & DEST_IMM) {
        if (s->hde.flags & F_IMM8 && (s->ndest & 0x80)) {
            s->ndest += 0xFFFFFF00;
        }
        dest = s->ndest;
    } else if (s->flags & DEST_REG) {
        int dest2 = thread.Exx[s->ndest];
        if (s->flags & DEST_BITS32) {
            memcpy(&dest, &dest2, 4);
        }
        if (s->flags & DEST_BITS16) {
            memcpy(&dest, &dest2, 2);
        }
        if (s->flags & DEST_BITS8) {
            memcpy(&dest, &dest2, 1);
        }
    } else if (s->flags & DEST_RM) {
        DWORD * ptr;
        EMU_READ_MEM(ptr, (DWORD) modrm_calc(thread, s));
        if (s->flags & DEST_BITS32) {
            memcpy(&dest, ptr, 4);
        }
        if (s->flags & DEST_BITS16) {
            memcpy(&dest, ptr, 2);
        }
        if (s->flags & DEST_BITS8) {
            memcpy(&dest, ptr, 1);
        }
    }
    if (s->hde.flags & F_PREFIX_66) {
        EMU_WRITE_MEM(thread.Exx[4] - 2, (DWORD) 2, (unsigned char *) &dest);
        thread.Exx[4] -= 2;
    } else {
        thread.stack->push(dest);
    }
    // cout << "src = " << dest << "\n";
    return 0;
}

// ---------------
int op_pop(Thread & thread, DISASM_INSTRUCTION * s) {
    DWORD src;

    if (s->hde.flags & F_PREFIX_66) {
        DWORD * src2;
        EMU_READ_MEM(src2, thread.Exx[4]);
        src            = (DWORD) src2;
        thread.Exx[4] += 2;
    } else {
        src = (DWORD) thread.stack->pop();
    }
    DWORD dest, result;
    if (s->flags & DEST_REG) {
        dest = thread.Exx[s->ndest];
        // thread.Exx[s->ndest]=src;
        if (s->flags & DEST_BITS32) {
            memcpy(&thread.Exx[s->ndest], &src, 4);
        }
        if (s->flags & DEST_BITS16) {
            memcpy(&thread.Exx[s->ndest], &src, 2);
        }
        if (s->flags & DEST_BITS8) {
            memcpy(&thread.Exx[s->ndest], &src, 1);
        }
        result = thread.Exx[s->ndest];
    } else if (s->flags & DEST_RM) {
        DWORD * ptr;
        EMU_READ_MEM(ptr, (DWORD) modrm_calc(thread, s));
        dest = *ptr;
        if (s->flags & DEST_BITS32) {
            DWORD n = src;
            EMU_WRITE_MEM((DWORD) modrm_calc(thread, s), (DWORD) 4, (unsigned char *) &n);
        }
        if (s->flags & DEST_BITS16) {
            unsigned short n = src;
            EMU_WRITE_MEM((DWORD) modrm_calc(thread, s), (DWORD) 2, (unsigned char *) &n);
        }
        if (s->flags & DEST_BITS8) {
            unsigned char n = src;
            EMU_WRITE_MEM((DWORD) modrm_calc(thread, s), (DWORD) 1, (unsigned char *) &n);
        }
        result = *ptr;
    }
    return 0;
}

// ----------------------------------------------------------------------------------------
// PUSHAD & POPAD
int op_pushad(Thread & thread, DISASM_INSTRUCTION * s) {
    for (int i = 0; i < 8; i++) {
        thread.stack->push(thread.Exx[i]);
    }
    return 0;
}

int op_popad(Thread & thread, DISASM_INSTRUCTION * s) {
    for (int i = 7; i >= 0; i--) {
        if (i != 4) {
            thread.Exx[i] = thread.stack->pop();
        } else {
            thread.stack->pop();
        }
    }
    return 0;
}

// ----------------------------------------------------------------------------------------
// PUSHFD & POPFD
int op_pushfd(Thread & thread, DISASM_INSTRUCTION * s) {
    thread.stack->push(thread.EFlags);
    return 0;
}

int op_popfd(Thread & thread, DISASM_INSTRUCTION * s) {
    thread.EFlags = thread.stack->pop();
    return 0;
}

// ----------------------------------------------------------------------------------------
int op_enter(Thread & thread, DISASM_INSTRUCTION * s) {
    thread.stack->push(thread.Exx[5]);
    thread.Exx[5] = thread.Exx[4];
    for (int i = 0; i < s->nsrc; i++) {
        thread.stack->push(thread.Exx[5]);
        thread.Exx[5] = thread.Exx[4];
    }
    thread.Exx[4] -= s->ndest;
    // thread.Eip=thread.stack->pop();
    return 0;
}

// ----------------------------------------------------------------------------------------
int op_leave(Thread & thread, DISASM_INSTRUCTION * s) {
    thread.Exx[4] = thread.Exx[5];
    DWORD src = (DWORD) thread.stack->pop();
    memcpy(&thread.Exx[5], &src, 4);
    // thread.Eip=thread.stack->pop();
    return 0;
}

// =============================================================================================================
// JCC
int op_jcc(Thread & thread, DISASM_INSTRUCTION * s) {
    int  dest = 0;
    bool rel  = false;

    if (s->flags & DEST_IMM) {
        dest = s->ndest;
        // converting the negative imm8 to negative imm32
        if ((s->flags & DEST_BITS8) && ((dest >> 7) == 1)) {
            dest += 0xFFFFFF00;
        }
        rel = true; // that's mean that the dest will be added to or subtracted from the eip of the thread
    } else if (s->flags & DEST_REG) {
        dest = thread.Exx[s->ndest];
    } else if (s->flags & DEST_RM) {
        DWORD * ptr;
        EMU_READ_MEM(ptr, (DWORD) modrm_calc(thread, s));
        memcpy(&dest, ptr, 4);
    }
    // now we have the offset and now we need to chack if we will jump to it or not
    // cout <<s->opcode->substr(0,s->opcode->size()) <<"\n";
    if (s->opcode->substr(0, s->opcode->size()).compare("jmp") == 0) {
        goto Yes_JmptoIt;
    } else if ((s->opcode->substr(0, s->opcode->size()).compare("ja") == 0) || (s->opcode->substr(0, s->opcode->size()).compare("jnbe") == 0)) {
        if (!(thread.EFlags & EFLG_CF) && !(thread.EFlags & EFLG_ZF)) {
            goto Yes_JmptoIt;
        }
    } else if ((s->opcode->substr(0, s->opcode->size()).compare("jae") == 0) || (s->opcode->substr(0, s->opcode->size()).compare("jnb") == 0)) {
        if (!(thread.EFlags & EFLG_CF)) {
            goto Yes_JmptoIt;
        }
    } else if ((s->opcode->substr(0, s->opcode->size()).compare("jnae") == 0) || (s->opcode->substr(0, s->opcode->size()).compare("jb") == 0)) {
        if (thread.EFlags & EFLG_CF) {
            goto Yes_JmptoIt;
        }
    } else if ((s->opcode->substr(0, s->opcode->size()).compare("jbe") == 0) || (s->opcode->substr(0, s->opcode->size()).compare("jna") == 0)) {
        if ((thread.EFlags & EFLG_CF) || (thread.EFlags & EFLG_ZF)) {
            goto Yes_JmptoIt;
        }
    } else if ((s->opcode->substr(0, s->opcode->size()).compare("je") == 0) || (s->opcode->substr(0, s->opcode->size()).compare("jz") == 0)) {
        if (thread.EFlags & EFLG_ZF) {
            goto Yes_JmptoIt;
        }
    } else if ((s->opcode->substr(0, s->opcode->size()).compare("jne") == 0) || (s->opcode->substr(0, s->opcode->size()).compare("jnz") == 0)) {
        if (!(thread.EFlags & EFLG_ZF)) {
            goto Yes_JmptoIt;
        }
    } else if ((s->opcode->substr(0, s->opcode->size()).compare("jnp") == 0) || (s->opcode->substr(0, s->opcode->size()).compare("jpo") == 0)) {
        if (!(thread.EFlags & EFLG_PF)) {
            goto Yes_JmptoIt;
        }
    } else if ((s->opcode->substr(0, s->opcode->size()).compare("jp") == 0) || (s->opcode->substr(0, s->opcode->size()).compare("jpe") == 0)) {
        if (thread.EFlags & EFLG_PF) {
            goto Yes_JmptoIt;
        }
    } else if ((s->opcode->substr(0, s->opcode->size()).compare("jg") == 0) || (s->opcode->substr(0, s->opcode->size()).compare("jnle") == 0)) {
        if (!(thread.EFlags & EFLG_SF) ^ (thread.EFlags & EFLG_OF) && !(thread.EFlags & EFLG_ZF)) {
            goto Yes_JmptoIt;
        }
    } else if ((s->opcode->substr(0, s->opcode->size()).compare("jnge") == 0) || (s->opcode->substr(0, s->opcode->size()) .compare("jl") == 0)) {
        if ((thread.EFlags & EFLG_SF) ^ (thread.EFlags & EFLG_OF)) {
            goto Yes_JmptoIt;
        }
    } else if ((s->opcode->substr(0, s->opcode->size()).compare("jge") == 0) || (s->opcode->substr(0, s->opcode->size()) .compare("jnl") == 0)) {
        if (!((thread.EFlags & EFLG_SF) ^ (thread.EFlags & EFLG_OF))) {
            goto Yes_JmptoIt; // not SF xor OF as 1 xor 1 == jump
        }
    } else if ((s->opcode->substr(0, s->opcode->size()).compare("jle") == 0) || (s->opcode->substr(0, s->opcode->size()).compare("jng") == 0)) {
        if ((thread.EFlags & EFLG_SF) ^ (thread.EFlags & EFLG_OF) || (thread.EFlags & EFLG_ZF)) {
            goto Yes_JmptoIt;
        }
    } else if (s->opcode->substr(0, s->opcode->size()).compare("jns") == 0) {
        if (!(thread.EFlags & EFLG_SF)) {
            goto Yes_JmptoIt;
        }
    } else if (s->opcode->substr(0, s->opcode->size()).compare("js") == 0) {
        if (thread.EFlags & EFLG_SF) {
            goto Yes_JmptoIt;
        }
    } else if (s->opcode->substr(0, s->opcode->size()).compare("jno") == 0) {
        if (!(thread.EFlags & EFLG_OF)) {
            goto Yes_JmptoIt;
        }
    } else if (s->opcode->substr(0, s->opcode->size()).compare("jo") == 0) {
        if (thread.EFlags & EFLG_OF) {
            goto Yes_JmptoIt;
        }
    } else if ((s->hde.opcode == 0xE3) && !(s->hde.flags & F_PREFIX_67)) {
        if (thread.Exx[1] == 0) {
            goto Yes_JmptoIt;
        }
    } else if ((s->hde.opcode == 0xE3) && (s->hde.flags & F_PREFIX_67)) {
        if ((thread.Exx[1] & 0xffff) == 0) {
            goto Yes_JmptoIt;
        }
    } else if (s->opcode->substr(0, s->opcode->size()).compare("loop") == 0) {
        thread.Exx[1]--;
        if (thread.Exx[1] != 0) {
            goto Yes_JmptoIt;
        }
    } else if ((s->opcode->substr(0, s->opcode->size()).compare("loope") == 0) || (s->opcode->substr(0, s->opcode->size()).compare("loopz") == 0)) {
        thread.Exx[1]--;
        if (thread.EFlags & EFLG_ZF && (thread.Exx[1] != 0)) {
            goto Yes_JmptoIt;
        }
    } else if ((s->opcode->substr(0, s->opcode->size()).compare("loopne") == 0) || (s->opcode->substr(0, s->opcode->size()).compare("loopnz") == 0)) {
        thread.Exx[1]--;
        if (!(thread.EFlags & EFLG_ZF) && (thread.Exx[1] != 0)) {
            goto Yes_JmptoIt;
        }
    }

    return 0;
Yes_JmptoIt:
    if (rel) {
        thread.Eip = (DWORD) ((signed int) thread.Eip + (signed int) dest);
    } else {
        thread.Eip = dest; // - s->hde.len;
    }

    return 0;
}

// =============================================================================================================
// SETCC
int op_setcc(Thread & thread, DISASM_INSTRUCTION * s) {
    unsigned char * ptr = 0;
    bool   rel = false;

    if (s->flags & DEST_REG) {
        if (s->ndest < 3) {
            ptr = (unsigned char *) &thread.Exx[s->ndest];
        } else {
            ptr = (unsigned char *) &thread.Exx[s->ndest - 4];
            ptr++;
        }
    } else if (s->flags & DEST_RM) {
        DWORD * ptr2;
        EMU_READ_MEM(ptr2, (DWORD) modrm_calc(thread, s));
        ptr = (unsigned char *) ptr2;
    }
    // now we have the offset and now we need to chack if we will jump to it or not
    // cout <<s->opcode->substr(0,s->opcode->size()) <<"\n";
    if ((s->opcode->substr(0, s->opcode->size()).compare("seta") == 0) || (s->opcode->substr(0, s->opcode->size()).compare("setnbe") == 0)) {
        if (!(thread.EFlags & EFLG_CF) && !(thread.EFlags & EFLG_ZF)) {
            goto Yes_JmptoIt;
        }
    } else if ((s->opcode->substr(0, s->opcode->size()).compare("setae") == 0) || (s->opcode->substr(0, s->opcode->size()).compare("setnb") == 0)) {
        if (!(thread.EFlags & EFLG_CF)) {
            goto Yes_JmptoIt;
        }
    } else if ((s->opcode->substr(0, s->opcode->size()).compare("setnae") == 0) || (s->opcode->substr(0, s->opcode->size()).compare("setb") == 0)) {
        if (thread.EFlags & EFLG_CF) {
            goto Yes_JmptoIt;
        }
    } else if ((s->opcode->substr(0, s->opcode->size()).compare("setbe") == 0) || (s->opcode->substr(0, s->opcode->size()).compare("setna") == 0)) {
        if ((thread.EFlags & EFLG_CF) || (thread.EFlags & EFLG_ZF)) {
            goto Yes_JmptoIt;
        }
    } else if ((s->opcode->substr(0, s->opcode->size()).compare("sete") == 0) || (s->opcode->substr(0, s->opcode->size()).compare("setz") == 0)) {
        if (thread.EFlags & EFLG_ZF) {
            goto Yes_JmptoIt;
        }
    } else if ((s->opcode->substr(0, s->opcode->size()).compare("setne") == 0) || (s->opcode->substr(0, s->opcode->size()).compare("setnz") == 0)) {
        if (!(thread.EFlags & EFLG_ZF)) {
            goto Yes_JmptoIt;
        }
    } else if ((s->opcode->substr(0, s->opcode->size()).compare("setnp") == 0) || (s->opcode->substr(0, s->opcode->size()).compare("setpo") == 0)) {
        if (!(thread.EFlags & EFLG_PF)) {
            goto Yes_JmptoIt;
        }
    } else if ((s->opcode->substr(0, s->opcode->size()).compare("setp") == 0) || (s->opcode->substr(0, s->opcode->size()).compare("setpe") == 0)) {
        if (thread.EFlags & EFLG_PF) {
            goto Yes_JmptoIt;
        }
    } else if ((s->opcode->substr(0, s->opcode->size()).compare("setg") == 0) || (s->opcode->substr(0, s->opcode->size()).compare("setnle") == 0)) {
        if (!(thread.EFlags & EFLG_SF) ^ (thread.EFlags & EFLG_OF) && !(thread.EFlags & EFLG_ZF)) {
            goto Yes_JmptoIt;
        }
    } else if ((s->opcode->substr(0, s->opcode->size()).compare("setnge") == 0) || (s->opcode->substr(0, s->opcode->size()).compare("setl") == 0)) {
        if ((thread.EFlags & EFLG_SF) ^ (thread.EFlags & EFLG_OF)) {
            goto Yes_JmptoIt;
        }
    } else if ((s->opcode->substr(0, s->opcode->size()).compare("setge") == 0) || (s->opcode->substr(0, s->opcode->size()).compare("setnl") == 0)) {
        if (!((thread.EFlags & EFLG_SF) ^ (thread.EFlags & EFLG_OF))) {
            goto Yes_JmptoIt; // not SF xor OF as 1 xor 1 == set
        }
    } else if ((s->opcode->substr(0, s->opcode->size()).compare("setle") == 0) || (s->opcode->substr(0, s->opcode->size()).compare("setng") == 0)) {
        if ((thread.EFlags & EFLG_SF) ^ (thread.EFlags & EFLG_OF) || (thread.EFlags & EFLG_ZF)) {
            goto Yes_JmptoIt;
        }
    } else if (s->opcode->substr(0, s->opcode->size()).compare("setns") == 0) {
        if (!(thread.EFlags & EFLG_SF)) {
            goto Yes_JmptoIt;
        }
    } else if (s->opcode->substr(0, s->opcode->size()).compare("sets") == 0) {
        if (thread.EFlags & EFLG_SF) {
            goto Yes_JmptoIt;
        }
    } else if (s->opcode->substr(0, s->opcode->size()).compare("setno") == 0) {
        if (!(thread.EFlags & EFLG_OF)) {
            goto Yes_JmptoIt;
        }
    } else if (s->opcode->substr(0, s->opcode->size()).compare("seto") == 0) {
        if (thread.EFlags & EFLG_OF) {
            goto Yes_JmptoIt;
        }
    } else if ((s->hde.opcode == 0xE3) && !(s->hde.flags & F_PREFIX_67)) {
        if (thread.Exx[1] == 0) {
            goto Yes_JmptoIt;
        }
    } else if ((s->hde.opcode == 0xE3) && (s->hde.flags & F_PREFIX_67)) {
        if ((thread.Exx[1] & 0xffff) == 0) {
            goto Yes_JmptoIt;
        }
    }

    return 0;
Yes_JmptoIt:
    cout << (int *) ptr << "\n";
    cout << (int *) *ptr << "\n";
    *ptr = 1;

    return 0;
}

// =============================================================================================================
// CALL
int op_call(Thread & thread, DISASM_INSTRUCTION * s) {
    int  dest = 0;
    bool rel  = false;

    if (s->flags & DEST_IMM) {
        dest = s->ndest;
        rel  = true;   // that's mean that the dest will be added to or subtracted from the eip of the thread
    } else if (s->flags & DEST_REG) {
        dest = thread.Exx[s->ndest];
    } else if (s->flags & DEST_RM) {
        DWORD * ptr;
        EMU_READ_MEM(ptr, (DWORD) modrm_calc(thread, s));
        memcpy(&dest, ptr, 4);
    }
    // push the pointer to the next instruction
    // we work here as the process increase the eip before emulating the instruction so the eip now pointing to the next instruction
    thread.stack->push(thread.Eip);
    if (rel) {
        thread.Eip = (DWORD) ((signed int) thread.Eip + (signed int) dest); // - s->hde.len+1;
    } else {
        thread.Eip = dest; // we subtract it because the process::emulate  will add it again
    }
    return 0;
}

// =============================================================================================================
// RET
int op_ret(Thread & thread, DISASM_INSTRUCTION * s) {
    int bf   = 0;
    int dest = 0;

    // this for the parameters of the function
    thread.Eip = thread.stack->pop();
    if (s->flags & DEST_IMM) {
        thread.Exx[4] += s->ndest;
    }
    if ((thread.Eip == TLS_MAGIC) && thread.still_tls) {
        thread.TLSContinue();
    }
    if (thread.Eip == SEH_MAGIC) {
        thread.sehReturn();
    }
    return 0;
}
