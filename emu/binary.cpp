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

// ---------------------------------------------------------------------------------
// SHL
int op_shl(Thread & thread, DISASM_INSTRUCTION * s) {
    // first we will test the source and get the value that we will put in the dest in src variable
    DWORD dest, result;
    int   src = 0;

    if (s->flags & SRC_IMM) {
        src = s->nsrc & 0x1F; // not bigger than 31
    } else if (s->flags & SRC_REG) {
        DWORD src2 = thread.Exx[1];
            memcpy(&src, &src2, 1);
    }
    // now we have the value of the src that we will put it in the dest now we will test the dest
    if (s->flags & DEST_REG) {
        dest = thread.Exx[s->ndest];
        DWORD n = thread.Exx[s->ndest];
        n = n << src;
        if (s->flags & DEST_BITS32) {
            memcpy(&thread.Exx[s->ndest], &n, 4);
            result = thread.Exx[s->ndest];
        }
        if (s->flags & DEST_BITS16) {
            memcpy(&thread.Exx[s->ndest], &n, 2);
            result = thread.Exx[s->ndest] & 0xffff;
        }
        if (s->flags & DEST_BITS8) {
            unsigned char * dest2 = (unsigned char *) &thread.Exx[s->ndest];
            if (s->ndest > 3) {
                dest2 = (unsigned char *) (&thread.Exx[s->ndest - 4]);
                dest2++;
            }
            *dest2 = *dest2 << src;
            result = *dest2 & 0xff;
        }
    } else if (s->flags & DEST_RM) {
        DWORD * ptr;
        EMU_READ_MEM(ptr, (DWORD) modrm_calc(thread, s));
        DWORD n = *ptr;
        dest = n;
        if (s->flags & DEST_BITS32) {
            DWORD n = *ptr;
            n = (n & 0xffffffff) << src;
            EMU_WRITE_MEM((DWORD) modrm_calc(thread, s), (DWORD) 4, (unsigned char *) &n);
        }
        if (s->flags & DEST_BITS16) {
            unsigned short n = *ptr;
            n = (n & 0xffff) << src;
            EMU_WRITE_MEM((DWORD) modrm_calc(thread, s), (DWORD) 2, (unsigned char *) &n);
        }
        if (s->flags & DEST_BITS8) {
            unsigned char n = *ptr;
            n = (n & 0xff) << src;
            EMU_WRITE_MEM((DWORD) modrm_calc(thread, s), (DWORD) 1, (unsigned char *) &n);
        }
        result = *ptr;
    }
    thread.updateflags(dest, 0, result, 0, s->flags);
    thread.EFlags &= (EFLG_PF | EFLG_ZF | EFLG_SF | EFLG_SYS);
    // Set The CF with the last shifted out bit
    if (s->flags & DEST_BITS16) {
        if ((dest >> (16 - src)) & 1) {
            thread.EFlags |= EFLG_CF;
        }
    } else if (s->flags & DEST_BITS8) {
        if ((dest >> (8 - src)) & 1) {
            thread.EFlags |= EFLG_CF;
        }
    } else {
        if ((dest >> (32 - src)) & 1) {
            thread.EFlags |= EFLG_CF;
        }
    }
    // cout << "dest= "<<(int*)dest << "\nresult= " << (int*)result<<"\nflags= "<< (int*)thread.EFlags << "\n";
    return 0;
}

// ---------------------------------------------------------------------------------
// SHR
int op_shr(Thread & thread, DISASM_INSTRUCTION * s) {
    // first we will test the source and get the value that we will put in the dest in src variable
    DWORD dest, result;
    int   src = 0;

    if (s->flags & SRC_IMM) {
        src = s->nsrc & 0x1F; // not bigger than 31
    } else if (s->flags & SRC_REG) {
        DWORD src2 = thread.Exx[1];
            memcpy(&src, &src2, 1);
    }
    // now we have the value of the src that we will put it in the dest now we will test the dest
    if (s->flags & DEST_REG) {
        dest = thread.Exx[s->ndest];
        DWORD n = thread.Exx[s->ndest];
        if (s->flags & DEST_BITS32) {
            n = n >> src;
            memcpy(&thread.Exx[s->ndest], &n, 4);
            result = thread.Exx[s->ndest];
        }
        if (s->flags & DEST_BITS16) {
            n &= 0xffff;
            n  = n >> src;
            memcpy(&thread.Exx[s->ndest], &n, 2);
            result = thread.Exx[s->ndest];
        }
        if (s->flags & DEST_BITS8) {
            unsigned char * dest2 = (unsigned char *) &thread.Exx[s->ndest];
            if (s->ndest > 3) {
                dest2 = (unsigned char *) (&thread.Exx[s->ndest - 4]);
                dest2++;
            }
            dest   = *dest2;
            n      = 0;
            n      = *dest2 & 0xff;
            *dest2 = n >> src;
            result = *dest2 & 0xff;
        }
    } else if (s->flags & DEST_RM) {
        DWORD * ptr;
        EMU_READ_MEM(ptr, (DWORD) modrm_calc(thread, s));
        DWORD n = *ptr;
        dest = n;
        if (s->flags & DEST_BITS32) {
            DWORD n = *ptr;
            n = (n & 0xffffffff) >> src;
            EMU_WRITE_MEM((DWORD) modrm_calc(thread, s), (DWORD) 4, (unsigned char *) &n);
        }
        if (s->flags & DEST_BITS16) {
            unsigned short n = *ptr;
            n = (n & 0xffff) >> src;
            EMU_WRITE_MEM((DWORD) modrm_calc(thread, s), (DWORD) 2, (unsigned char *) &n);
        }
        if (s->flags & DEST_BITS8) {
            unsigned char n = *ptr;
            n = (n & 0xff) >> src;
            EMU_WRITE_MEM((DWORD) modrm_calc(thread, s), (DWORD) 1, (unsigned char *) &n);
        }
        result = *ptr;
    }
    thread.updateflags(dest, 0, result, 0, s->flags);
    thread.EFlags &= (EFLG_PF | EFLG_ZF | EFLG_SF | EFLG_SYS);
    if ((dest >> (src - 1)) & 1) {
        thread.EFlags |= EFLG_CF;
    }
    // cout << "dest= "<<(int*)dest << "\nresult= " << (int*)result<<"\nflags= "<< (int*)thread.EFlags << "\n";
    return 0;
}

// ---------------------------------------------------------------------------------
// ROL
int op_rol(Thread & thread, DISASM_INSTRUCTION * s) {
    // first we will test the source and get the value that we will put in the dest in src variable
    DWORD dest, result;
    int   src = 0;

    if (s->flags & SRC_IMM) {
        src = s->nsrc;
    } else if (s->flags & SRC_REG) {
        DWORD src2 = thread.Exx[1];
            memcpy(&src, &src2, 1);
    }
    // now we have the value of the src that we will put it in the dest now we will test the dest
    if (s->flags & DEST_REG) {
        dest = thread.Exx[s->ndest];
        DWORD n = thread.Exx[s->ndest];
        if (s->flags & DEST_BITS32) {
            src = src & 0x1F;
            n   = ((n & 0xffffffff) << src) + ((n & 0xffffffff) >> (32 - src));
            memcpy(&thread.Exx[s->ndest], &n, 4);
            result = thread.Exx[s->ndest];
        }
        if (s->flags & DEST_BITS16) {
            src = src & 0xF;
            n   = ((n & 0xffff) << src) + ((n & 0xffff) >> (16 - src));
            memcpy(&thread.Exx[s->ndest], &n, 2);
            result = thread.Exx[s->ndest];
        }
        if (s->flags & DEST_BITS8) {
            unsigned char * dest2 = (unsigned char *) &thread.Exx[s->ndest];
            if (s->ndest > 3) {
                dest2 = (unsigned char *) (&thread.Exx[s->ndest - 4]);
                dest2++;
            }
            n      = *dest2;
            src    = src & 7;
            *dest2 = ((n & 0xff) << src) + ((n & 0xff) >> (8 - src));
            result = *dest2 & 0xff;
        }
    } else if (s->flags & DEST_RM) {
        DWORD * ptr;
        EMU_READ_MEM(ptr, (DWORD) modrm_calc(thread, s));
        DWORD n = *ptr;
        dest = n;
        if (s->flags & DEST_BITS32) {
            src = src & 0x1F;
            DWORD n = *ptr;
            n = (n << src) + ((n & 0xffffffff) >> 32 - src);
            EMU_WRITE_MEM((DWORD) modrm_calc(thread, s), (DWORD) 4, (unsigned char *) &n);
        }
        if (s->flags & DEST_BITS16) {
            src = src & 0xF;
            unsigned short n = *ptr;
            n = (n << src) + ((n & 0xffff) >> 16 - src);
            EMU_WRITE_MEM((DWORD) modrm_calc(thread, s), (DWORD) 2, (unsigned char *) &n);
        }
        if (s->flags & DEST_BITS8) {
            src = src & 0x7;
            unsigned char n = *ptr;
            n = (n << src) + ((n & 0xff) >> (8 - src));
            EMU_WRITE_MEM((DWORD) modrm_calc(thread, s), (DWORD) 1, (unsigned char *) &n);
        }
        result = *ptr;
    }
    thread.updateflags(dest, 0, result, 0, s->flags);
    thread.EFlags &= (EFLG_PF | EFLG_ZF | EFLG_SF | EFLG_SYS);
    if (s->flags & DEST_BITS16) {
        if ((dest >> (16 - src)) & 1) {
            thread.EFlags |= EFLG_CF;
        }
    } else if (s->flags & DEST_BITS8) {
        if ((dest >> (8 - src)) & 1) {
            thread.EFlags |= EFLG_CF;
        }
    } else {
        if ((dest >> (32 - src)) & 1) {
            thread.EFlags |= EFLG_CF;
        }
    }

    // cout << "dest= "<<(int*)dest << "\nresult= " << (int*)result<<"\nflags= "<< (int*)thread.EFlags << "\n";
    return 0;
}

// ---------------------------------------------------------------------------------
// ROR
int op_ror(Thread & thread, DISASM_INSTRUCTION * s) {
    // first we will test the source and get the value that we will put in the dest in src variable
    DWORD dest, result;
    int   src = 0;

    if (s->flags & SRC_IMM) {
        src = s->nsrc;
    } else if (s->flags & SRC_REG) {
        DWORD src2 = thread.Exx[1];
            memcpy(&src, &src2, 1);
    }
    // now we have the value of the src that we will put it in the dest now we will test the dest
    if (s->flags & DEST_REG) {
        dest = thread.Exx[s->ndest];
        DWORD n = thread.Exx[s->ndest];
        if (s->flags & DEST_BITS32) {
            src = src & 0x1F;
            n   = ((n & 0xffffffff) >> src) + ((n & 0xffffffff) << (32 - src));
            memcpy(&thread.Exx[s->ndest], &n, 4);
            result = thread.Exx[s->ndest];
        }
        if (s->flags & DEST_BITS16) {
            src = src & 0xF;
            n   = ((n & 0xffff) >> src) + ((n & 0xffff) << (16 - src));
            memcpy(&thread.Exx[s->ndest], &n, 2);
            result = thread.Exx[s->ndest];
        }
        if (s->flags & DEST_BITS8) {
            unsigned char * dest2 = (unsigned char *) &thread.Exx[s->ndest];
            if (s->ndest > 3) {
                dest2 = (unsigned char *) (&thread.Exx[s->ndest - 4]);
                dest2++;
            }
            src    = src & 0x7;
            *dest2 = ((*dest2 & 0xff) >> src) + ((*dest2 & 0xff) << (8 - src));
            result = *dest2 & 0xff;
        }
    } else if (s->flags & DEST_RM) {
        DWORD * ptr;
        EMU_READ_MEM(ptr, (DWORD) modrm_calc(thread, s));
        DWORD n = *ptr;
        dest = n;
        if (s->flags & DEST_BITS32) {
            src = src & 0x1F;
            DWORD n = *ptr;
            n = ((n & 0xffffffff) >> src) + ((n & 0xffffffff) << 32 - src);
            // __asm(
            // "mov %cl,(src)"
            // "rorl (n),%cl"
            // );
            EMU_WRITE_MEM((DWORD) modrm_calc(thread, s), (DWORD) 4, (unsigned char *) &n);
        }
        if (s->flags & DEST_BITS16) {
            src = src & 0xF;
            unsigned short n = *ptr;
            n = ((n & 0xffff) >> src) + ((n & 0xffff) << 16 - src);
            EMU_WRITE_MEM((DWORD) modrm_calc(thread, s), (DWORD) 2, (unsigned char *) &n);
        }
        if (s->flags & DEST_BITS8) {
            src = src & 0x7;
            unsigned char n = *ptr;
            n = ((n & 0xff) >> src) + ((n & 0xff) << (8 - src));
            EMU_WRITE_MEM((DWORD) modrm_calc(thread, s), (DWORD) 1, (unsigned char *) &n);
        }
        result = *ptr;
    }
    thread.updateflags(dest, 0, result, 0, s->flags);
    thread.EFlags &= (EFLG_PF | EFLG_ZF | EFLG_SF | EFLG_SYS);
    if ((dest >> (src - 1)) & 1) {
        thread.EFlags |= EFLG_CF;
    }
    return 0;
}

// =====================================================================================================
// SAR
int op_sar(Thread & thread, DISASM_INSTRUCTION * s) {
    // first we will test the source and get the value that we will put in the dest in src variable
    DWORD dest, result;
    int   src = 0;

    if (s->flags & SRC_IMM) {
        src = s->nsrc & 0x1F; // not bigger than 31
    } else if (s->flags & SRC_REG) {
        DWORD src2 = thread.Exx[1];
        memcpy(&src, &src2, 1);
    }
    // now we have the value of the src that we will put it in the dest now we will test the dest
    if (s->flags & DEST_REG) {
        dest = thread.Exx[s->ndest];
        DWORD n = thread.Exx[s->ndest];
		__asm{
			mov ecx, src
			mov edx, n
			sar edx, cl
			mov n, edx
		}
        if (s->flags & DEST_BITS32) {
            memcpy(&thread.Exx[s->ndest], &n, 4);
            result = thread.Exx[s->ndest];
        }
        if (s->flags & DEST_BITS16) {
            memcpy(&thread.Exx[s->ndest], &n, 2);
            result = thread.Exx[s->ndest];
        }
        if (s->flags & DEST_BITS8) {
            if (s->ndest > 3) {
                n = thread.Exx[s->ndest - 4];
                __asm
				{
					mov ecx,src
					mov edx,n
					sar dh,cl
					mov n,edx
				}
                memcpy(&thread.Exx[s->ndest - 4], &n, 2);
            } else {
				__asm
				{
					mov ecx, src
					mov edx, n
					sar edx, cl
					mov n, edx
				}
                result = n & 0xff;
                memcpy(&thread.Exx[s->ndest], &n, 1);
            }
        }
    } else if (s->flags & DEST_RM) {
        DWORD * ptr;
        EMU_READ_MEM(ptr, (DWORD) modrm_calc(thread, s));
        DWORD n = *ptr;
        dest = n;
        if (s->flags & DEST_BITS32) {
            DWORD n   = *ptr;
            DWORD num = (n & 0xffffffff);
            __asm
			{
				mov ecx, src
				mov edx, n
				sar edx, cl
				mov n, edx
			}
            // n=(n & 0xffffffff) >> src;
            EMU_WRITE_MEM((DWORD) modrm_calc(thread, s), (DWORD) 4, (unsigned char *) &num);
        }
        if (s->flags & DEST_BITS16) {
            unsigned short n   = *ptr;
            DWORD          num = n & 0xffff;
			__asm
			{
				mov ecx, src
				mov dx, n
				sar dx, cl
				mov num,edx
			}
            // n=(n & 0xffff) >> src;
            EMU_WRITE_MEM((DWORD) modrm_calc(thread, s), (DWORD) 2, (unsigned char *) &num);
        }
        if (s->flags & DEST_BITS8) {
            unsigned char  n   = *ptr;
            DWORD num = n & 0xff;
			__asm
			{
				mov ecx, src
				movzx edx, n
				sar dl, cl
				mov num,edx
			}
            // n=(n & 0xff) >> src;
            EMU_WRITE_MEM((DWORD) modrm_calc(thread, s), (DWORD) 1, (unsigned char *) &num);
        }
        result = *ptr;
    }
    thread.updateflags(dest, 0, result, 0, s->flags);
    thread.EFlags &= (EFLG_PF | EFLG_ZF | EFLG_SF | EFLG_SYS);
    if ((dest >> (src - 1)) & 1) {
        thread.EFlags |= EFLG_CF;
    }
    // cout << "dest= "<<(int*)dest << "\nresult= " << (int*)result<<"\nflags= "<< (int*)thread.EFlags << "\n";
    return 0;
}

// ---------------------------------------------------------------------------------
// RCL
int op_rcl(Thread & thread, DISASM_INSTRUCTION * s) {
    // first we will test the source and get the value that we will put in the dest in src variable
    DWORD dest, result;
    int   src = 0;

    if (s->flags & SRC_IMM) {
        src = s->nsrc & 0x1F; // not bigger than 31
    } else if (s->flags & SRC_REG) {
        DWORD src2 = thread.Exx[1];
        memcpy(&src, &src2, 1);
    }
    // now we have the value of the src that we will put it in the dest now we will test the dest
    if (s->flags & DEST_REG) {
        dest = thread.Exx[s->ndest];
        DWORD n = thread.Exx[s->ndest];
        if (s->flags & DEST_BITS32) {
			__asm{clc};
            if (thread.EFlags & EFLG_CF) {
				__asm{stc};
            }
			__asm{
				mov ecx,src
				mov edx,n
				rcr edx,cl
				mov n,edx
			}
            memcpy(&thread.Exx[s->ndest], &n, 4);
        }
        if (s->flags & DEST_BITS16) {
			__asm{clc};
            if (thread.EFlags & EFLG_CF) {
				__asm{stc};
            }
			__asm
			{
				mov ecx,src
				mov edx,n
				rcl dx,cl
				mov n,edx
			}
            memcpy(&thread.Exx[s->ndest], &n, 2);
        }
        if (s->flags & DEST_BITS8) {
            if (s->ndest < 4) {
				__asm{clc};
                if (thread.EFlags & EFLG_CF) {
					__asm{stc};
                }
				__asm
				{
					mov ecx,src
					mov edx,n
					rcl dl,cl
					mov n,edx
				}
                memcpy(&thread.Exx[s->ndest], &n, 1);
            } else {
                n = thread.Exx[s->ndest - 4];
				__asm{clc};
                if (thread.EFlags & EFLG_CF) {
					__asm{stc};
                }
                __asm
				{
					mov ecx,src
					mov edx,n
					rcl dh,cl
					mov n,edx
				}
                memcpy(&thread.Exx[s->ndest - 4], &n, 4);
            }
        }
        result = thread.Exx[s->ndest];
    } else if (s->flags & DEST_RM) {
        DWORD * ptr;
        EMU_READ_MEM(ptr, (DWORD) modrm_calc(thread, s));
        DWORD n = *ptr;
        dest = n;
        if (s->flags & DEST_BITS32) {
            DWORD n = *ptr;
			__asm{clc};
            if (thread.EFlags & EFLG_CF) {
				__asm{stc};
            }
            __asm
			{
				mov ecx,src
				mov edx,n
				rcl edx,cl
				mov n,edx
			}
            EMU_WRITE_MEM((DWORD) modrm_calc(thread, s), (DWORD) 4, (unsigned char *) &n);
        }
        if (s->flags & DEST_BITS16) {
            unsigned short n = *ptr;
			__asm{clc};
            if (thread.EFlags & EFLG_CF) {
				__asm{stc};
            }
            __asm
			{
				mov ecx,src
				movzx edx,n
				rcl dx,cl
				mov n,dx
			}
            EMU_WRITE_MEM((DWORD) modrm_calc(thread, s), (DWORD) 2, (unsigned char *) &n);
        }
        if (s->flags & DEST_BITS8) {
            unsigned char n = *ptr;
			__asm{clc};
            if (thread.EFlags & EFLG_CF) {
				__asm{stc};
            }
            __asm
			{
				mov ecx,src
				movzx edx,n
				rcl dl,cl
				mov n,dl
			}
            EMU_WRITE_MEM((DWORD) modrm_calc(thread, s), (DWORD) 1, (unsigned char *) &n);
        }
        result = *ptr;
    }
    thread.updateflags(dest, 0, result, 0, s->flags);
    thread.EFlags &= (EFLG_PF | EFLG_ZF | EFLG_SF | EFLG_SYS);
    if (s->flags & DEST_BITS16) {
        if ((dest >> (16 - src)) & 1) {
            thread.EFlags |= EFLG_CF;
        }
    } else if (s->flags & DEST_BITS8) {
        if ((dest >> (8 - src)) & 1) {
            thread.EFlags |= EFLG_CF;
        }
    } else {
        if ((dest >> (32 - src)) & 1) {
            thread.EFlags |= EFLG_CF;
        }
    }
    // cout << "dest= "<<(int*)dest << "\nresult= " << (int*)result<<"\nflags= "<< (int*)thread.EFlags << "\n";
    return 0;
}

// ---------------------------------------------------------------------------------
// RCR
int op_rcr(Thread & thread, DISASM_INSTRUCTION * s) {
    // first we will test the source and get the value that we will put in the dest in src variable
    DWORD dest, result;
    int   src = 0;

    if (s->flags & SRC_IMM) {
        src = s->nsrc & 0x1F; // not bigger than 31
    } else if (s->flags & SRC_REG) {
        DWORD src2 = thread.Exx[1];
        memcpy(&src, &src2, 1);
    }
    // now we have the value of the src that we will put it in the dest now we will test the dest
    if (s->flags & DEST_REG) {
        dest = thread.Exx[s->ndest];
        int n = thread.Exx[s->ndest];
        if (s->flags & DEST_BITS32) {
			__asm{clc};
            if (thread.EFlags & EFLG_CF) {
				__asm{stc};
            }
            __asm
			{
				mov ecx,src
				mov edx,n
				rcr edx,cl
				mov n,edx
			}
            memcpy(&thread.Exx[s->ndest], &n, 4);
        }
        if (s->flags & DEST_BITS16) {
			__asm{clc};
            if (thread.EFlags & EFLG_CF) {
				__asm{stc};
            }
            __asm
			{
				mov ecx,src
				mov edx,n
				rcr dx,cl
				mov n,edx
			}
            memcpy(&thread.Exx[s->ndest], &n, 2);
        }
        if (s->flags & DEST_BITS8) {
            if (s->ndest < 4) {
				__asm{clc};
                if (thread.EFlags & EFLG_CF) {
					__asm{stc};
                }
				__asm
				{
					mov ecx,src
					mov edx,n
					rcr dl,cl
					mov n,edx
				}
                memcpy(&thread.Exx[s->ndest], &n, 1);
            } else {
				__asm{clc};
                if (thread.EFlags & EFLG_CF) {
					__asm{stc};
                }
                n = thread.Exx[s->ndest - 4];
                __asm
				{
					mov ecx,src
					mov edx,n
					rcr dh,cl
					mov n,edx
				}
                memcpy(&thread.Exx[s->ndest - 4], &n, 4);
            }
        }
        result = thread.Exx[s->ndest];
    } else if (s->flags & DEST_RM) {
        DWORD * ptr;
        EMU_READ_MEM(ptr, (DWORD) modrm_calc(thread, s));
        DWORD n = *ptr;
        dest = n;
        if (s->flags & DEST_BITS32) {
            DWORD n = *ptr;
			__asm{clc};
            if (thread.EFlags & EFLG_CF) {
				__asm{stc};
            }
			__asm
			{
				mov ecx,src
				mov edx,n
				rcr edx,cl
				mov n,edx
			}
            EMU_WRITE_MEM((DWORD) modrm_calc(thread, s), (DWORD) 4, (unsigned char *) &n);
        }
        if (s->flags & DEST_BITS16) {
            unsigned short n = *ptr;
			__asm{clc};
            if (thread.EFlags & EFLG_CF) {
				__asm{stc};
            }
            __asm
			{
				mov ecx,src
				movzx edx,n
				rcr dx,cl
				mov n,dx
			}
            EMU_WRITE_MEM((DWORD) modrm_calc(thread, s), (DWORD) 2, (unsigned char *) &n);
        }
        if (s->flags & DEST_BITS8) {
            unsigned char n = *ptr;
			__asm{clc};
            if (thread.EFlags & EFLG_CF) {
				__asm{stc};
            }
			__asm
			{
				mov ecx,src
				movzx edx,n
				rcr dl,cl
				mov n,dl
			}
            EMU_WRITE_MEM((DWORD) modrm_calc(thread, s), (DWORD) 1, (unsigned char *) &n);
        }
        result = *ptr;
    }
    thread.updateflags(dest, 0, result, 0, s->flags);
    thread.EFlags &= (EFLG_PF | EFLG_ZF | EFLG_SF | EFLG_SYS);
    if ((dest >> (src - 1)) & 1) {
        thread.EFlags |= EFLG_CF;
    }
    // cout << "dest= "<<(int*)dest << "\nresult= " << (int*)result<<"\nflags= "<< (int*)thread.EFlags << "\n";
    return 0;
}

// ----------------------------------------------------------------------
// STC
int op_stc(Thread & thread, DISASM_INSTRUCTION * s) {
    thread.EFlags |= EFLG_CF;
    return 0;
}

// CLC
int op_clc(Thread & thread, DISASM_INSTRUCTION * s) {
    thread.EFlags &= ~EFLG_CF;
    return 0;
}
