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

// global variables
// ----------------
int   pages = 0;            // the number of pages that our buffer use (no of bytes = pages*0x1000)
DWORD mem   = 0;            // pointer to our buffer
int   cur   = 0;            // the place where we are in the buffer
// -------
// Arrays
// ------

string nums[10] = {
    "0", "1", "2", "3", "4", "5", "6", "7", "8", "9"
};
string numshex[16] = {
    "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "a", "b", "c", "d", "e", "f"
};
string eip[1] = {
    "eip"
};
string num_identifier[2] = {
    "h", "b"
};
string level4[2] = {
    "&&", "||"
};
string level32[2] = {
    ">", "<"
};
string level3[4] = {
    ">=", "<=", "==", "!="
};
string level2[4] = {
    "+", "-", "|", "#"
};
string level1[4] = {
    "*", "/", "&", "%"
};
string level0[4] = {
    "~", "-"
};
// --------------------------
DWORD AsmDebugger::parser(string s) {
    pages = 0;
    cur   = 0;
    mem   = (DWORD) malloc(0x1000);
    memset((char *) mem, 0, 0x1000);
    pages++;
    s = to_lower_case(s);
    add_to_buffer(process->getsystem()->assembl("push esi"));
    add_to_buffer(process->getsystem()->assembl("push edi"));
    add_to_buffer(process->getsystem()->assembl("push ecx"));
    add_to_buffer(process->getsystem()->assembl("push edx"));
    add_to_buffer(process->getsystem()->assembl("mov esi,ecx"));
    add_to_buffer(process->getsystem()->assembl("mov edi,edx"));
    boolexp(s);
    add_to_buffer(process->getsystem()->assembl("pop edx"));
    add_to_buffer(process->getsystem()->assembl("pop ecx"));
    add_to_buffer(process->getsystem()->assembl("pop edi"));
    add_to_buffer(process->getsystem()->assembl("pop esi"));
    add_to_buffer(process->getsystem()->assembl("ret"));
    return mem;
}

DWORD AsmDebugger::boolexp(string & s) {
    bool con = true; // continue the loop

    boolexp2(s);
    while (con) {
        s = trim(s);
        int n = compare_array(s, level4, 2, 2);
        if (n != 0) {
            s = s.substr(2, s.size());
            switch (n)
			{
				case 1:
				{
					doandbool(s);
					break;
				}

				case 2:
				{
					doorbool(s);
					break;
				}
            }
        } else {
            con = false;
        }
    }
	return 0;
}

DWORD AsmDebugger::boolexp2(string & s) {
    bool con = true; // continue the loop

    mathexp(s);
    while (con) {
        s = trim(s);
        int n = compare_array(s, level3, 4, 2);
        if (n == 0) {
            n = compare_array(s, level32, 2, 1);
            if (n > 0) {
                n += 4;
            }
        }
        if (n != 0) {
            if (n > 4) {
                s = s.substr(1, s.size());
            } else {
                s = s.substr(2, s.size());
            }
            switch (n) {
            case 1: {
                dogreaterequal(s);
                break;
            }

            case 2: {
                dolowerequal(s);
                break;
            }

            case 3: {
                doequal(s);
                break;
            }

            case 4: {
                donotequal(s);
                break;
            }

            case 5: {
                dogreater(s);
                break;
            }

            case 6: {
                dolower(s);
                break;
            }
            }
        } else {
            con = false;
        }
        // con=false;
    }
	return 0;
}

DWORD AsmDebugger::mathexp(string & s) {
    bool con = true; // continue the loop

    mulexp(s);
    while (con) {
        s = trim(s);
        int n = compare_array(s, level2, 4, 1);
        if ((n == 3) && (compare_array(s, level4, 2, 2) == 2)) {
            n = 0;
        }
        if (n != 0) {
            s = s.substr(1, s.size());
            switch (n) {
            case 1: {
                doadd(s);
                break;
            }

            case 2: {
                dosub(s);
                break;
            }

            case 3: {
                door(s);
                break;
            }

            case 4: {
                doxor(s);
                break;
            }
            }
        } else {
            con = false;
        }
        // con=false;
    }
	return 0;
}

DWORD AsmDebugger::mulexp(string & s) {
    bool con = true; // continue the loop

    getnum(s);
    while (con) {
        s = trim(s);
        int n = compare_array(s, level1, 4, 1);
        if ((n == 3) && (compare_array(s, level4, 2, 2) == 1)) {
            n = 0;
        }
        if (n != 0) {
            s = s.substr(1, s.size());
            switch (n) {
            case 1: {
                domul(s);
                break;
            }

            case 2: {
                dodiv(s);
                break;
            }

            case 3: {
                doand(s);
                break;
            }

            case 4: {
                domod(s);
                break;
            }
            }
        } else {
            con = false;
        }
        // con=false;
    }
	return 0;
}

DWORD AsmDebugger::getnum(string & s) {
    s = trim(s);
    string s2;
	if (s.substr(0, 1).compare("(")== 0) {
        s = s.substr(1, s.size());
        boolexp(s);
		if (s.substr(0, 1).compare(")")== 0) {
            s = s.substr(1, s.size());
        } else {
            lasterror = ((string) "expect a ')' ");
            throw (1);
        }
    } else if (compare_array(s, level0, 2, 1)) { // for not & neg
        int n = compare_array(s, level0, 2, 1);
        s = s.substr(1, s.size());
        switch (n) {
        case 1: {
            donot(s);
            break;
        }

        case 2: {
            doneg(s);
            break;
        }
        }
    } else if (compare_array(s, reg32, 8, 3)) {
        int n = compare_array(s, reg32, 8, 3) - 1;
        s = s.substr(3, s.size());
        doreg32(n);
    } else if (compare_array(s, eip, 1, 3)) {
        s = s.substr(3, s.size());
        doreg32(8);
    } else if (s.substr(0, 2).compare("__")== 0) { // function
        s = s.substr(2, s.size());
        callfunc(s);
    } else if (s.substr(0, 1).compare("'")== 0) {
        strfunc(s);
    } else if (compare_array(s, nums, 10, 1)) {
        int n = 1;
        if (s.substr(0, 2).compare("0x")== 0) {
            s = s.substr(n + 1, s.size());
            n = 0;
            while (true) {
                if (compare_array(s.substr(n, n + 1), numshex, 16, 1)) {
                    n++;
                } else {
                    break;
                }
            }

            s2 = "mov eax,0";
            s2.append(s.substr(0, n));
            s2.append("h");
        } else {
            while (true) {
                if (compare_array(s.substr(n, n + 1), nums, 10, 1)) {
                    n++;
                } else if (compare_array(s.substr(n, n + 1), num_identifier, 2, 1)) {
                    n++;
                } else {
                    break;
                }
            }
            s2 = "mov eax,";
            s2.append(s.substr(0, n));
        }
        add_to_buffer(process->getsystem()->assembl(s2));
        s = s.substr(n, s.size());
    } else {
        lasterror = ((string) "expect a number");
        throw (1);
    }
	return 0;
}

// -------------------------------------------------------------------------------------
// do Math Functions
// -----------------
DWORD AsmDebugger::domul(string & s) {
    add_to_buffer(process->getsystem()->assembl("push eax"));
    getnum(s);
    add_to_buffer(process->getsystem()->assembl("mov ecx,eax"));
    add_to_buffer(process->getsystem()->assembl("pop eax"));
    add_to_buffer(process->getsystem()->assembl("mul ecx"));
	return 0;
}

DWORD AsmDebugger::dodiv(string & s) {
    add_to_buffer(process->getsystem()->assembl("push eax"));
    getnum(s);
    add_to_buffer(process->getsystem()->assembl("or eax,eax"));
    add_to_buffer(process->getsystem()->assembl("jnz 5h"));
    add_to_buffer(process->getsystem()->assembl("pop edx"));
    add_to_buffer(process->getsystem()->assembl("pop ecx"));
    add_to_buffer(process->getsystem()->assembl("pop edi"));
    add_to_buffer(process->getsystem()->assembl("pop esi"));
    add_to_buffer(process->getsystem()->assembl("ret"));
    add_to_buffer(process->getsystem()->assembl("mov ecx,eax"));
    add_to_buffer(process->getsystem()->assembl("pop eax"));
    add_to_buffer(process->getsystem()->assembl("xor edx,edx"));
    add_to_buffer(process->getsystem()->assembl("div ecx"));
	return 0;
}

DWORD AsmDebugger::domod(string & s) {
    dodiv(s);
    add_to_buffer(process->getsystem()->assembl("mov eax,edx"));
	return 0;
}

DWORD AsmDebugger::doand(string & s) {
    add_to_buffer(process->getsystem()->assembl("push eax"));
    getnum(s);
    add_to_buffer(process->getsystem()->assembl("pop ecx"));
    add_to_buffer(process->getsystem()->assembl("and eax,ecx"));
	return 0;
}

// -------
DWORD AsmDebugger::doadd(string & s) {
    add_to_buffer(process->getsystem()->assembl("push eax"));
    mulexp(s);
    add_to_buffer(process->getsystem()->assembl("pop ecx"));
    add_to_buffer(process->getsystem()->assembl("add eax,ecx"));
	return 0;
}

DWORD AsmDebugger::dosub(string & s) {
    add_to_buffer(process->getsystem()->assembl("push eax"));
    mulexp(s);
    add_to_buffer(process->getsystem()->assembl("pop ecx"));
    add_to_buffer(process->getsystem()->assembl("sub eax,ecx"));
    add_to_buffer(process->getsystem()->assembl("neg eax"));
	return 0;
}

DWORD AsmDebugger::door(string & s) {
    add_to_buffer(process->getsystem()->assembl("push eax"));
    mulexp(s);
    add_to_buffer(process->getsystem()->assembl("pop ecx"));
    add_to_buffer(process->getsystem()->assembl("or eax,ecx"));
	return 0;
}

DWORD AsmDebugger::doxor(string & s) {
    add_to_buffer(process->getsystem()->assembl("push eax"));
    mulexp(s);
    add_to_buffer(process->getsystem()->assembl("pop ecx"));
    add_to_buffer(process->getsystem()->assembl("xor eax,ecx"));
	return 0;
}

DWORD AsmDebugger::dogreaterequal(string & s) {
    add_to_buffer(process->getsystem()->assembl("push eax"));
    mathexp(s);
    add_to_buffer(process->getsystem()->assembl("pop ecx"));
    add_to_buffer(process->getsystem()->assembl("xchg eax,ecx"));
    add_to_buffer(process->getsystem()->assembl("cmp eax,ecx"));
    add_to_buffer(process->getsystem()->assembl("jge 4"));
    add_to_buffer(process->getsystem()->assembl("xor eax,eax"));
    add_to_buffer(process->getsystem()->assembl("jmp 5"));
    add_to_buffer(process->getsystem()->assembl("mov eax,1"));
	return 0;
}

DWORD AsmDebugger::dolowerequal(string & s) {
    add_to_buffer(process->getsystem()->assembl("push eax"));
    mathexp(s);
    add_to_buffer(process->getsystem()->assembl("pop ecx"));
    add_to_buffer(process->getsystem()->assembl("xchg eax,ecx"));
    add_to_buffer(process->getsystem()->assembl("cmp eax,ecx"));
    add_to_buffer(process->getsystem()->assembl("jle 4"));
    add_to_buffer(process->getsystem()->assembl("xor eax,eax"));
    add_to_buffer(process->getsystem()->assembl("jmp 5"));
    add_to_buffer(process->getsystem()->assembl("mov eax,1"));
	return 0;
}

DWORD AsmDebugger::doequal(string & s) {
    add_to_buffer(process->getsystem()->assembl("push eax"));
    mathexp(s);
    add_to_buffer(process->getsystem()->assembl("pop ecx"));
    add_to_buffer(process->getsystem()->assembl("xchg eax,ecx"));
    add_to_buffer(process->getsystem()->assembl("cmp eax,ecx"));
    add_to_buffer(process->getsystem()->assembl("jz 4"));
    add_to_buffer(process->getsystem()->assembl("xor eax,eax"));
    add_to_buffer(process->getsystem()->assembl("jmp 5"));
    add_to_buffer(process->getsystem()->assembl("mov eax,1"));
	return 0;
}

DWORD AsmDebugger::donotequal(string & s) {
    add_to_buffer(process->getsystem()->assembl("push eax"));
    mathexp(s);
    add_to_buffer(process->getsystem()->assembl("pop ecx"));
    add_to_buffer(process->getsystem()->assembl("xchg eax,ecx"));
    add_to_buffer(process->getsystem()->assembl("cmp eax,ecx"));
    add_to_buffer(process->getsystem()->assembl("jnz 4"));
    add_to_buffer(process->getsystem()->assembl("xor eax,eax"));
    add_to_buffer(process->getsystem()->assembl("jmp 5"));
    add_to_buffer(process->getsystem()->assembl("mov eax,1"));
	return 0;
}

DWORD AsmDebugger::dogreater(string & s) {
    add_to_buffer(process->getsystem()->assembl("push eax"));
    mathexp(s);
    add_to_buffer(process->getsystem()->assembl("pop ecx"));
    add_to_buffer(process->getsystem()->assembl("xchg eax,ecx"));
    add_to_buffer(process->getsystem()->assembl("cmp eax,ecx"));
    add_to_buffer(process->getsystem()->assembl("jg 4"));
    add_to_buffer(process->getsystem()->assembl("xor eax,eax"));
    add_to_buffer(process->getsystem()->assembl("jmp 5"));
    add_to_buffer(process->getsystem()->assembl("mov eax,1"));
	return 0;
}

DWORD AsmDebugger::dolower(string & s) {
    add_to_buffer(process->getsystem()->assembl("push eax"));
    mathexp(s);
    add_to_buffer(process->getsystem()->assembl("pop ecx"));
    add_to_buffer(process->getsystem()->assembl("xchg eax,ecx"));
    add_to_buffer(process->getsystem()->assembl("cmp eax,ecx"));
    add_to_buffer(process->getsystem()->assembl("jl 4"));
    add_to_buffer(process->getsystem()->assembl("xor eax,eax"));
    add_to_buffer(process->getsystem()->assembl("jmp 5"));
    add_to_buffer(process->getsystem()->assembl("mov eax,1"));
	return 0;
}

DWORD AsmDebugger::doandbool(string & s) {
    add_to_buffer(process->getsystem()->assembl("or eax,eax"));
    add_to_buffer(process->getsystem()->assembl("jz 5"));
    add_to_buffer(process->getsystem()->assembl("mov eax,1"));
    add_to_buffer(process->getsystem()->assembl("push eax"));
    boolexp2(s);
    add_to_buffer(process->getsystem()->assembl("pop ecx"));
    add_to_buffer(process->getsystem()->assembl("or eax,eax"));
    add_to_buffer(process->getsystem()->assembl("jz 5"));
    add_to_buffer(process->getsystem()->assembl("mov eax,1"));
    add_to_buffer(process->getsystem()->assembl("and eax,ecx"));
	return 0;
}

DWORD AsmDebugger::doorbool(string & s) {
    add_to_buffer(process->getsystem()->assembl("or eax,eax"));
    add_to_buffer(process->getsystem()->assembl("jz 5"));
    add_to_buffer(process->getsystem()->assembl("mov eax,1"));
    add_to_buffer(process->getsystem()->assembl("push eax"));
    boolexp2(s);
    add_to_buffer(process->getsystem()->assembl("pop ecx"));
    add_to_buffer(process->getsystem()->assembl("or eax,eax"));
    add_to_buffer(process->getsystem()->assembl("jz 5"));
    add_to_buffer(process->getsystem()->assembl("mov eax,1"));
    add_to_buffer(process->getsystem()->assembl("or eax,ecx"));
	return 0;
}

DWORD AsmDebugger::donot(string & s) {
    getnum(s);
    add_to_buffer(process->getsystem()->assembl("not eax"));
	return 0;
}

DWORD AsmDebugger::doneg(string & s) {
    getnum(s);
    add_to_buffer(process->getsystem()->assembl("neg eax"));
	return 0;
}

DWORD AsmDebugger::doreg32(int n) {
    Thread * s = new Thread();
    DWORD    n2;

    if (n == 8) {
        n2 = (DWORD) & s->Eip - (DWORD) s;
    } else {
        n2 = (DWORD) & s->Exx[n] - (DWORD) s;
    }
    string s2 = "mov eax,DWORD ptr [esi+";
    char   s3[3];
    sprintf(s3, "%i", n2);
    // s3[0]=(char)(n2+0x30);
    s3[2] = 0;
    s2.append(s3);
    s2.append("]");
    add_to_buffer(process->getsystem()->assembl(s2));
	return 0;
}

// -------------------------------------------------------------------------------------------
DWORD AsmDebugger::callfunc(string & s) {
    int n     = s.find_first_of("(");
    int nfunc = 0;

    if (n >= s.size()) {
        lasterror = ((string) "expect '(' for fuction call");
        throw (1);
    }
    n++; // to add "("
    string s2 = s.substr(0, n);
    for (int i = 0; i < func_entries; i++) {
        if (s2.compare(funcs[i].name) == 0) {
            nfunc = i;
            goto Func_Found;
        }
    }
    lasterror = ((string) "unknown function ").append(s2);
    throw (1);
Func_Found:
    s = s.substr(n, s.size());
    for (int i = 0; i < funcs[nfunc].params; i++) {
        mathexp(s);
        add_to_buffer(process->getsystem()->assembl("push eax"));
        if ((s.substr(0, 1).compare(",")!= 0) && (s.substr(0, 1).compare(")") != 0)) {
            lasterror = ((string) "missing parameter(s) for function ").append(s2);
            throw (1);
        } else {
            s = s.substr(1, s.size());
        }
    }
    if (funcs[nfunc].params == 0) {
        s = s.substr(1, s.size()); // for ")"
    }
        add_to_buffer(process->getsystem()->assembl("push edi"));
        add_to_buffer(process->getsystem()->assembl("push esi"));
    string s3 = "mov eax,0";
    char   s4[50];
    sprintf(s4, "%x", funcs[nfunc].dbg_func);
    s4[9] = 0;
    s3.append(s4);
    s3.append("h");
    add_to_buffer(process->getsystem()->assembl(s3));
    add_to_buffer(process->getsystem()->assembl("call eax"));
    s3 = "add esp,0";
    char s5[20];
    sprintf(s5, "%x", (funcs[nfunc].params + 2) * 4);
    s5[3] = 0;
    s3.append(s5);
    s3.append("h");
    add_to_buffer(process->getsystem()->assembl(s3));
	return 0;
}

// -------------------------------------------------------------------------------------------
DWORD AsmDebugger::strfunc(string & s) {
    s = s.substr(1, s.size());
    int n = s.find_first_of("'", 0);
    if (n > s.size()) {
        lasterror = "unclosed string ";
        lasterror.append(s);
        throw (1);
    }
    string s2  = s.substr(0, n);
    DWORD  ptr = (DWORD) malloc(s2.size() + 1);
    memset((DWORD *) ptr, 0, s2.size() + 1);
    memcpy((DWORD *) ptr, s2.c_str(), s2.size());
    s = s.substr(n + 1, s.size());
    string s3 = "mov eax,0";
    char   s5[8];
    sprintf(s5, "%x", ptr);
    s5[8] = 0;
    s3.append(s5);
    s3.append("h");
    add_to_buffer(process->getsystem()->assembl(s3));
	return 0;
}

void AsmDebugger::add_to_buffer(bytes * ins) {
    for (int i = 0; i < ins->length; i++) {
        ((char *) mem)[cur] = ins->s[i];
        cur++;
    }
}

// -------------------------------------------------------------------------------------------
