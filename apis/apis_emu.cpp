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

int GetProcAddress_emu(Thread * thread, DWORD * Args) {
    char    * str = 0;
    Process * c   = thread->GetProcess();
    DWORD   * readptr;

    API_READ_MEM(readptr, Args[1]);
    str = (char *) readptr;
    if (str[0] == 0) {
        return 0; // if the string begins with zero
    }
    // invalid pointer return 0
    // if valid don't return zero but return any address
    if (Args[0] == 0) {
        return 0xBBBB0000 + c->imports[c->nimports - 1]->napis << 8 + c->nimports;
    }
    API_READ_MEM(readptr, Args[0]);
    DWORD dllhandle = (DWORD) readptr;
    DWORD ptr       = thread->process->getsystem()->GetAPI(str, dllhandle);
    ptr = thread->mem->get_virtual_pointer(ptr);
    if (c->imports[c->nimports - 1]->napis == (MAX_NUM_APIS_PER_DLL - 1)) {
        return ptr;
    }
    c->imports[c->nimports - 1]->apis[c->imports[c->nimports - 1]->napis] = Args[1];
    c->imports[c->nimports - 1]->napis++;
    if (ptr == 0) {
        // A magic number to search again for it to reconstruct the import table
        // cout << (int*)(0xBBBB0000+((c->imports[c->nimports-1]->napis-1) << 8) +c->nimports-1)<<"\n";
        return 0xBBBB0000 + ((c->imports[c->nimports - 1]->napis - 1) << 8) + c->nimports - 1;
    }
    free(Args);
    return ptr;
}

int GetModuleHandleA_emu(Thread * thread, DWORD * Args) {
    if (Args[0] == 0) {
        return thread->GetProcess()->GetImagebase();
    }
    DWORD * readptr;
    API_READ_MEM(readptr, Args[0]);
    char * str = (char *) readptr;
    DWORD  ptr = thread->process->getsystem()->GetDllBase(str);
    ptr = thread->mem->get_virtual_pointer(ptr);
    Process * c = thread->process;
    c->imports[c->nimports] = (Imports *) malloc(sizeof(Imports));
    memset(c->imports[c->nimports], 0, sizeof(Imports));
    c->imports[c->nimports]->name    = Args[0];
    c->imports[c->nimports]->addr    = ptr;
    c->imports[c->nimports]->defined = true;
    if (ptr == 0) {
        ptr                              = thread->process->getsystem()->DLLs[0].vAddr;
        c->imports[c->nimports]->addr    = 0xBBBBBB00 + c->nimports;
        c->imports[c->nimports]->defined = false;
    }
    c->nimports++;
    return ptr;
}

int LoadLibraryA_emu(Thread * thread, DWORD * Args) {
    DWORD * readptr;

    API_READ_MEM(readptr, Args[0]);
    char * str = (char *) readptr;
    DWORD  ptr = thread->process->getsystem()->GetDllBase(str);
    ptr = thread->mem->get_virtual_pointer(ptr);
    Process * c = thread->process;
    c->imports[c->nimports] = (Imports *) malloc(sizeof(Imports));
    memset(c->imports[c->nimports], 0, sizeof(Imports));
    c->imports[c->nimports]->name    = Args[0];
    c->imports[c->nimports]->addr    = ptr;
    c->imports[c->nimports]->defined = true;
    if (ptr == 0) {
        ptr                              = thread->process->getsystem()->DLLs[0].vAddr;
        c->imports[c->nimports]->addr    = 0xBBBBBB00 + c->nimports;
        c->imports[c->nimports]->defined = false;
    }
    c->nimports++;
    return ptr;
}

int VirtualAlloc_emu(Thread * thread, DWORD * Args) {

    if ((Args[1] & 0x0FFF) != 0)
	{
        Args[1] = (Args[1] & 0xFFFFF000) + 0x1000; // round it to 0x1000
    }
#ifdef WIN32
    DWORD ptr = (DWORD) VirtualAlloc(NULL, Args[1], MEM_COMMIT, PAGE_READWRITE); // the virtual place
#else
    DWORD ptr = (DWORD) malloc(Args[1]); // the virtual place
#endif
    memset((void *) ptr, 0, Args[1]);
    DWORD addr = Args[0]; // the address

    if ((addr == 0) || (thread->mem->read_virtual_mem(addr) == 0))
	{
		addr = thread->mem->create_memory_address(VMEM_TYPE_ALLOC);
    }
    thread->mem->add_pointer(ptr, addr, Args[1]);
    return addr;
}

int VirtualFree_emu(Thread * thread, DWORD * Args) {
    thread->mem->delete_pointer(Args[0]);
    return 1;
}

int VirtualProtect_emu(Thread * thread, DWORD * Args) {
    DWORD   vptr = Args[0];
    DWORD * readptr;

    API_READ_MEM(readptr, Args[0]);
    DWORD rptr = (DWORD) readptr;
    if ((Args[1] & 0x0FFF) != 0) {
        Args[1] = (Args[1] & 0xFFFFF000) + 0x1000; // round it to 0x1000
    }
    DWORD size = Args[1];
	//cout << "Virtual Protect:\n" << "Read Ptr: " << (int*)rptr << "\nVirtual Ptr: " << (int*)vptr << "\nSize: " << (int*)size << "\n";
    thread->mem->add_pointer(rptr, vptr, size, MEM_VIRTUALPROTECT);
    return 1;
}

int SetUnhandledExceptionFilter_emu(Thread * thread, DWORD * Args) {
    thread->stack->push(Args[0]);
    thread->stack->push(*thread->mem->read_virtual_mem(thread->GetFS()));
    *thread->mem->read_virtual_mem(thread->GetFS()) = thread->Exx[4];
	return 1;
}
