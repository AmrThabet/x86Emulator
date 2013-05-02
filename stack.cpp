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
#include "x86emu.h"
Stack::Stack(Thread & s) {
    thread = &s;
}

void Stack::push(DWORD value) {
    thread->mem->write_virtual_mem(thread->Exx[4] - 4, (DWORD) 4, (unsigned char *) &value);
    thread->Exx[4] -= 4;
}

int Stack::pop() {
    int   value;
    int * ptr = (int *) thread->mem->read_virtual_mem(thread->Exx[4]);

    thread->Exx[4] += 4;
    memcpy(&value, ptr, 4);
    return value;
}
