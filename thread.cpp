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
#ifdef WIN32
#include "windows.h"
#endif
bool is_negative(DWORD num, DWORD ins_flags);

char PF_Flags[256] = {
    1, 0, 0, 1, 0, 1, 1, 0,
    0, 1, 1, 0, 1, 0, 0, 1,
    0, 1, 1, 0, 1, 0, 0, 1,
    1, 0, 0, 1, 0, 1, 1, 0,
    0, 1, 1, 0, 1, 0, 0, 1,
    1, 0, 0, 1, 0, 1, 1, 0,
    1, 0, 0, 1, 0, 1, 1, 0,
    0, 1, 1, 0, 1, 0, 0, 1,
    0, 1, 1, 0, 1, 0, 0, 1,
    1, 0, 0, 1, 0, 1, 1, 0,
    1, 0, 0, 1, 0, 1, 1, 0,
    0, 1, 1, 0, 1, 0, 0, 1,
    1, 0, 0, 1, 0, 1, 1, 0,
    0, 1, 1, 0, 1, 0, 0, 1,
    0, 1, 1, 0, 1, 0, 0, 1,
    1, 0, 0, 1, 0, 1, 1, 0,
    0, 1, 1, 0, 1, 0, 0, 1,
    1, 0, 0, 1, 0, 1, 1, 0,
    1, 0, 0, 1, 0, 1, 1, 0,
    0, 1, 1, 0, 1, 0, 0, 1,
    1, 0, 0, 1, 0, 1, 1, 0,
    0, 1, 1, 0, 1, 0, 0, 1,
    0, 1, 1, 0, 1, 0, 0, 1,
    1, 0, 0, 1, 0, 1, 1, 0,
    1, 0, 0, 1, 0, 1, 1, 0,
    0, 1, 1, 0, 1, 0, 0, 1,
    0, 1, 1, 0, 1, 0, 0, 1,
    1, 0, 0, 1, 0, 1, 1, 0,
    0, 1, 1, 0, 1, 0, 0, 1,
    1, 0, 0, 1, 0, 1, 1, 0,
    1, 0, 0, 1, 0, 1, 1, 0,
    1, 0, 0, 1, 0, 1, 1, 0,
};
Thread::Thread() {
    // this for the parser as we don't need anything from the thread just work in pointers
}

Process * Thread::GetProcess() {
    return process;
}

Thread::Thread(DWORD neip, Process & s) {
    // initialize the thread

    process = &s;
    // create the stack
    // it's created from tests only and I didn't use the Heap Commit & Reserve
    // I use Size = 0xA000
    seh_enable = true;
#ifdef WIN32
    DWORD x = (DWORD) VirtualAlloc(NULL, 0xA000, MEM_COMMIT, PAGE_READWRITE); // the virtual place
#else
    DWORD x = (DWORD) malloc(0xA000); // the virtual place
#endif
    memset((void *) x, 0, 0xA000);
    stack = new Stack(*this);
    mem   = process->SharedMem;
	DWORD StackAddr = mem->create_memory_address(VMEM_TYPE_STACK);
    mem->add_pointer(x, StackAddr, 0xA000);
    Exx[4] = StackAddr + 0x9F90; // esp
    Exx[5] = StackAddr + 0x9F94; // ebp

    // preparing the TIB,TEB
    CreateTEB();

	still_tls = false;
	if (process->AppType != PROCESS_SHELLCODE)
	{
		DWORD          image        = (DWORD) s.SharedMem->read_virtual_mem(s.GetImagebase());
		DWORD          PEHeader_ptr = ((dos_header *) image)->e_lfanew + image;
		image_header * PEHeader     = (image_header *) PEHeader_ptr;
		if (PEHeader->optional.data_directory[IMAGE_DIRECTORY_ENTRY_TLS].virtual_address != 0) {
			_IMAGE_TLS_DIRECTORY * tlsheader = (_IMAGE_TLS_DIRECTORY *) ((DWORD) PEHeader->optional.data_directory[IMAGE_DIRECTORY_ENTRY_TLS].virtual_address + image);
			if (tlsheader->AddressOfCallBacks != 0) {
				DWORD * callbacks = s.SharedMem->read_virtual_mem((DWORD) tlsheader->AddressOfCallBacks);
				if (callbacks[0] != 0) {
					stack->push(0);
					stack->push(1);
					stack->push(0);
					stack->push(TLS_MAGIC);
					this->Eip = callbacks[0];
					log       = new Log(this->Eip);
					still_tls = true;
					tls_callback_index++;
				}
			}
		}
	}
    entry_point = neip;
    if (still_tls == false) {
        this->Eip = neip;
        log       = new Log(neip);
        if (process->IsDLL) {
            stack->push(0);
            stack->push(1);
            stack->push(process->GetImagebase());
        }
        stack->push(mem->get_virtual_pointer(process->getsystem()->APITable[0].addr)); // pushes the pointer to ExitProcess (some viruses get the kernelbase from it
    }
    // preparing FPU
    SelectedReg = 0;
    for (int i = 0; i < 8; i++)
	{
        ST[i] = 0;
    }
}

void Thread::updateflags(DWORD dest, DWORD src, DWORD result, int flags, DWORD ins_flags) {
    bool CF = false; // reserve The CF

    if ((EFlags & EFLG_CF) && (flags != UPDATEFLAGS_ADD) && (flags != UPDATEFLAGS_SUB)) {
        CF = true; // save the CF before being deleted
    }
    this->EFlags = EFLG_SYS;
    // --------------------------------------------------
    // ZF & SF & OF
    if (result == 0) {
        EFlags |= EFLG_ZF; // zero
    }
    if (is_negative(result, ins_flags)) {
        EFlags |= EFLG_SF; // negative
    }
    if ((flags == UPDATEFLAGS_ADD) && (is_negative(dest, ins_flags) == false) && (is_negative(result, ins_flags) == true)) {
        EFlags |= EFLG_OF; // From Positive to Negative
    }
    if ((flags == UPDATEFLAGS_SUB) && (is_negative(dest, ins_flags) == false) && (is_negative(src, ins_flags) == true) && (is_negative(result, ins_flags) == true)) {
        EFlags |= EFLG_OF; // from positive to negative
    }
    // --------------------------------------------------
    // CF & AF
    if ((flags == UPDATEFLAGS_ADD) && (dest > result)) {
        EFlags |= EFLG_CF | EFLG_AF; // overflow of positive
    }
    if ((flags == UPDATEFLAGS_SUB) && (result > dest)) {
        EFlags |= EFLG_CF | EFLG_AF; // overflow of negative
    }
    // -------------------------------------------------
    // PF
    char PFindex = result & 0xFF;
    if (PF_Flags[PFindex] == 1) {
        EFlags |= EFLG_PF;
    }

    if (CF) {
        EFlags |= EFLG_CF; // restore CF
    }
}

// This function determines if this number is positive or negative based on the operand size
bool is_negative(DWORD num, DWORD ins_flags) {
    if (ins_flags & DEST_BITS8) {
        if (num & 0x80) {
            return true;
        } else {
            return false;
        }
    } else if (ins_flags & DEST_BITS16) {
        if (num & 0x8000) {
            return true;
        } else {
            return false;
        }
    } else {
        if (num & 0x80000000) {
            return true;
        } else {
            return false;
        }
    }
    return false;
}

void Thread::FpuUpdateFlags( DISASM_INSTRUCTION * s)
{
	FpuEnv.LastInstructionPointer = Eip - s->hde.len;
	FpuEnv.LastOpcode = (s->hde.opcode - 0xD8) << 8 + s->hde.modrm;
}
void Thread::CreateTEB() {
#ifdef WIN32
    tib = (TIB *) VirtualAlloc(NULL, sizeof(TEB) + sizeof(TIB), MEM_COMMIT, PAGE_READWRITE); // the virtual place
#else
    tib = (TIB *) malloc(sizeof(TEB) + sizeof(TIB)); // the virtual place
#endif
    memset(tib, 0, sizeof(TEB) + sizeof(TIB));
    teb                = (TEB *) ((DWORD) tib + (DWORD) sizeof(TIB));
    tib->ExceptionList = (_PEXCEPTION_REGISTRATION_RECORD *) 0x0012FFC4;
    int n = 0xFFFFFFFF; // End of SEH Chain
    mem->write_virtual_mem((DWORD) tib->ExceptionList, (DWORD) 4, (unsigned char *) &n);
    mem->write_virtual_mem((DWORD) (tib->ExceptionList + 4), (DWORD) 4, (unsigned char *) &n);
    tib->TIBOffset = 0x7FFDF000; // pointer to SEH Chain
    teb->Peb = (PEB *) 0x7FFD5000;
    this->fs = 0x7FFDF000; // set the fs segment to this place
    mem->add_pointer((DWORD) tib, 0x7FFDF000, sizeof(TEB) + sizeof(TIB));
    DWORD ptr = *mem->read_virtual_mem(GetFS());
}

DWORD Thread::GetFS() {
    return fs;
}

void Thread::TLSContinue() {
    if (still_tls) {
        DWORD                  image        = (DWORD) mem->read_virtual_mem(process->GetImagebase());
        DWORD                  PEHeader_ptr = ((dos_header *) image)->e_lfanew + image;
        image_header         * PEHeader     = (image_header *) PEHeader_ptr;
        _IMAGE_TLS_DIRECTORY * tlsheader    = (_IMAGE_TLS_DIRECTORY *) ((DWORD) PEHeader->optional.data_directory[IMAGE_DIRECTORY_ENTRY_TLS].virtual_address + image);
        DWORD                * callbacks    = mem->read_virtual_mem((DWORD) tlsheader->AddressOfCallBacks);
        if (callbacks[tls_callback_index] != 0) {
            stack->push(0);
            stack->push(1);
            stack->push(0);
            stack->push(TLS_MAGIC);
            this->Eip = callbacks[tls_callback_index];
            log       = new Log(this->Eip);
            still_tls = true;
            tls_callback_index++;
        } else {
            still_tls = false;
            this->Eip = entry_point;
            log       = new Log(entry_point);
        }
    }
}
