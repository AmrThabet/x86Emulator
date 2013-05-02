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
// this file for emulating the general instructions
// here is the modrm calculator .it's small because the disassembler do nearby all the work

DWORD modrm_calc(Thread & thread, DISASM_INSTRUCTION * s) {
    DWORD result = 0;

    for (int i = 0; i < s->modrm.length; i++) {
        if (s->modrm.flags[i] & RM_REG) {
            int n = 0;
            if (s->modrm.flags[i] & RM_ADDR16) {
                n = (thread.Exx[s->modrm.items[i]] & 0xffff);
            } else {
                n = thread.Exx[s->modrm.items[i]];
            }
            if (s->modrm.flags[i] & RM_MUL2) {
                n *= 2;
            }
            if (s->modrm.flags[i] & RM_MUL4) {
                n *= 4;
            }
            if (s->modrm.flags[i] & RM_MUL8) {
                n *= 8;
            }
            result += n;
        } else if (s->modrm.flags[i] & RM_DISP) {
            if (s->modrm.flags[i] & RM_DISP8 && ((s->modrm.items[i] >> 7) == 1)) {
                s->modrm.items[i] += 0xFFFFFF00;

            }
            if (s->modrm.flags[i] & RM_DISP16 && ((s->modrm.items[i] >> 15) == 1)) {
                s->modrm.items[i] += 0xFFFF0000;

            }
            result = (DWORD) ((signed int) result + (signed int) s->modrm.items[i]);

        }
    }
    if (s->hde.flags & F_PREFIX_SEG && (s->hde.p_seg == PREFIX_SEGMENT_FS)) {
        result += thread.GetFS();
    }
    return result;
    return 0;
}

// ---------------------------------------------------------------------------------
// here we will state every opcode emulation function

// ADD
int op_add(Thread & thread, DISASM_INSTRUCTION * s) {
    // first we will test the source and get the value that we will put in the dest in src variable
    DWORD dest, result;
    int   src = 0;

    if (s->flags & SRC_IMM) {
        src = s->nsrc;
    } else if (s->flags & SRC_REG) {
        int src2 = thread.Exx[s->nsrc];
        if (s->flags & SRC_BITS32) {
            memcpy(&src, &src2, 4);
        }
        if (s->flags & SRC_BITS16) {
            memcpy(&src, &src2, 2);
        }
        if (s->flags & SRC_BITS8) {
            if (s->nsrc > 3) {
                src2 = thread.Exx[s->nsrc - 4] >> 8;
            }
            memcpy(&src, &src2, 1);
        }
    } else if (s->flags & SRC_RM) {
        DWORD * ptr;
        EMU_READ_MEM(ptr, (DWORD) modrm_calc(thread, s));
        if (s->flags & SRC_BITS32) {
            memcpy(&src, ptr, 4);
        }
        if (s->flags & SRC_BITS16) {
            memcpy(&src, ptr, 2);
        }
        if (s->flags & SRC_BITS8) {
            memcpy(&src, ptr, 1);
        }
    }
    // now we have the value of the src that we will put it in the dest now we will test the dest
    if (s->flags & DEST_REG) {
        dest = thread.Exx[s->ndest];
        if (s->flags & DEST_BITS32) {
            src += thread.Exx[s->ndest];
            memcpy(&thread.Exx[s->ndest], &src, 4);
            result = thread.Exx[s->ndest];
        }
        if (s->flags & DEST_BITS16) {
            dest &= 0xffff;
            src  += thread.Exx[s->ndest];
            memcpy(&thread.Exx[s->ndest], &src, 2);
            result = thread.Exx[s->ndest] & 0xffff;
        }
        if (s->flags & DEST_BITS8) {
            unsigned char * dest2 = (unsigned char *) &thread.Exx[s->ndest];
            if (s->ndest > 3) {
                dest2 = (unsigned char *) (&thread.Exx[s->ndest - 4]);
                dest2++;
            }
            dest    = *dest2 & 0xff;
            *dest2 += (unsigned char) src;
            result  = *dest2 & 0xff;
        }
        // result=thread.Exx[s->ndest];
    } else if (s->flags & DEST_RM) {
        DWORD * ptr;
        // EMU_READ_MEM(ptr,(DWORD)modrm_calc(thread,s));
        EMU_READ_MEM(ptr, modrm_calc(thread, s)); //

        DWORD n = *ptr;
        dest = n;
        if (s->flags & DEST_BITS32) {
            DWORD n = *ptr;
            n += src;
            EMU_WRITE_MEM((DWORD) modrm_calc(thread, s), (DWORD) 4, (unsigned char *) &n);
        }
        if (s->flags & DEST_BITS16) {
            unsigned short n = *ptr;
            n += src;
            EMU_WRITE_MEM((DWORD) modrm_calc(thread, s), (DWORD) 2, (unsigned char *) &n);
        }
        if (s->flags & DEST_BITS8) {
            unsigned char n = *ptr;
            n += src;
            EMU_WRITE_MEM((DWORD) modrm_calc(thread, s), (DWORD) 1, (unsigned char *) &n);
        }
        result = *ptr;
    }
    thread.updateflags(dest, 0, result, UPDATEFLAGS_ADD, s->flags);
    // cout << "dest= "<<(int*)dest << "\nresult= " << (int*)result<<"\nflags= "<< (int*)thread.EFlags << "\n";
    return 0;
}

// ---------------------------------------------------------------------------------
// OR
int op_or(Thread & thread, DISASM_INSTRUCTION * s) {
    // first we will test the source and get the value that we will put in the dest in src variable
    DWORD dest, result;
    int   src = 0;

    if (s->flags & SRC_IMM) {
        src = s->nsrc;
    } else if (s->flags & SRC_REG) {
        int src2 = thread.Exx[s->nsrc];
        if (s->flags & SRC_BITS32) {
            memcpy(&src, &src2, 4);
        }
        if (s->flags & SRC_BITS16) {
            memcpy(&src, &src2, 2);
        }
        if (s->flags & SRC_BITS8) {
            if (s->nsrc > 3) {
                src2 = thread.Exx[s->nsrc - 4] >> 8;
            }
            memcpy(&src, &src2, 1);
        }
    } else if (s->flags & SRC_RM) {
        DWORD * ptr;
        EMU_READ_MEM(ptr, (DWORD) modrm_calc(thread, s));
        if (s->flags & SRC_BITS32) {
            memcpy(&src, ptr, 4);
        }
        if (s->flags & SRC_BITS16) {
            memcpy(&src, ptr, 2);
        }
        if (s->flags & SRC_BITS8) {
            memcpy(&src, ptr, 1);
        }
    }
    // now we have the value of the src that we will put it in the dest now we will test the dest
    if (s->flags & DEST_REG) {
        dest = thread.Exx[s->ndest];
        if (s->flags & DEST_BITS32) {
            src |= thread.Exx[s->ndest];
            memcpy(&thread.Exx[s->ndest], &src, 4);
            result = thread.Exx[s->ndest];
        }
        if (s->flags & DEST_BITS16) {
            dest &= 0xffff;
            src  |= thread.Exx[s->ndest];
            memcpy(&thread.Exx[s->ndest], &src, 2);
            result = thread.Exx[s->ndest] & 0xffff;
        }
        if (s->flags & DEST_BITS8) {
            unsigned char * dest2 = (unsigned char *) &thread.Exx[s->ndest];
            if (s->ndest > 3) {
                dest2 = (unsigned char *) (&thread.Exx[s->ndest - 4]);
                dest2++;
            }
            dest    = *dest2 & 0xff;
            *dest2 |= (unsigned char) src;
            result  = *dest2 & 0xff;
        }
    } else if (s->flags & DEST_RM) {
        DWORD * ptr;
        EMU_READ_MEM(ptr, (DWORD) modrm_calc(thread, s));
        DWORD n = *ptr;
        dest = n;
        if (s->flags & DEST_BITS32) {
            DWORD n = *ptr;
            n |= src;
            EMU_WRITE_MEM((DWORD) modrm_calc(thread, s), (DWORD) 4, (unsigned char *) &n);
        }
        if (s->flags & DEST_BITS16) {
            unsigned short n = *ptr;
            n |= src;
            EMU_WRITE_MEM((DWORD) modrm_calc(thread, s), (DWORD) 2, (unsigned char *) &n);
        }
        if (s->flags & DEST_BITS8) {
            unsigned char n = *ptr;
            n |= src;
            EMU_WRITE_MEM((DWORD) modrm_calc(thread, s), (DWORD) 1, (unsigned char *) &n);
        }
        result = *ptr;
    }
    thread.updateflags(dest, 0, result, 0, s->flags);
    thread.EFlags &= (EFLG_PF | EFLG_ZF | EFLG_SF | EFLG_SYS);
    // cout << "dest= "<<(int*)dest << "\nresult= " << (int*)result<<"\nflags= "<< (int*)thread.EFlags << "\n";
    return 0;
}

// -------------------------------------------------------------------------------------------------
// ADC
int op_adc(Thread & thread, DISASM_INSTRUCTION * s) {
    // first we will test the source and get the value that we will put in the dest in src variable
    DWORD dest, result;
    int   src = 0;

    if (s->flags & SRC_IMM) {
        src = s->nsrc;
    } else if (s->flags & SRC_REG) {
        int src2 = thread.Exx[s->nsrc];
        if (s->flags & SRC_BITS32) {
            memcpy(&src, &src2, 4);
        }
        if (s->flags & SRC_BITS16) {
            memcpy(&src, &src2, 2);
        }
        if (s->flags & SRC_BITS8) {
            if (s->nsrc > 3) {
                src2 = thread.Exx[s->nsrc - 4] >> 8;
            }
            memcpy(&src, &src2, 1);
        }
    } else if (s->flags & SRC_RM) {
        DWORD * ptr;
        EMU_READ_MEM(ptr, (DWORD) modrm_calc(thread, s));
        if (s->flags & SRC_BITS32) {
            memcpy(&src, ptr, 4);
        }
        if (s->flags & SRC_BITS16) {
            memcpy(&src, ptr, 2);
        }
        if (s->flags & SRC_BITS8) {
            memcpy(&src, ptr, 1);
        }
    }
    // now we have the value of the src that we will put it in the dest now we will test the dest
    if (s->flags & DEST_REG) {
        dest = thread.Exx[s->ndest];

        if (s->flags & DEST_BITS32) {
            src += thread.Exx[s->ndest] + (thread.EFlags & 1);
            memcpy(&thread.Exx[s->ndest], &src, 4);
        }
        if (s->flags & DEST_BITS16) {
            src += thread.Exx[s->ndest] + (thread.EFlags & 1);
            memcpy(&thread.Exx[s->ndest], &src, 2);
        }
        if (s->flags & DEST_BITS8) {
            unsigned char * dest2 = (unsigned char *) &thread.Exx[s->ndest];
            if (s->ndest > 3) {
                dest2 = (unsigned char *) (&thread.Exx[s->ndest - 4]);
                dest2++;
            }
            *dest2 += (unsigned char) src + (thread.EFlags & 1);
        }
        result = thread.Exx[s->ndest];
    } else if (s->flags & DEST_RM) {
        DWORD * ptr;
        EMU_READ_MEM(ptr, (DWORD) modrm_calc(thread, s));
        // cout << *ptr << "\n\n";
        DWORD n = *ptr;
        dest = n;
        if (s->flags & DEST_BITS32) {
            DWORD n = *ptr;
            n += src + (thread.EFlags & 1);
            EMU_WRITE_MEM((DWORD) modrm_calc(thread, s), (DWORD) 4, (unsigned char *) &n);
        }
        if (s->flags & DEST_BITS16) {
            unsigned short n = *ptr;
            n += src + (thread.EFlags & 1);
            EMU_WRITE_MEM((DWORD) modrm_calc(thread, s), (DWORD) 2, (unsigned char *) &n);
        }
        if (s->flags & DEST_BITS8) {
            unsigned char n = *ptr;
            n += src + (thread.EFlags & 1);
            EMU_WRITE_MEM((DWORD) modrm_calc(thread, s), (DWORD) 1, (unsigned char *) &n);
        }
        result = *ptr;
    }

    int result2 = result - (thread.EFlags & 1);
    thread.updateflags(dest, 0, result, UPDATEFLAGS_ADD, s->flags);
    thread.EFlags &= ~EFLG_CF;
    if ((unsigned int) dest > (unsigned int) result2) {
        thread.EFlags |= EFLG_CF;
    }
    return 0;
}

// ---------------------------------------------------------------------------------
// SBB
int op_sbb(Thread & thread, DISASM_INSTRUCTION * s) {
    // first we will test the source and get the value that we will put in the dest in src variable
    DWORD dest, result;
    int   src = 0;
    bool  neg = false;

    if (s->flags & SRC_IMM) {
        src = s->nsrc;
        if (s->flags & SRC_BITS8 && (src >> 7 == 1)) {
            neg = true;
            src = (-(unsigned char) src);
        }
        if (s->flags & SRC_BITS16 && (src >> 15 == 1)) {
            neg = true;
            src = (-(short) src);
        }
    } else if (s->flags & SRC_REG) {
        int src2 = thread.Exx[s->nsrc];
        if (s->flags & SRC_BITS32) {
            memcpy(&src, &src2, 4);
        }
        if (s->flags & SRC_BITS16) {
            memcpy(&src, &src2, 2);
        }
        if (s->flags & SRC_BITS8) {
            if (s->nsrc > 3) {
                src2 = thread.Exx[s->nsrc - 4] >> 8;
            }
            memcpy(&src, &src2, 1);
        }
    } else if (s->flags & SRC_RM) {
        DWORD * ptr;
        EMU_READ_MEM(ptr, (DWORD) modrm_calc(thread, s));
        if (s->flags & SRC_BITS32) {
                memcpy(&src, ptr, 4);
        }
        if (s->flags & SRC_BITS16) {
                memcpy(&src, ptr, 2);
        }
        if (s->flags & SRC_BITS8) {
                memcpy(&src, ptr, 1);
        }
    }
    // now we have the value of the src that we will put it in the dest now we will test the dest
    if (s->flags & DEST_REG) {
        if (s->flags & DEST_BITS8 && (s->ndest > 3)) {
            unsigned char * dest2 = (unsigned char *) (&thread.Exx[s->ndest - 4]);
            dest2++;
            if (neg) {
                src += *dest2 - (thread.EFlags & 1);
            } else {
                src -= *dest2 - (thread.EFlags & 1);
                src  = -src;
            }
            dest = *dest2 & 0xff;
                memcpy(dest2, &src, 1);
            result = *dest2 & 0xff;
        } else {
            dest = thread.Exx[s->ndest];
            if (neg) {
                src += (thread.Exx[s->ndest] - (thread.EFlags & 1));
            } else {
                src -= (thread.Exx[s->ndest] - (thread.EFlags & 1));
                src  = -src;
            }
            if (s->flags & DEST_BITS32) {
                memcpy(&thread.Exx[s->ndest], &src, 4);
                result = thread.Exx[s->ndest];
            }
            if (s->flags & DEST_BITS16) {
                dest &= 0xffff;
                memcpy(&thread.Exx[s->ndest], &src, 2);
                result = thread.Exx[s->ndest] & 0xffff;
            }
            if (s->flags & DEST_BITS8) {
                dest &= 0xff;
                memcpy(&thread.Exx[s->ndest], &src, 1);
                result = thread.Exx[s->ndest] & 0xff;
            }
        }
    } else if (s->flags & DEST_RM) {
        DWORD * ptr;
        EMU_READ_MEM(ptr, (DWORD) modrm_calc(thread, s));
        // cout << *ptr << "\n\n";
        DWORD n = *ptr;
        dest = n;
        if (s->flags & DEST_BITS32) {
            DWORD n = *ptr;
            if (neg) {
                n += (src - (thread.EFlags & 1));
            } else {
                n -= (src + (thread.EFlags & 1));
            }
            EMU_WRITE_MEM((DWORD) modrm_calc(thread, s), (DWORD) 4, (unsigned char *) &n);
        }
        if (s->flags & DEST_BITS16) {
            unsigned short n = *ptr;
            if (neg) {
                n += (src - (thread.EFlags & 1));
            } else {
                n -= (src + (thread.EFlags & 1));
            }
            EMU_WRITE_MEM((DWORD) modrm_calc(thread, s), (DWORD) 2, (unsigned char *) &n);
        }
        if (s->flags & DEST_BITS8) {
            unsigned char n = *ptr;
            if (neg) {
                n += (src - (thread.EFlags & 1));
            } else {
                n -= (src + (thread.EFlags & 1));
            }
            EMU_WRITE_MEM((DWORD) modrm_calc(thread, s), (DWORD) 1, (unsigned char *) &n);
        }
        result = *ptr;
    }
    thread.updateflags(dest, 0, result, UPDATEFLAGS_SUB, s->flags);
    // cout << "dest= "<<(int*)dest << "\nresult= " << (int*)result<<"\nflags= "<< (int*)thread.EFlags << "\n";
    return 0;
}

// ---------------------------------------------------------------------------------
// AND
int op_and(Thread & thread, DISASM_INSTRUCTION * s) {
    // first we will test the source and get the value that we will put in the dest in src variable
    DWORD dest, result;
    int   src = 0;

    if (s->flags & SRC_IMM) {
        src = s->nsrc;
    } else if (s->flags & SRC_REG) {
        int src2 = thread.Exx[s->nsrc];
        if (s->flags & SRC_BITS32) {
            memcpy(&src, &src2, 4);
        }
        if (s->flags & SRC_BITS16) {
            memcpy(&src, &src2, 2);
        }
        if (s->flags & SRC_BITS8) {
            if (s->nsrc > 3) {
                src2 = thread.Exx[s->nsrc - 4] >> 8;
            }
            memcpy(&src, &src2, 1);
        }
    } else if (s->flags & SRC_RM) {
        DWORD * ptr;
        EMU_READ_MEM(ptr, (DWORD) modrm_calc(thread, s));
        if (s->flags & SRC_BITS32) {
            memcpy(&src, ptr, 4);
        }
        if (s->flags & SRC_BITS16) {
            memcpy(&src, ptr, 2);
        }
        if (s->flags & SRC_BITS8) {
            memcpy(&src, ptr, 1);
        }
    }
    // now we have the value of the src that we will put it in the dest now we will test the dest
    if (s->flags & DEST_REG) {
        dest = thread.Exx[s->ndest];

        if (s->flags & DEST_BITS32) {
            src &= thread.Exx[s->ndest];
            memcpy(&thread.Exx[s->ndest], &src, 4);
            result = thread.Exx[s->ndest];
        }
        if (s->flags & DEST_BITS16) {
            dest &= 0xffff;
            src  &= thread.Exx[s->ndest];
            memcpy(&thread.Exx[s->ndest], &src, 2);
            result = thread.Exx[s->ndest] & 0xffff;
        }
        if (s->flags & DEST_BITS8) {
            unsigned char * dest2 = (unsigned char *) &thread.Exx[s->ndest];
            if (s->ndest > 3) {
                dest2 = (unsigned char *) (&thread.Exx[s->ndest - 4]);
                dest2++;
            }
            dest    = *dest2 & 0xff;
            *dest2 &= (unsigned char) src;
            result  = *dest2 & 0xff;
        }
    } else if (s->flags & DEST_RM) {
        DWORD * ptr;
        EMU_READ_MEM(ptr, (DWORD) modrm_calc(thread, s));
        DWORD n = *ptr;
        dest = n;
        if (s->flags & DEST_BITS32) {
            DWORD n = *ptr;
            n &= src;
            EMU_WRITE_MEM((DWORD) modrm_calc(thread, s), (DWORD) 4, (unsigned char *) &n);
        }
        if (s->flags & DEST_BITS16) {
            unsigned short n = *ptr;
            n &= src;
            EMU_WRITE_MEM((DWORD) modrm_calc(thread, s), (DWORD) 2, (unsigned char *) &n);
        }
        if (s->flags & DEST_BITS8) {
            unsigned char n = *ptr;
            n &= src;
            EMU_WRITE_MEM((DWORD) modrm_calc(thread, s), (DWORD) 1, (unsigned char *) &n);
        }
        result = *ptr;
    }
    thread.updateflags(dest, 0, result, 0, s->flags);
    thread.EFlags &= (EFLG_PF | EFLG_ZF | EFLG_SF | EFLG_SYS);
    // cout << "dest= "<<(int*)dest << "\nresult= " << (int*)result<<"\nflags= "<< (int*)thread.EFlags << "\n";
    return 0;
}

// ---------------------------------------------------------------------------------
// SUB
int op_sub(Thread & thread, DISASM_INSTRUCTION * s) {
    // first we will test the source and get the value that we will put in the dest in src variable
    DWORD dest, result;
    int   src = 0;
    bool  neg = false;

    if (s->flags & SRC_IMM) {
        src = s->nsrc;
        if (s->flags & SRC_BITS8 && (src >> 7 == 1)) {
            neg = true;
            src = (-(unsigned char) src);
        }
        if (s->flags & SRC_BITS16 && (src >> 15 == 1)) {
            neg = true;
            src = (-(short) src);
        }
    } else if (s->flags & SRC_REG) {
        int src2 = thread.Exx[s->nsrc];
        if (s->flags & SRC_BITS32) {
            memcpy(&src, &src2, 4);
        }
        if (s->flags & SRC_BITS16) {
            memcpy(&src, &src2, 2);
        }
        if (s->flags & SRC_BITS8) {
            if (s->nsrc > 3) {
                src2 = thread.Exx[s->nsrc - 4] >> 8;
            }
            memcpy(&src, &src2, 1);
        }
    } else if (s->flags & SRC_RM) {
        DWORD * ptr;
        EMU_READ_MEM(ptr, (DWORD) modrm_calc(thread, s));
        if (s->flags & SRC_BITS32) {
                memcpy(&src, ptr, 4);
        }
        if (s->flags & SRC_BITS16) {
                memcpy(&src, ptr, 2);
        }
        if (s->flags & SRC_BITS8) {
                memcpy(&src, ptr, 1);
        }
    }
    // now we have the value of the src that we will put it in the dest now we will test the dest
    if (s->flags & DEST_REG) {
        if (s->flags & DEST_BITS8 && (s->ndest > 3)) {
            unsigned char * dest2 = (unsigned char *) (&thread.Exx[s->ndest - 4]);
            dest2++;
            if (neg) {
                src += *dest2;
            } else {
                src -= *dest2;
                src  = -src;
            }
            dest = *dest2 & 0xff;
                memcpy(dest2, &src, 1);
            result = *dest2 & 0xff;
        } else {
            dest = thread.Exx[s->ndest];
            if (neg) {
                src += thread.Exx[s->ndest];
            } else {
                src -= thread.Exx[s->ndest];
                src  = -src;
            }
            if (s->flags & DEST_BITS32) {
                memcpy(&thread.Exx[s->ndest], &src, 4);
                result = thread.Exx[s->ndest];
            }
            if (s->flags & DEST_BITS16) {
                dest &= 0xffff;
                memcpy(&thread.Exx[s->ndest], &src, 2);
                result = thread.Exx[s->ndest] & 0xffff;
            }
            if (s->flags & DEST_BITS8) {
                dest &= 0xff;
                memcpy(&thread.Exx[s->ndest], &src, 1);
                result = thread.Exx[s->ndest] & 0xff;
            }
        }
    } else if (s->flags & DEST_RM) {
        DWORD * ptr;
        EMU_READ_MEM(ptr, (DWORD) modrm_calc(thread, s));
        // cout << *ptr << "\n\n";
        DWORD n = *ptr;
        dest = n;
        if (s->flags & DEST_BITS32) {
            DWORD n = *ptr;
            if (neg) {
                n += src;
            } else {
                n -= src;
            }
            EMU_WRITE_MEM((DWORD) modrm_calc(thread, s), (DWORD) 4, (unsigned char *) &n);
        }
        if (s->flags & DEST_BITS16) {
            unsigned short n = *ptr;
            if (neg) {
                n += src;
            } else {
                n -= src;
            }
            EMU_WRITE_MEM((DWORD) modrm_calc(thread, s), (DWORD) 2, (unsigned char *) &n);
        }
        if (s->flags & DEST_BITS8) {
            unsigned char n = *ptr;
            if (neg) {
                n += src;
            } else {
                n -= src;
            }
            EMU_WRITE_MEM((DWORD) modrm_calc(thread, s), (DWORD) 1, (unsigned char *) &n);
        }
        result = *ptr;
    }
    thread.updateflags(dest, 0, result, UPDATEFLAGS_SUB, s->flags);
    // cout << "dest= "<<(int*)dest << "\nresult= " << (int*)result<<"\nflags= "<< (int*)thread.EFlags << "\n";
    return 0;
}

// ---------------------------------------------------------------------------------
// XOR
int op_xor(Thread & thread, DISASM_INSTRUCTION * s) {
    // first we will test the source and get the value that we will put in the dest in src variable
    DWORD dest, result;
    int   src = 0;

    if (s->flags & SRC_IMM) {
        src = s->nsrc;
    } else if (s->flags & SRC_REG) {
        int src2 = thread.Exx[s->nsrc];
        if (s->flags & SRC_BITS32) {
            memcpy(&src, &src2, 4);
        }
        if (s->flags & SRC_BITS16) {
            memcpy(&src, &src2, 2);
        }
        if (s->flags & SRC_BITS8) {
            if (s->nsrc > 3) {
                src2 = thread.Exx[s->nsrc - 4] >> 8;
            }
            memcpy(&src, &src2, 1);
        }
    } else if (s->flags & SRC_RM) {
        DWORD * ptr;
        EMU_READ_MEM(ptr, (DWORD) modrm_calc(thread, s));
        if (s->flags & SRC_BITS32) {
            memcpy(&src, ptr, 4);
        }
        if (s->flags & SRC_BITS16) {
            memcpy(&src, ptr, 2);
        }
        if (s->flags & SRC_BITS8) {
            memcpy(&src, ptr, 1);
        }
    }
    // now we have the value of the src that we will put it in the dest now we will test the dest
    if (s->flags & DEST_REG) {
        dest = thread.Exx[s->ndest];
        if (s->flags & DEST_BITS32) {
            src ^= thread.Exx[s->ndest];
            memcpy(&thread.Exx[s->ndest], &src, 4);
            result = thread.Exx[s->ndest];
        }
        if (s->flags & DEST_BITS16) {
            dest &= 0xffff;
            src  ^= thread.Exx[s->ndest];
            memcpy(&thread.Exx[s->ndest], &src, 2);
            result = thread.Exx[s->ndest] & 0xffff;
        }
        if (s->flags & DEST_BITS8) {
            unsigned char * dest2 = (unsigned char *) &thread.Exx[s->ndest];
            if (s->ndest > 3) {
                dest2 = (unsigned char *) (&thread.Exx[s->ndest - 4]);
                dest2++;
            }
            dest    = *dest2 & 0xff;
            *dest2 ^= src;
            result  = *dest2 & 0xff;
        }
        result = thread.Exx[s->ndest];
    } else if (s->flags & DEST_RM) {
        DWORD * ptr;
        EMU_READ_MEM(ptr, (DWORD) modrm_calc(thread, s));
        // cout << *ptr << "\n\n";
        DWORD n = *ptr;
        dest = n;
        if (s->flags & DEST_BITS32) {
            DWORD n = *ptr;
            n ^= src;
            EMU_WRITE_MEM((DWORD) modrm_calc(thread, s), (DWORD) 4, (unsigned char *) &n);
        }
        if (s->flags & DEST_BITS16) {
            unsigned short n = *ptr;
            n ^= src;
            EMU_WRITE_MEM((DWORD) modrm_calc(thread, s), (DWORD) 2, (unsigned char *) &n);
        }
        if (s->flags & DEST_BITS8) {
            unsigned char n = *ptr;
            n ^= src;
            EMU_WRITE_MEM((DWORD) modrm_calc(thread, s), (DWORD) 1, (unsigned char *) &n);
        }
        result = *ptr;
    }
    thread.updateflags(dest, 0, result, 0, s->flags);
    thread.EFlags &= (EFLG_PF | EFLG_ZF | EFLG_SF | EFLG_SYS);
    // cout << "dest= "<<(int*)dest << "\nresult= " << (int*)result<<"\nflags= "<< (int*)thread.EFlags << "\n";
    return 0;
}

// ---------------------------------------------------------------------------------
// CMP
int op_cmp(Thread & thread, DISASM_INSTRUCTION * s) {
    // first we will test the source and get the value that we will put in the dest in src variable
    DWORD dest, result;
    int   src = 0;
    bool  neg = false;

    if (s->flags & SRC_IMM) {
        src = s->nsrc;
        if (s->flags & SRC_BITS8 && (src >> 7 == 1)) {
            neg = true;
            src = (-(unsigned char) src);
        }
        if (s->flags & SRC_BITS16 && (src >> 15 == 1)) {
            neg = true;
            src = (-(short) src);
        }
    } else if (s->flags & SRC_REG) {
        int src2 = thread.Exx[s->nsrc];
        if (s->flags & SRC_BITS32) {
            memcpy(&src, &src2, 4);
        }
        if (s->flags & SRC_BITS16) {
            memcpy(&src, &src2, 2);
        }
        if (s->flags & SRC_BITS8) {
            if (s->nsrc > 3) {
                src2 = thread.Exx[s->nsrc - 4] >> 8;
            }
            memcpy(&src, &src2, 1);
        }
    } else if (s->flags & SRC_RM) {
        DWORD * ptr;
        EMU_READ_MEM(ptr, (DWORD) modrm_calc(thread, s));
        if (s->flags & SRC_BITS32) {
            memcpy(&src, ptr, 4);
        }
        if (s->flags & SRC_BITS16) {
            memcpy(&src, ptr, 2);
        }
        if (s->flags & SRC_BITS8) {
            memcpy(&src, ptr, 1);
        }
    }
    // now we have the value of the src that we will put it in the dest now we will test the dest
    if (s->flags & DEST_REG) {
        if (s->flags & DEST_BITS8 && (s->ndest > 3)) {
            unsigned char * dest2 = (unsigned char *) (&thread.Exx[s->ndest - 4]);
            dest2++;
            if (neg) {
                src += *dest2;
            } else {
                src -= *dest2;
                src  = -src;
            }
            dest   = *dest2 & 0xff;
            result = src & 0xff;
        } else {
            dest = thread.Exx[s->ndest];
            if (neg) {
                src += thread.Exx[s->ndest];
            } else {
                src -= thread.Exx[s->ndest];
                src  = -src;
            }
            if (s->flags & DEST_BITS32) {
                result = src;
            }
            if (s->flags & DEST_BITS16) {
                dest  &= 0xffff;
                result = src & 0xffff;
            }
            if (s->flags & DEST_BITS8) {
                dest  &= 0xff;
                result = src & 0xff;
            }
        }
    } else if (s->flags & DEST_RM) {
        DWORD * ptr;
        EMU_READ_MEM(ptr, (DWORD) modrm_calc(thread, s));
        // cout << *ptr << "\n\n";
        DWORD n = *ptr;
        dest = n;
        if (s->flags & DEST_BITS32) {
            DWORD n = *ptr;
            if (neg) {
                n += src;
            } else {
                n -= src;
            }
            result = (DWORD) n;
        }
        if (s->flags & DEST_BITS16) {
            unsigned short n = *ptr;
            if (neg) {
                n += src;
            } else {
                n -= src;
            }
            result = (DWORD) n;
        }
        if (s->flags & DEST_BITS8) {
            unsigned char n = *ptr;
            if (neg) {
                n += src;
            } else {
                n -= src;
            }
            result = (DWORD) n;
        }
    }
    thread.updateflags(dest, 0, result, UPDATEFLAGS_SUB, s->flags);
    // cout << "dest= "<<(int*)dest << "\nresult= " << (int*)result<<"\nflags= "<< (int*)thread.EFlags << "\n";
    return 0;
}

// ---------------------------------------------------------------------------------
// TEST
int op_test(Thread & thread, DISASM_INSTRUCTION * s) {
    // first we will test the source and get the value that we will put in the dest in src variable
    DWORD dest, result;
    int   src = 0;

    if (s->flags & SRC_IMM) {
        src = s->nsrc;
    } else if (s->flags & SRC_REG) {
        int src2 = thread.Exx[s->nsrc];
        if (s->flags & SRC_BITS32) {
            memcpy(&src, &src2, 4);
        }
        if (s->flags & SRC_BITS16) {
            memcpy(&src, &src2, 2);
        }
        if (s->flags & SRC_BITS8) {
            if (s->nsrc > 3) {
                src2 = thread.Exx[s->nsrc - 4] >> 8;
            }
            memcpy(&src, &src2, 1);
        }
    } else if (s->flags & SRC_RM) {
        DWORD * ptr;
        EMU_READ_MEM(ptr, (DWORD) modrm_calc(thread, s));
        if (s->flags & SRC_BITS32) {
            memcpy(&src, ptr, 4);
        }
        if (s->flags & SRC_BITS16) {
            memcpy(&src, ptr, 2);
        }
        if (s->flags & SRC_BITS8) {
            memcpy(&src, ptr, 1);
        }
    }
    // now we have the value of the src that we will put it in the dest now we will test the dest
    if (s->flags & DEST_REG) {
        dest = thread.Exx[s->ndest];
        if (s->flags & DEST_BITS8 && (s->ndest > 3)) {
            dest = thread.Exx[s->ndest - 4];
            src  = src << 8;
        }
        result = dest & src;
    } else if (s->flags & DEST_RM) {
        DWORD * ptr;
        EMU_READ_MEM(ptr, (DWORD) modrm_calc(thread, s));
        // cout << *ptr << "\n\n";
        DWORD n = *ptr;
        dest = n;
        if (s->flags & DEST_BITS32) {
            DWORD n = *ptr;
            result = n & src;
        }
        if (s->flags & DEST_BITS16) {
            unsigned short n = *ptr;
            result = (n & 0xffff) & src;
        }
        if (s->flags & DEST_BITS8) {
            unsigned char n = *ptr;
            result = (n & 0xff) & src;
        }
    }
    thread.updateflags(dest, 0, result, 0, s->flags);
    thread.EFlags &= (EFLG_PF | EFLG_ZF | EFLG_SF | EFLG_SYS);
    // cout << "dest= "<<(int*)dest << "\nresult= " << (int*)result<<"\nflags= "<< (int*)thread.EFlags << "\n";
    return 0;
}

// ==============================================================================================================================

// ---------------------------------------------------
// INC
int op_inc(Thread & thread, DISASM_INSTRUCTION * s) {
    // first we will test the source and get the value that we will put in the dest in src variable
    DWORD dest, result;
    int   src = 1;

    if (s->flags & DEST_REG) {
        dest = thread.Exx[s->ndest];
        if (s->flags & DEST_BITS8) {
            unsigned char * dest2 = (unsigned char *) &thread.Exx[s->ndest];
            if (s->ndest > 3) {
                dest2 = (unsigned char *) (&thread.Exx[s->ndest - 4]);
                dest2++;
				
            }
            *dest2 += (unsigned char) 1;
			result  = *dest2 & 0xff;
        } else {
            thread.Exx[s->ndest]++;
            result = thread.Exx[s->ndest];
        }
    } else if (s->flags & DEST_RM) {
        DWORD * ptr;
        EMU_READ_MEM(ptr, (DWORD) modrm_calc(thread, s));
        DWORD n = *ptr;
        dest = n;
        if (s->flags & DEST_BITS32) {
            DWORD n = *ptr;
            n += src;
            EMU_WRITE_MEM((DWORD) modrm_calc(thread, s), (DWORD) 4, (unsigned char *) &n);
        }
        if (s->flags & DEST_BITS16) {
            unsigned short n = *ptr;
            n += src;
            EMU_WRITE_MEM((DWORD) modrm_calc(thread, s), (DWORD) 2, (unsigned char *) &n);
        }
        if (s->flags & DEST_BITS8) {
            unsigned char n = *ptr;
            n += src;
            EMU_WRITE_MEM((DWORD) modrm_calc(thread, s), (DWORD) 1, (unsigned char *) &n);
        }
        result = *ptr;
    }
    DWORD OldEFlags = thread.EFlags;
    thread.updateflags(dest, 0, result, 0, s->flags);
    thread.EFlags &= (EFLG_PF | EFLG_ZF | EFLG_SF | EFLG_SYS);
    thread.EFlags += (OldEFlags & EFLG_CF);
    // cout << "dest= "<<(int*)dest << "\nresult= " << (int*)result<<"\nflags= "<< (int*)thread.EFlags << "\n";
    return 0;
}

// ---------------------------------------------------------------------------------
// DEC
int op_dec(Thread & thread, DISASM_INSTRUCTION * s) {
    // first we will test the source and get the value that we will put in the dest in src variable
    DWORD dest, result;
    int   src = 1;

    if (s->flags & DEST_REG) {
        dest = thread.Exx[s->ndest];
        src -= thread.Exx[s->ndest];
        src  = -src;
        if (s->flags & DEST_BITS32) {
            memcpy(&thread.Exx[s->ndest], &src, 4);
            result = thread.Exx[s->ndest];
        }
        if (s->flags & DEST_BITS16) {
            dest &= 0xffff;
            memcpy(&thread.Exx[s->ndest], &src, 2);
            result = thread.Exx[s->ndest] & 0xffff;
        }
        if (s->flags & DEST_BITS8) {
            unsigned char * dest2 = (unsigned char *) &thread.Exx[s->ndest];
            if (s->ndest > 3) {
                dest2 = (unsigned char *) (&thread.Exx[s->ndest - 4]);
                dest2++;
            }
            dest    = *dest2 & 0xff;
            *dest2 -= (unsigned char) 1;
            result  = *dest2 & 0xff;
        }
    } else if (s->flags & DEST_RM) {
        DWORD * ptr;
        EMU_READ_MEM(ptr, (DWORD) modrm_calc(thread, s));
        // cout << *ptr << "\n\n";
        DWORD n = *ptr;
        dest = n;
        if (s->flags & DEST_BITS32) {
            DWORD n = *ptr;
            n -= src;
            EMU_WRITE_MEM((DWORD) modrm_calc(thread, s), (DWORD) 4, (unsigned char *) &n);
        }
        if (s->flags & DEST_BITS16) {
            unsigned short n = *ptr;
            n -= src;
            EMU_WRITE_MEM((DWORD) modrm_calc(thread, s), (DWORD) 2, (unsigned char *) &n);
        }
        if (s->flags & DEST_BITS8) {
            unsigned char n = *ptr;
            n -= src;
            EMU_WRITE_MEM((DWORD) modrm_calc(thread, s), (DWORD) 1, (unsigned char *) &n);
        }
        result = *ptr;
    }
    DWORD OldEFlags = thread.EFlags;
    thread.updateflags(dest, 0, result, 0, s->flags);
    thread.EFlags &= (EFLG_PF | EFLG_ZF | EFLG_SF | EFLG_SYS);
    thread.EFlags += (OldEFlags & EFLG_CF);
    // cout << "dest= "<<(int*)dest << "\nresult= " << (int*)result<<"\nflags= "<< (int*)thread.EFlags << "\n";
    return 0;
}

// -------------------------------------------------------------------------------------------------------
// NOT
int op_not(Thread & thread, DISASM_INSTRUCTION * s) {
    // first we will test the source and get the value that we will put in the dest in src variable
    DWORD dest, result;
    int   src = 1;

    if (s->flags & DEST_REG) {
        dest = ~(thread.Exx[s->ndest]);
        if (s->flags & DEST_BITS32) {
            memcpy(&thread.Exx[s->ndest], &dest, 4);
        }
        if (s->flags & DEST_BITS16) {
            memcpy(&thread.Exx[s->ndest], &dest, 2);
        }
        if (s->flags & DEST_BITS8) {
            unsigned char * dest2 = (unsigned char *) &thread.Exx[s->ndest];
            if (s->ndest > 3) {
                dest2 = (unsigned char *) (&thread.Exx[s->ndest - 4]);
                dest2++;
            }
            *dest2 = (unsigned char) (~*dest2);
        }
    } else if (s->flags & DEST_RM) {
        DWORD * ptr;
        EMU_READ_MEM(ptr, (DWORD) modrm_calc(thread, s));
        DWORD n = *ptr;
        if (s->flags & DEST_BITS32) {
            DWORD n = *ptr;
            n = ~n;
            EMU_WRITE_MEM((DWORD) modrm_calc(thread, s), (DWORD) 4, (unsigned char *) &n);
        }
        if (s->flags & DEST_BITS16) {
            unsigned short n = *ptr;
            n = ~n;
            EMU_WRITE_MEM((DWORD) modrm_calc(thread, s), (DWORD) 2, (unsigned char *) &n);
        }
        if (s->flags & DEST_BITS8) {
            unsigned char n = *ptr;
            n = ~n;
            EMU_WRITE_MEM((DWORD) modrm_calc(thread, s), (DWORD) 1, (unsigned char *) &n);
        }
    }
    // cout << "dest= "<<(int*)dest << "\nresult= " << (int*)result<<"\nflags= "<< (int*)thread.EFlags << "\n";
    return 0;
}

// ---------------------------------------------------------------------------------
// NEG
int op_neg(Thread & thread, DISASM_INSTRUCTION * s) {
    // first we will test the source and get the value that we will put in the dest in src variable
    DWORD dest, result;

    if (s->flags & DEST_REG) {
        dest = (-thread.Exx[s->ndest]);
        if (s->flags & DEST_BITS32) {
            memcpy(&thread.Exx[s->ndest], &dest, 4);
        }
        if (s->flags & DEST_BITS16) {
            memcpy(&thread.Exx[s->ndest], &dest, 2);
        }
        if (s->flags & DEST_BITS8) {
            unsigned char * dest2 = (unsigned char *) &thread.Exx[s->ndest];
            if (s->ndest > 3) {
                dest2 = (unsigned char *) (&thread.Exx[s->ndest - 4]);
                dest2++;
            }
            *dest2 = (unsigned char) (-*dest2);
        }
        result = thread.Exx[s->ndest];
    } else if (s->flags & DEST_RM) {
        DWORD * ptr;
        EMU_READ_MEM(ptr, (DWORD) modrm_calc(thread, s));
        // cout << *ptr << "\n\n";
        DWORD n = *ptr;
        dest = n;
        if (s->flags & DEST_BITS32) {
            DWORD n = *ptr;
            n = -n;
            EMU_WRITE_MEM((DWORD) modrm_calc(thread, s), (DWORD) 4, (unsigned char *) &n);
        }
        if (s->flags & DEST_BITS16) {
            unsigned short n = *ptr;
            n = -n;
            EMU_WRITE_MEM((DWORD) modrm_calc(thread, s), (DWORD) 2, (unsigned char *) &n);
        }
        if (s->flags & DEST_BITS8) {
            unsigned char n = *ptr;
            n = -n;
            EMU_WRITE_MEM((DWORD) modrm_calc(thread, s), (DWORD) 1, (unsigned char *) &n);
        }
        result = *ptr;
    }
    thread.updateflags(dest, 0, result, 0, s->flags);
    thread.EFlags &= (EFLG_PF | EFLG_ZF | EFLG_SF | EFLG_SYS);
    if (result != 0) {
        thread.EFlags |= EFLG_CF;
    } else {
        thread.EFlags &= (~EFLG_CF);
    }
    // cout << "dest= "<<(int*)dest << "\nresult= " << (int*)result<<"\nflags= "<< (int*)thread.EFlags << "\n";
    return 0;
}

// ==============================================================================================================================
// MOV

int op_mov(Thread & thread, DISASM_INSTRUCTION * s) {
    // first we will test the source and get the value that we will put in the dest in src variable
    DWORD dest, result;
    int   src = 0;

    if (s->flags & SRC_IMM) {
        src = s->nsrc;
    } else if (s->flags & SRC_REG) {
        int src2 = thread.Exx[s->nsrc];
        if (s->flags & SRC_BITS32) {
            memcpy(&src, &src2, 4);
        }
        if (s->flags & SRC_BITS16) {
            memcpy(&src, &src2, 2);
        }
        if (s->flags & SRC_BITS8) {
            if (s->nsrc > 3) {
                src2 = thread.Exx[s->nsrc - 4] >> 8;
            }
            memcpy(&src, &src2, 1);
        }
    } else if (s->flags & SRC_RM) {
        DWORD * ptr;
        EMU_READ_MEM(ptr, (DWORD) modrm_calc(thread, s));
        if (s->flags & SRC_BITS32) {
            memcpy(&src, ptr, 4);
        }
        if (s->flags & SRC_BITS16) {
            memcpy(&src, ptr, 2);
        }
        if (s->flags & SRC_BITS8) {
            memcpy(&src, ptr, 1);
        }
    }
    // now we have the value of the src that we will put it in the dest now we will test the dest
    if (s->flags & DEST_REG) {
        dest = thread.Exx[s->ndest];
        if (s->flags & DEST_BITS32) {
            memcpy(&thread.Exx[s->ndest], &src, 4);
        }
        if (s->flags & DEST_BITS16) {
            dest &= 0xffff;
            memcpy(&thread.Exx[s->ndest], &src, 2);
        }
        if (s->flags & DEST_BITS8) {
            unsigned char * dest2 = (unsigned char *) &thread.Exx[s->ndest];
            if (s->ndest > 3) {
                dest2 = (unsigned char *) (&thread.Exx[s->ndest - 4]);
                dest2++;
            }
            dest = *dest2 & 0xff;
            unsigned char s = (unsigned char) src;
            *dest2 = s;
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
            unsigned char n = src & 0xFF;
            EMU_WRITE_MEM((DWORD) modrm_calc(thread, s), (DWORD) 1, (unsigned char *) &n);
            result = *ptr;
        }
    }
    return 0;
}

// ----------------------------------------------------------------------------------------------------------------
// LEA
int op_lea(Thread & thread, DISASM_INSTRUCTION * s) {
    DWORD dest, result;
    int   src = 0;

    if (s->flags & SRC_RM) {
        DWORD ptr;
        src = (DWORD) modrm_calc(thread, s);
    }
    if (s->flags & DEST_REG) {
        dest                 = thread.Exx[s->ndest];
        thread.Exx[s->ndest] = src;
        result               = thread.Exx[s->ndest];
    }
    return 0;
}

// ==============================================================================================================================
// MOVZX

int op_movzx(Thread & thread, DISASM_INSTRUCTION * s) {
    // first we will test the source and get the value that we will put in the dest in src variable
    DWORD dest, result;
    int   src = 0;

    if (s->flags & SRC_IMM) {
        src = s->nsrc;
    } else if (s->flags & SRC_REG) {
        int src2 = thread.Exx[s->nsrc];
        if (s->hde.opcode2 == 0xB7) {
            memcpy(&src, &src2, 2);
        }
        if (s->hde.opcode2 == 0xB6) {
            unsigned char * src3 = (unsigned char *) &thread.Exx[s->nsrc];
            if (s->nsrc > 3) {
                src3 = (unsigned char *) &thread.Exx[s->nsrc - 4];
                src3++;
            }
            memcpy(&src, src3, 1);
        }
    } else if (s->flags & SRC_RM) {
        DWORD * ptr;
        EMU_READ_MEM(ptr, (DWORD) modrm_calc(thread, s));
        if (s->hde.opcode2 == 0xB7) {
            memcpy(&src, ptr, 2);
        }
        if (s->hde.opcode2 == 0xB6) {
            memcpy(&src, ptr, 1);
        }
    }
    // now we have the value of the src that we will put it in the dest now we will test the dest
    if (s->flags & DEST_REG) {
        dest = thread.Exx[s->ndest];
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

// ==============================================================================================================================
// MOVSX

int op_movsx(Thread & thread, DISASM_INSTRUCTION * s) {
    // first we will test the source and get the value that we will put in the dest in src variable
    DWORD dest, result;
    int   src = 0;

    if (s->flags & SRC_IMM) {
        src = s->nsrc;
    } else if (s->flags & SRC_REG) {
        int src2 = thread.Exx[s->nsrc];
        if (s->hde.opcode2 == 0xBF) {
            memcpy(&src, &src2, 2);
            if ((src >> 15) == 1) {
                src = src + 0xFFFF0000;
            }
        }
        if (s->hde.opcode2 == 0xBE) {
            unsigned char * src3 = (unsigned char *) &thread.Exx[s->nsrc];
            if (s->nsrc > 3) {
                src3 = (unsigned char *) &thread.Exx[s->nsrc - 4];
                src3++;
            }
            memcpy(&src, src3, 1);
            if ((src >> 7) == 1) {
                src = src + 0xFFFFFF00;
            }
        }
    } else if (s->flags & SRC_RM) {
        DWORD * ptr;
        EMU_READ_MEM(ptr, (DWORD) modrm_calc(thread, s));
        if (s->hde.opcode2 == 0xBE) {
            memcpy(&src, ptr, 2);
        }
        if (s->hde.opcode2 == 0xBF) {
            memcpy(&src, ptr, 1);
        }
    }
    // now we have the value of the src that we will put it in the dest now we will test the dest
    if (s->flags & DEST_REG) {
        dest = thread.Exx[s->ndest];
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
