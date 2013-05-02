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

DWORD FPU_push(Thread & thread, double value) {
    for (int i = 0; i < 6; i++) {
        thread.ST[i + 1] = thread.ST[i];
    }
    thread.ST[0] = value;
    // cout << value << "\n";
    // cout << thread.SelectedReg << "\n";
    // cout << thread.ST[thread.SelectedReg] << "\n";
    thread.SelectedReg++;
	return 0;
}

DWORD FPU_pop(Thread & thread) {
    if ((thread.SelectedReg == 1) || (thread.SelectedReg == 0)) {
        thread.SelectedReg = 0;
        return 0;
    }
    for (int i = 0; i < thread.SelectedReg - 1; i++) {
        thread.ST[i] = thread.ST[i + 1];
    }
    thread.SelectedReg--;
	return 0;
}

int op_faddp(Thread & thread, DISASM_INSTRUCTION * s) {
    thread.ST[s->ndest - 1] += thread.ST[0];
    FPU_pop(thread);
	thread.FpuUpdateFlags(s);
    return 0;
}

int op_fild(Thread & thread, DISASM_INSTRUCTION * s) {
    DWORD * ptr;

    EMU_READ_MEM(ptr, (DWORD) modrm_calc(thread, s));
    DWORD n = *ptr;
    // system("pause");
    if (s->flags & FPU_BITS32) {
        double value = 0.0;
        value = n;
        FPU_push(thread, n);
    } else {
        FPU_push(thread, (n & 0xFFFF));
    }
	thread.FpuUpdateFlags(s);
    return 0;
}

int op_fistp(Thread & thread, DISASM_INSTRUCTION * s) {
    int ST_in_int = thread.ST[0];

    if (s->flags & FPU_BITS32) {
        EMU_WRITE_MEM((DWORD) modrm_calc(thread, s), 4, (unsigned char *) &ST_in_int);
    } else {
        ST_in_int &= 0xFFFF;
        EMU_WRITE_MEM((DWORD) modrm_calc(thread, s), 2, (unsigned char *) &ST_in_int);
    }
    FPU_pop(thread);
	thread.FpuUpdateFlags(s);
    return 0;
}

int op_fimul(Thread & thread, DISASM_INSTRUCTION * s) {
    DWORD * ptr;

    EMU_READ_MEM(ptr, (DWORD) modrm_calc(thread, s));
    DWORD n = *ptr;
    if (s->flags & FPU_BITS32) {
        thread.ST[0] *= n;
    } else {
        thread.ST[0] *= (n & 0xFFFF);
    }
	thread.FpuUpdateFlags(s);
    return 0;
}

int op_fnstenv(Thread &thread, DISASM_INSTRUCTION * s) {

	EMU_WRITE_MEM((DWORD) modrm_calc(thread, s), sizeof(thread.FpuEnv), (unsigned char *) &thread.FpuEnv);
	thread.FpuUpdateFlags(s);
    return 0;
}