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

#include "../../x86emu.h"
int AsmDebugger::AddBp(string s) {
    bp[nbp].ptr   = parser(s);
    bp[nbp].state = BP_RUN;
    string str;
    DWORD  n = bp[nbp].ptr;

    /*
    for (int i=0;i<24;i++){
        DISASM_INSTRUCTION* ins=process->getsystem()->disasm((DISASM_INSTRUCTION*)malloc(sizeof(DISASM_INSTRUCTION)),(char*)n,str);
        n+=ins->hde.len;
        //cout << str <<"\n";
    };
    */
    nbp++;
    return nbp - 1;
}

bool AsmDebugger::TestBp(int num, Thread & thread, DISASM_INSTRUCTION * ins) {
    if (num >= nbp) {
        return false; // outside the limits
    }
    int b = call_to_func(bp[num].ptr, (DWORD) & thread, (DWORD) ins);
    if (b != 0) {
        return true;
    }
    return false;
}

bool AsmDebugger::TestBp(Thread & thread, DISASM_INSTRUCTION * ins) {
    bool b = false;

    for (int n = 0; n < nbp; n++) {
        int i = n;
        if (bp[i].state == BP_RUN) {
            b = TestBp(i, thread, ins);
        }
        if (b == true) {
            goto YES;
        }
    }
    return false;
YES:
    return true;
}

AsmDebugger::AsmDebugger(Process & c) {
    process      = &c;
    func_entries = 0;
    init_funcs();
    nbp       = 0;
    lasterror = "";
}

int call_to_func(DWORD mem, DWORD a, DWORD c) {
    int b = 0;

    // cout << a << "\n";
	__asm
	{
		mov eax,mem
		mov ecx, a
		mov edx, c
		push ecx
		call eax
		pop ecx
		mov b,eax
	}
    // cout << n << "\n";
    return b;
}
