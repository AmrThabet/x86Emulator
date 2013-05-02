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

string Debugger::GetLastError() {
    return lasterror;
}

void Debugger::RemoveBp(int index) {
    if (index < nbp) {
        bp[index].state = BP_REMOVE;
        free((DWORD *) bp[index].ptr);
    }
}

void Debugger::PauseBp(int index) {
    if (index < nbp) {
        bp[index].state = BP_PAUSE;
    }
}

void Debugger::ActivateBp(int index) {
    if (index < nbp) {
        if (bp[index].state != BP_REMOVE) {
            bp[index].state = BP_RUN;
        }
    }
}

Debugger::Debugger() {}

bool Debugger::TestBp(Thread & thread, DISASM_INSTRUCTION * ins) {
    return false;
}

int Debugger::AddBp(string s) {
    return 0;
}

int Debugger::define_func(string name, int params, DWORD func, int flags) {
    funcs[func_entries].name     = name.append("(");
    funcs[func_entries].params   = params;
    funcs[func_entries].dbg_func = func;
    funcs[func_entries].flags    = flags;
    func_entries++;
	return (func_entries-1);
}

DWORD readfunc(Thread * thread, DISASM_INSTRUCTION * ins, DWORD ptr) {
    try {
        DWORD * ptr2 = (DWORD *) thread->mem->read_virtual_mem((DWORD) ptr);
        return *ptr2;
    } catch (...) {
        return 0;
    }
}

bool isapi(Thread * thread, DISASM_INSTRUCTION * ins) {
    if (ins->flags & API_CALL) {
        return true;
    }
    return false;
}

bool isapiequal(Thread * thread, DISASM_INSTRUCTION * ins, char * s) {
    if (ins->flags & API_CALL) {
        char * s2 = (char *) to_lower_case(thread->process->getsystem()->GetTiggeredAPI(*thread)).c_str();
        if (!strcmp(s, s2)) {
            return true;
        }
    }
    return false;
}

bool lastaccessed(Thread * thread, DWORD ins, int index) {
    // return thread->mem->get_memory_flags(ptr);
    return thread->mem->get_last_accessed(index);
}

DWORD lastmodified(Thread * thread, DWORD ins, int index) {
    return thread->mem->get_last_modified(index);
}

DWORD dispfunc(Thread * thread, DISASM_INSTRUCTION * ins) {
    for (int i = 0; i < ins->modrm.length; i++) {
        if (ins->modrm.flags[i] & RM_DISP) {
            return ins->modrm.items[i];
        }
    }
    return 0;
}

DWORD immfunc(Thread * thread, DISASM_INSTRUCTION * ins) {
    if (ins->flags & DEST_IMM) {
        return ins->ndest;
    }
    if (ins->flags & SRC_IMM) {
        return ins->nsrc;
    }
    return 0;
}

int isdirty(Thread * thread, DWORD ins, int ptr) {
    return thread->mem->get_memory_flags(ptr);
}

int Debugger::init_funcs() {
    define_func("isdirty",      1, (DWORD) & isdirty,      0);
    define_func("lastmodified", 1, (DWORD) & lastmodified, 0);
    define_func("lastwritten",  1, (DWORD) & lastmodified, 0);
    define_func("lastaccessed", 1, (DWORD) & lastaccessed, 0);
    define_func("isapi",        0, (DWORD) & isapi,        0);
    define_func("rm",           0, (DWORD) & modrm_calc,   0);
    define_func("disp",         0, (DWORD) & dispfunc,     0);
    define_func("imm",          0, (DWORD) & immfunc,      0);
    define_func("read",         1, (DWORD) & readfunc,     0);
    define_func("isapiequal",   1, (DWORD) & isapiequal,   0);
	return 0;
}
