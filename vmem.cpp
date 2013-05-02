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
#define MAXIMUM_STATIC_SIZE 100

#define VMEM_START_VALLOC	0x00870000
#define VMEM_START_DLL		0x77000000
#define VMEM_START_STACK	0x00126000

VirtualMemory::VirtualMemory()
{
    // it's a dynamic array to vMem* so we will initialize it
    vAllocCommittedPages = VMEM_START_VALLOC;
	DLLCommittedPages = VMEM_START_DLL;
	StackCommittedPages = VMEM_START_STACK;
    vmem           = (vMem **) malloc(MAXIMUM_STATIC_SIZE * 4);
    memset((void *) vmem, 0, 4);
    vmem_length = 0;
    cmem        = (cMem **) malloc(4);
    memset((void *) cmem, 0, 4);
    cmem_length   = 0;
    last_accessed = new Log(0);
    last_modified = new Log(0);
}

//This function create a random address for DLL Imagebase, VirtualProtect or Stack
DWORD VirtualMemory::create_memory_address(int Type)
{
	DWORD* pCommitedPages = NULL;
	if (Type == VMEM_TYPE_STACK)
	{
		pCommitedPages = &StackCommittedPages;
	}
	else if (Type == VMEM_TYPE_DLL)
	{
		pCommitedPages = &DLLCommittedPages;
	}
	else if (Type == VMEM_TYPE_ALLOC)
	{
		pCommitedPages = &vAllocCommittedPages;
	}
	
	if (pCommitedPages == NULL)return 0;

	while(1)
	{
		if (read_virtual_mem(*pCommitedPages) != 0)
		{
			*pCommitedPages += 0x10000;
		}
		else
		{
			break;
		}
	}
	return *pCommitedPages;

}
// DWORD VirtualMemory::add_pointer(DWORD rptr,DWORD vptr,DWORD size){
// add_pointer(rptr,vptr,size,MEM_READWRITE);
// };
void _cdecl VirtualMemory::add_pointer(DWORD rptr, DWORD vptr, DWORD size, int flags) {
	
    if (vmem_length == 0) {
        vmem[0]        = (vMem *) malloc(MAXIMUM_STATIC_SIZE * sizeof(vMem));
		vmem[0]->rmem  = rptr;
        vmem[0]->vmem  = vptr;
        vmem[0]->size  = size;
        vmem[0]->flags = flags;
        vmem_length++;
    } else {
        if (vmem_length >= MAXIMUM_STATIC_SIZE) {
            DWORD c = (DWORD) vmem;
            vmem = (vMem **) realloc((void *) vmem, (vmem_length + 1) * 4);
            memcpy((void *) c, vmem, (vmem_length) * 4);
            vmem[vmem_length] = (vMem *) malloc(sizeof(vMem));
            // if (vmem[vmem_length]==0)vmem[vmem_length]=(vMem*)alloc(sizeof(vMem));
            memset(vmem[vmem_length], 0, 4);
        } else {
            // vmem=(vMem**)realloc((void*)vmem,(vmem_length+1)*4) ;
            vmem[vmem_length] = (vMem*)((DWORD)vmem[vmem_length - 1] + (DWORD)sizeof(vMem));
        }
        vmem[vmem_length]->rmem  = rptr;
        vmem[vmem_length]->vmem  = vptr;
        vmem[vmem_length]->size  = size;
        vmem[vmem_length]->flags = flags; // */
        vmem_length++;
    }
}

DWORD VirtualMemory::get_virtual_pointer(DWORD ptr) {
    for (int i = this->vmem_length - 1; i >= 0; i--) {
        if ((ptr >= vmem[i]->rmem) && (ptr < (vmem[i]->rmem + vmem[i]->size)) && (vmem[i]->size != 0)) {
            ptr -= vmem[i]->rmem;
            ptr += vmem[i]->vmem;
            return ptr;
        }
    }
    // throw(EXP_INVALIDPOINTER);  //we don't need errors this time
    return 0;
}

DWORD * VirtualMemory::read_virtual_mem(DWORD ptr) {
    DWORD vptr = ptr;

    for (int i = this->vmem_length - 1; i >= 0; i--) {
        // cout << (int*)vptr << "   "<<(int*)vmem[i]->vmem << "\n";
        if ((ptr >= vmem[i]->vmem) && (ptr < (vmem[i]->vmem + vmem[i]->size)) && (vmem[i]->size != 0)) {
            ptr -= vmem[i]->vmem;
            ptr += vmem[i]->rmem;
            last_accessed->addlog(vptr);
            return (DWORD *) ptr;
        }
    }
    return 0;
}

bool VirtualMemory::get_memory_flags(DWORD ptr) {
    for (int i = 0; i < this->cmem_length; i++) {
        if ((ptr >= cmem[i]->ptr) && (ptr < (cmem[i]->ptr + cmem[i]->size))) {
            return true;
        }
    }
    // cout << this->cmem_length <<"\n";
    return false;
}

DWORD VirtualMemory::write_virtual_mem(DWORD ptr, DWORD size,unsigned char * buff) {
    int vptr  = ptr;
    int entry = 0;

    for (int i = this->vmem_length - 1; i >= 0; i--) {
        if ((ptr >= vmem[i]->vmem) && (ptr < (vmem[i]->vmem + vmem[i]->size)) && (vmem[i]->size != 0)) {
			
			ptr  -= vmem[i]->vmem;
            ptr  += vmem[i]->rmem;
            entry = i;
			
            goto mem_found;
        }
    }
    return EXP_INVALIDPOINTER;
mem_found:
    if (vmem[entry]->flags == MEM_IMAGEBASE) {
        if (!check_writeaccess(vptr, vmem[entry]->vmem)) {
            return EXP_WRITEACCESS_DENIED;
        }
    }
    if ((vmem[entry]->flags == MEM_READONLY) || (vmem[entry]->flags == MEM_DLLBASE)) {
        return EXP_WRITEACCESS_DENIED;
    }
    memcpy((void *) ptr, buff, size);
    last_modified->addlog(vptr);
    set_memory_flags((DWORD) vptr, size);
    return 0;
}

void VirtualMemory::set_memory_flags(DWORD ptr, int size) {
    for (int i = 0; i < this->cmem_length; i++) {
        if ((ptr >= cmem[i]->ptr) && (ptr < (cmem[i]->ptr + cmem[i]->size))) {
            // so it's allready written
            goto  found_ptr;
        } else if (ptr == (cmem[i]->ptr + cmem[i]->size)) { // here if it's the next DWORD or the next byte (for loop on decrypting something
            cmem[i]->size += size;
            goto  found_ptr;
        } else if ((ptr + size) == cmem[i]->ptr) { // the prev byte or DWORD (decrypting from the end to the top)
            cmem[i]->ptr  -= size;
            cmem[i]->size += size;
            goto  found_ptr;
        }
    }
    // if not found so add it
    if (cmem_length == 0) {
        cmem[0]       = (cMem *) malloc(sizeof(cMem));
        cmem[0]->ptr  = ptr;
        cmem[0]->size = size;
        cmem_length++;
    } else {
        cmem                    = (cMem **) realloc((void *) cmem, (cmem_length + 1) * 4);
        cmem[cmem_length]       = (cMem *) malloc(sizeof(cMem));
        cmem[cmem_length]->ptr  = ptr;
        cmem[cmem_length]->size = size;
        cmem_length++;
    }
found_ptr:;
}

bool VirtualMemory::check_writeaccess(DWORD ptr, DWORD imagebase) {
    // cout << (int*)ptr << "\n"<< (int*)imagebase << "\n";
    image_header         * PEHeader;
    DWORD                  FileHandler, PEHeader_ptr;
    image_section_header * data;

    FileHandler  = (DWORD) read_virtual_mem(imagebase);
    PEHeader_ptr = ((dos_header *) FileHandler)->e_lfanew + FileHandler;
    PEHeader     = (image_header *) PEHeader_ptr;
    if (ptr < (imagebase + PEHeader->optional.section_alignment)) {
        return false;
    }
    ptr -= imagebase;
    image_section_header * sections = (image_section_header *) (PEHeader->header.size_of_optional_header + (DWORD) & PEHeader->optional);
    if (PEHeader->header.number_of_sections != 0) {
        for (int i = 0; i < PEHeader->header.number_of_sections - 1; i++) {
            if ((ptr >= sections[i].virtual_address) && (ptr < (sections[i + 1].virtual_address))) {
                if (sections[i].characteristics & IMAGE_SCN_MEM_WRITE) {
                    return true;
                } else {
                    /*if(ptr == 0x33b0 ){
                            cout << "Imagebase : "<< (int*)imagebase << "\n";
                            cout << "Section : "<< i << "\n";
                            cout << "Characteristics : "<< (int*)sections[i].characteristics << "\n";
                            cout << "VirtualAddress : " << (int*)sections[i].virtual_address << "\n";
                            cout << "VirtualSize : " << (int*)sections[i+1].virtual_address << "\n";
                            };//*/
                    return false;
                }
            }
        }
        int   n = PEHeader->header.number_of_sections - 1;
        DWORD s = (DWORD) & sections[n];
        s += sizeof(image_section_header) + 1;
        image_section_header * f = (image_section_header *) s;
        if ((ptr >= sections[n].virtual_address) && (ptr < (PEHeader->optional.size_of_image))) {
            if (sections[n].characteristics & IMAGE_SCN_MEM_WRITE) { //
                return true;
            } else {
                return false;
            }
        }
    }
    return false;
}

DWORD VirtualMemory::delete_pointer(DWORD ptr) {
    for (int i = this->vmem_length - 1; i >= 0; i--) {
        if ((ptr >= vmem[i]->vmem) && (ptr <= (vmem[i]->vmem + vmem[i]->size)) && (vmem[i]->size != 0)) {
            vmem[i]->size = 0;
            return 0;
        }
    }
    return -1;
}

DWORD VirtualMemory::get_last_accessed(int index) {
    return last_accessed->getlog(index);
}

DWORD VirtualMemory::get_last_modified(int index) {
    return last_modified->getlog(index);
}

VirtualMemory::~VirtualMemory()
{
	for (int i = 0; i < vmem_length; i++) {
#ifdef WIN32
        if (!(vmem[i]->flags & MEM_VIRTUALPROTECT)) {
            VirtualFree((void *) vmem[i]->rmem, vmem[i]->size, MEM_DECOMMIT);
        }
#else
        free((void *) vmem[i]->rmem);
#endif
    }
}