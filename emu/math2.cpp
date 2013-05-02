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

// XCHG
int op_xchg(Thread & thread, DISASM_INSTRUCTION * s) {
    // first we will test the source and get the value that we will put in the dest in src variable
    DWORD   dest, result;
    DWORD * src = 0;
	bool x = false;
    if (s->flags & SRC_REG) {
		if (s->flags & SRC_BITS8)
		{
			x = true;
		}
        src = &thread.Exx[s->nsrc];
        if (s->flags & SRC_BITS8 && (s->nsrc > 3)) {
            unsigned char * src2 = (unsigned char *) &thread.Exx[s->nsrc - 4];
            src2++;
            src = (DWORD *) src2;
        }
    } else if (s->flags & SRC_RM) {
        EMU_READ_MEM(src, modrm_calc(thread, s));
        // this place for checking for write access
        EMU_WRITE_MEM((DWORD) modrm_calc(thread, s), (DWORD) 4, (unsigned char *) src);
    }
    // now we have the value of the src that we will put it in the dest now we will test the dest
    if (s->flags & DEST_REG) {
        dest = thread.Exx[s->ndest];
        if (s->flags & DEST_BITS32) {
	
                memcpy(&thread.Exx[s->ndest], src,   4);
                memcpy(src,                   &dest, 4);
        }
        if (s->flags & DEST_BITS16) {
                memcpy(&thread.Exx[s->ndest], src,   2);
                memcpy(src,                   &dest, 2);
        }
        if (s->flags & DEST_BITS8) {
            if (s->ndest > 3) {
                unsigned char * dest2 = (unsigned char *) &thread.Exx[s->ndest - 4];
                dest2++;
                unsigned char dest3 = *dest2;
                memcpy(dest2,                 src,    1);
                memcpy(src,                   &dest3, 1);
            } else {
                memcpy(&thread.Exx[s->ndest], src,    1);
                memcpy(src,                   &dest,  1);
            }
        }
        result = thread.Exx[s->ndest];
    } else if (s->flags & DEST_RM) {
        DWORD * ptr;
        EMU_READ_MEM(ptr, (DWORD) modrm_calc(thread, s));
        dest = *ptr;
        if (s->flags & DEST_BITS32) {
            DWORD n = *src;
            EMU_WRITE_MEM((DWORD) modrm_calc(thread, s), (DWORD) 4, (unsigned char *) &n);
            memcpy(src, &dest, 4);
        }
        if (s->flags & DEST_BITS16) {
            unsigned short n = *src;
            EMU_WRITE_MEM((DWORD) modrm_calc(thread, s), (DWORD) 2, (unsigned char *) &n);
            memcpy(src, &dest, 2);
        }
        if (s->flags & DEST_BITS8) {
            unsigned char n = *src;
            EMU_WRITE_MEM((DWORD) modrm_calc(thread, s), (DWORD) 1, (unsigned char *) &n);
            memcpy(src, &dest, 1);
        }
        result = *ptr;
    }
    // cout << "dest= "<<(int*)dest << "\nresult= " << (int*)result<<"\nflags= "<< (int*)thread.EFlags << "\n";
    return 0;
}

// ==============================================================================================================================
// BSWAP
int op_bswap(Thread & thread, DISASM_INSTRUCTION * s) {
    DWORD dest, result;

    if (s->flags & DEST_BITS32) {
        int src = thread.Exx[s->ndest];
        __asm
		{
			mov eax,src
			bswap eax
			mov src,eax
		}
        memcpy(&thread.Exx[s->ndest], &src, 4);
    }
    if (s->flags & DEST_BITS16) {
        unsigned short src = thread.Exx[s->ndest];
        unsigned short s2  = (src & 0xff) << 16;
        unsigned short s3  = (src & 0xff00) >> 16;
        src = s2 + s2;
        memcpy(&thread.Exx[s->ndest], &src, 2);
    }
    return 0;
}

// ==============================================================================================================================
// XADD
int op_xadd(Thread & thread, DISASM_INSTRUCTION * s) {
    // first we will test the source and get the value that we will put in the dest in src variable
    DWORD   dest, result;
    DWORD * src = 0;

    if (s->flags & SRC_REG) {
        src = &thread.Exx[s->nsrc];
        if (s->flags & SRC_BITS8 && (s->nsrc > 3)) {
            unsigned char * src2 = (unsigned char *) &thread.Exx[s->nsrc - 4];
            src2++; // this instruction for DWORD* will add 4 to the pointer rather than 1
            src = (DWORD *) src2;
        }
    } else if (s->flags & SRC_RM) {
        EMU_READ_MEM(src, (DWORD) modrm_calc(thread, s));
        // this place for checking for write access
        EMU_WRITE_MEM((DWORD) modrm_calc(thread, s), (DWORD) 4, (unsigned char *) src);
    }
    // now we have the value of the src that we will put it in the dest now we will test the dest
    if (s->flags & DEST_REG) {
        dest = thread.Exx[s->ndest];
        if (s->flags & DEST_BITS32) {
            thread.Exx[s->ndest] += *src;
            memcpy(src, &dest, 4);
        }
        if (s->flags & DEST_BITS16) {
            unsigned short addition = *src & 0xffff + thread.Exx[s->ndest] & 0xffff;
            memcpy(&thread.Exx[s->ndest], &addition, 2);
            memcpy(src,                   &dest,     2);
        }
        if (s->flags & DEST_BITS8) {
            unsigned char * dest2 = (unsigned char *) &thread.Exx[s->ndest];
            if (s->ndest > 3) {
                dest2 = (unsigned char *) (&thread.Exx[s->ndest - 4]);
                dest2++;
                dest = *dest2;
            }
            unsigned char addition = (*src & 0xff) + (dest & 0xff);
            memcpy(dest2, &addition, 1);
            memcpy(src,   &dest,     1);
        }
        result = thread.Exx[s->ndest];
    } else if (s->flags & DEST_RM) {
        DWORD * ptr;
            EMU_READ_MEM(ptr, (DWORD) modrm_calc(thread, s));
        dest = *ptr + *src;
        if (s->flags & DEST_BITS32) {
            DWORD * readmem;
            EMU_READ_MEM(readmem, (DWORD) modrm_calc(thread, s));
            DWORD n = *src + *readmem;
            EMU_WRITE_MEM((DWORD) modrm_calc(thread, s), (DWORD) 4, (unsigned char *) &n);
            memcpy(src, &dest, 4);
        }
        if (s->flags & DEST_BITS16) {
            DWORD * readmem;
            EMU_READ_MEM(readmem, (DWORD) modrm_calc(thread, s));
            unsigned short n = (*src & 0xffff) + (*readmem & 0xffff);
            EMU_WRITE_MEM((DWORD) modrm_calc(thread, s), (DWORD) 2, (unsigned char *) &n);
            memcpy(src, &dest, 2);
        }
        if (s->flags & DEST_BITS8) {
            DWORD * readmem;
            EMU_READ_MEM(readmem, (DWORD) modrm_calc(thread, s));
            unsigned char n = (*src & 0xff) + (*readmem & 0xff);
            EMU_WRITE_MEM((DWORD) modrm_calc(thread, s), (DWORD) 1, (unsigned char *) &n);
            memcpy(src, &dest, 1);
        }
        result = *ptr;
    }
    thread.updateflags(dest, 0, result, UPDATEFLAGS_ADD, s->flags);
    return 0;
}

// ==============================================================================================================================
// MUL

int op_mul(Thread & thread, DISASM_INSTRUCTION * s) {
    // first we will test the source and get the value that we will put in the dest in src variable
    DWORD dest, result;
    int   src = 0;
    int   a, b;

    if (s->flags & DEST_REG) {
        int src2 = thread.Exx[s->ndest];
        if (s->flags & DEST_BITS32) {
            memcpy(&src, &src2, 4);
        }
        if (s->flags & DEST_BITS16) {
            memcpy(&src, &src2, 2);
        }
        if (s->flags & DEST_BITS8) {
            if (s->ndest > 3) {
                src2 = thread.Exx[s->ndest - 4] >> 8;
            }
            memcpy(&src, &src2, 1);
        }
    } else if (s->flags & DEST_RM) {
        DWORD * ptr;
        EMU_READ_MEM(ptr, (DWORD) modrm_calc(thread, s));
        if (s->flags & DEST_BITS32) {
            memcpy(&src, ptr, 4);
        }
        if (s->flags & DEST_BITS16) {
            memcpy(&src, ptr, 2);
        }
        if (s->flags & DEST_BITS8) {
            memcpy(&src, ptr, 1);
        }
    }
    if (s->flags & DEST_BITS32) {
        __asm
		{
			mov eax,thread.Exx[0]
			mov edx, src
			mul edx
			mov thread.Exx[0],eax
			mov thread.Exx[2],edx
		}
    } else if (s->flags & DEST_BITS16) {
		__asm
		{
			mov eax, thread.Exx[0]
			mov ecx, src
			mov edx,thread.Exx[2]
			mul cx
			mov thread.Exx[0],eax
			mov thread.Exx[2],edx
		}
    } else if (s->flags & DEST_BITS8) {
        __asm
		{
			mov eax,thread.Exx[0]
			mov edx,thread.Exx[2]
			mul cl
			mov thread.Exx[0],eax
			mov thread.Exx[2],edx
		}
    }
    return 0;
}

// ==============================================================================================================================
// IMUL
int op_imul1(Thread & thread, DISASM_INSTRUCTION * s) {
    // first we will test the source and get the value that we will put in the dest in src variable
    DWORD dest, result;
    int   src = 0;
    int   a, b;

    if (s->flags & DEST_REG) {
        int src2 = thread.Exx[s->ndest];
        if (s->flags & DEST_BITS32) {
            memcpy(&src, &src2, 4);
        }
        if (s->flags & DEST_BITS16) {
            memcpy(&src, &src2, 2);
        }
        if (s->flags & DEST_BITS8) {
            if (s->ndest > 3) {
                src2 = thread.Exx[s->ndest - 4] >> 8;
            }
            memcpy(&src, &src2, 1);
        }
    } else if (s->flags & DEST_RM) {
        DWORD * ptr;
        EMU_READ_MEM(ptr, (DWORD) modrm_calc(thread, s));
        if (s->flags & DEST_BITS32) {
            memcpy(&src, ptr, 4);
        }
        if (s->flags & DEST_BITS16) {
            memcpy(&src, ptr, 2);
        }
        if (s->flags & DEST_BITS8) {
            memcpy(&src, ptr, 1);
        }
    }
    if (s->flags & DEST_BITS32) {
        __asm
		{
			mov eax,thread.Exx[0]
			mov edx,src
			imul edx
			mov thread.Exx[0],eax
			mov thread.Exx[2],edx
		}
    } else if (s->flags & DEST_BITS16) {
		__asm
		{
			mov eax,thread.Exx[0]
			mov ecx,src
			mov edx,thread.Exx[2]
			imul cx
			mov thread.Exx[0],eax
			mov thread.Exx[2],edx
		}
    } else if (s->flags & DEST_BITS8) {
		__asm
		{
			mov eax,thread.Exx[0]
			mov ecx,src
			mov edx,thread.Exx[2]
			imul cl
			mov thread.Exx[0],eax
			mov thread.Exx[2],edx
		}
    }
    return 0;
}

// -------------------------------------------------------------------------------------------------------------------------
// IMUL 2
int op_imul2(Thread & thread, DISASM_INSTRUCTION * s) {
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
        src *= thread.Exx[s->ndest];
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
        DWORD n = *ptr;
        dest = n;
        if (s->flags & DEST_BITS32) {
            DWORD n = *ptr;
            n *= src;
            EMU_WRITE_MEM((DWORD) modrm_calc(thread, s), (DWORD) 4, (unsigned char *) &n);
        }
        if (s->flags & DEST_BITS16) {
            unsigned short n = *ptr;
            n *= src;
            EMU_WRITE_MEM((DWORD) modrm_calc(thread, s), (DWORD) 2, (unsigned char *) &n);
        }
        if (s->flags & DEST_BITS8) {
            unsigned char n = *ptr;
            n *= src;
            EMU_WRITE_MEM((DWORD) modrm_calc(thread, s), (DWORD) 1, (unsigned char *) &n);
        }
        result = *ptr;
    }
    // thread.updateflags(dest,0,result,UPDATEFLAGS_ADD);
    return 0;
}

// -------------------------------------------------------------------------------------------------------------------------
// IMUL 3
int op_imul3(Thread & thread, DISASM_INSTRUCTION * s) {
    DWORD dest, result;
    int   src = 0, imm = s->other;

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
        src *= imm;
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
        DWORD n = *ptr;
        dest = n;
        if (s->flags & DEST_BITS32) {
            DWORD n = *ptr;
            n = imm * src;
            EMU_WRITE_MEM((DWORD) modrm_calc(thread, s), (DWORD) 4, (unsigned char *) &n);
        }
        if (s->flags & DEST_BITS16) {
            unsigned short n = *ptr;
            n = imm * src;
            EMU_WRITE_MEM((DWORD) modrm_calc(thread, s), (DWORD) 2, (unsigned char *) &n);
        }
        if (s->flags & DEST_BITS8) {
            unsigned char n = *ptr;
            n = imm * src;
            EMU_WRITE_MEM((DWORD) modrm_calc(thread, s), (DWORD) 1, (unsigned char *) &n);
        }
        result = *ptr;
    }
    // thread.updateflags(dest,0,result,UPDATEFLAGS_ADD);
    return 0;
}

// ==============================================================================================================================
// DIV

int op_div(Thread & thread, DISASM_INSTRUCTION * s) {
    // first we will test the source and get the value that we will put in the dest in src variable
    DWORD dest, result;
    int   src = 0;
    int   a, b;

    if (s->flags & DEST_REG) {
        int src2 = thread.Exx[s->ndest];
        if (s->flags & DEST_BITS32) {
            memcpy(&src, &src2, 4);
        }
        if (s->flags & DEST_BITS16) {
            memcpy(&src, &src2, 2);
        }
        if (s->flags & DEST_BITS8) {
            if (s->ndest > 3) {
                src2 = thread.Exx[s->ndest - 4] >> 8;
            }
            memcpy(&src, &src2, 1);
        }
    } else if (s->flags & DEST_RM) {
        DWORD * ptr;
        EMU_READ_MEM(ptr, (DWORD) modrm_calc(thread, s));
        if (s->flags & DEST_BITS32) {
            memcpy(&src, ptr, 4);
        }
        if (s->flags & DEST_BITS16) {
            memcpy(&src, ptr, 2);
        }
        if (s->flags & DEST_BITS8) {
            memcpy(&src, ptr, 1);
        }
    }
    if (src == 0) {
        return EXP_DIVID_BY_ZERO;
    }
    if (s->flags & DEST_BITS32) {
        if (thread.Exx[2] >= src) {
            return EXP_DIV_OVERFLOW;
        }
		__asm
		{
			mov eax,thread.Exx[0]
			mov ecx,src
			mov edx,thread.Exx[2]
			div ecx
			mov thread.Exx[0],eax
			mov thread.Exx[2],edx
		}
    } else if (s->flags & DEST_BITS16) {
        if ((thread.Exx[2] & 0xFFFF) >= (src & 0xFFFF)) {
            return EXP_DIV_OVERFLOW;
        }
		__asm
		{
			mov eax,thread.Exx[0]
			mov ecx,src
			mov edx,thread.Exx[2]
			div cx
			mov thread.Exx[0],eax
			mov thread.Exx[2],edx
		}
    } else if (s->flags & DEST_BITS8) {
        if ((thread.Exx[2] & 0xFF) >= (src & 0xFF)) {
            return EXP_DIV_OVERFLOW;
        }
		__asm
		{
			mov eax,thread.Exx[0]
			mov ecx,src
			mov edx,thread.Exx[2]
			div cl
			mov thread.Exx[0],eax
			mov thread.Exx[2],edx
		}
    }
    return 0;
}

// ==============================================================================================================================
// IDIV

int op_idiv(Thread & thread, DISASM_INSTRUCTION * s) {
    // first we will test the source and get the value that we will put in the dest in src variable
    DWORD dest, result;
    int   src = 0;
    int   a, b;

    if (s->flags & DEST_REG) {
        int src2 = thread.Exx[s->ndest];
        if (s->flags & DEST_BITS32) {
            memcpy(&src, &src2, 4);
        }
        if (s->flags & DEST_BITS16) {
            memcpy(&src, &src2, 2);
        }
        if (s->flags & DEST_BITS8) {
            if (s->ndest > 3) {
                src2 = thread.Exx[s->ndest - 4] >> 8;
            }
            memcpy(&src, &src2, 1);
        }
    } else if (s->flags & DEST_RM) {
        DWORD * ptr;
        EMU_READ_MEM(ptr, (DWORD) modrm_calc(thread, s));
        if (s->flags & DEST_BITS32) {
            memcpy(&src, ptr, 4);
        }
        if (s->flags & DEST_BITS16) {
            memcpy(&src, ptr, 2);
        }
        if (s->flags & DEST_BITS8) {
            memcpy(&src, ptr, 1);
        }
    }
    if (src == 0) {
        return EXP_DIVID_BY_ZERO;
    }
    if (s->flags & DEST_BITS32) {
        if ((thread.Exx[2] << 1) >= src) {
            return EXP_DIV_OVERFLOW;
        }
		__asm
		{
			mov eax,thread.Exx[0]
			mov ecx,src
			mov edx,thread.Exx[2]
			idiv ecx
			mov thread.Exx[0],eax
			mov thread.Exx[2],edx
		}
    } else if (s->flags & DEST_BITS16) {
        if (((thread.Exx[2] & 0xFFFF) << 1) >= (src & 0xFFFF)) {
            return EXP_DIV_OVERFLOW;
        }
		__asm
		{
			mov eax,thread.Exx[0]
			mov ecx,src
			mov edx,thread.Exx[2]
			idiv cx
			mov thread.Exx[0],eax
			mov thread.Exx[2],edx
		}
    } else if (s->flags & DEST_BITS8) {
        if (((thread.Exx[2] & 0xFF) << 1) >= (src & 0xFF)) {
            return EXP_DIV_OVERFLOW;
        }
		__asm
		{
			mov eax,thread.Exx[0]
			mov ecx,src
			mov edx,thread.Exx[2]
			idiv cl
			mov thread.Exx[0],eax
			mov thread.Exx[2],edx
		}
    }
    return 0;
}

// ==============================================================================================================================
// CDQ

int op_cdq(Thread & thread, DISASM_INSTRUCTION * s) {
    thread.Exx[2] = 0;
    return 0;
}
