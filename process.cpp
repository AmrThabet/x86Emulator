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

// -----------------------------------------------------------------------------------------------------------------------
int Process::ImportTableFixup(DWORD FileHandler)
{
    image_header * PEHeader = (image_header *) (((dos_header *) FileHandler)->e_lfanew + FileHandler);

    if (PEHeader->optional.data_directory[1].virtual_address == 0) {
        return 0;
    }
    image_import_descriptor * Imports = (image_import_descriptor *) (PEHeader->optional.data_directory[1].virtual_address + FileHandler);
	while(1)
	{
        if (Imports->name == 0)
		{
            break;
        }
        DWORD name    = Imports->name + FileHandler;
        DWORD dllbase = sys->GetDllBase((char *) name);
        APIsFixup(FileHandler, Imports, dllbase);
        Imports = (image_import_descriptor *) ((DWORD) Imports + (DWORD) sizeof(image_import_descriptor));
    }
	return 0;
}

// -----------------------------------------------------------------------------------------------------------------------
int Process::APIsFixup(DWORD FileHandler, image_import_descriptor * Imports, DWORD dllbase) {
    image_import_by_name ** names; // pointer to the names that we will get's address
    DWORD                 * pointers;                   // pointer to the the place that we will put the addresses there

    if (Imports->original_first_thunk != 0) {
        names = (image_import_by_name **) Imports->original_first_thunk;
    } else {
        names = (image_import_by_name **) Imports->first_thunk;
    }
    names    = (image_import_by_name **) ((DWORD) names + FileHandler);
    pointers = (DWORD *) (Imports->first_thunk + FileHandler);
    if (Imports->first_thunk == 0) {
        return 0;
    }
    // cout << (int*)FileHandler << "\n";
    for (int i = 0; i < 20000; i++) {
        if (names[i] == 0) {
            break;
        }
        if (!((DWORD) (names[i]->name + FileHandler) & 0x80000000)) {
            DWORD s   = (DWORD) names[i]->name;
            DWORD ptr = sys->GetAPI((char *) (s + FileHandler), dllbase);
            DWORD n   = this->SharedMem->get_virtual_pointer(ptr);
            // if (dllbase)
            // cout << (int*)dllbase << "   " << (int*)ptr << "\n";
            if (n != 0) {
                ptr = n;
            } else {
                n = sys->DLLs[0].imagebase; // equal to the kernel32 base address
            }
            // pointers[i]=ptr;
            memcpy(&pointers[i], &ptr, 4);
        }
    }
	return 0;
}

// -----------------------------------------------------------------------------------------------------------------------
Process::Process (System* sys,char* buff,int size,int Flags)
{
	AppType = Flags;
	if (buff == NULL)return;
	DWORD FileHandler = (DWORD)buff;
	DWORD Entrypoint;
	// 1.Load the buffer 
	if (Flags == PROCESS_UNLOADEDIMAGE)
	{
		FileHandler = PELoader(buff);
	}
	//cout << (int*)FileHandler << "\n";
	// 2.initialize the System
	this->sys = sys;
	
	// 3.initalize the Memory
	this->SharedMem = new VirtualMemory();

	if (Flags == PROCESS_LOADEDIMAGE || Flags == PROCESS_UNLOADEDIMAGE)
	{
		image_header * PEHeader;
		PEHeader  = (image_header *) (((dos_header *) FileHandler)->e_lfanew + FileHandler);
		Imagebase = PEHeader->optional.image_base; // the imagebase 
		this->SharedMem->add_pointer(FileHandler, PEHeader->optional.image_base, PEHeader->optional.size_of_image, MEM_IMAGEBASE);
		Entrypoint = PEHeader->optional.address_of_entry_point + PEHeader->optional.image_base;
		if (PEHeader->header.characteristics & IMAGE_FILE_DLL) {
			IsDLL = true;
		} else {
			IsDLL = false;
		}
	}
	else
	{
		this->SharedMem->add_pointer(FileHandler, FileHandler, size, MEM_READWRITE);
		Entrypoint = FileHandler;
	}
	
	//Load the dlls
    for (int i = 0; i < sys->dll_entries; i++) {
        if (sys->DLLs[i].vAddr != 0) {
            this->SharedMem->add_pointer(sys->DLLs[i].imagebase, sys->DLLs[i].vAddr, sys->DLLs[i].size, MEM_DLLBASE);
        } else {
            this->SharedMem->add_pointer(sys->DLLs[i].imagebase, sys->DLLs[i].imagebase, sys->DLLs[i].size, MEM_DLLBASE);
        }
    }
	
	// 4.fix the import table
    if (Flags == PROCESS_UNLOADEDIMAGE) ImportTableFixup(FileHandler);
    nimports = 0;
	
	// 5.initialize the debugger
    debugger = new AsmDebugger(*this);

    // 6.initialize the PEB
    CreatePEB();

    // 7.initialize the first thread
    nthreads            = 0;
    this->MaxIterations = sys->enVars.MaxIterations;
    CreateThread(Entrypoint);

    ins = (DISASM_INSTRUCTION *) malloc(sizeof(DISASM_INSTRUCTION)+1000); // preparing the buffer for instructions
	
}
Process::Process (System * sys, string filename) {
    // 1.open the file
    DWORD FileHandler = PELoader(filename);

    if (FileHandler == 0) {
        throw (ERROR_FILENAME);
    }
	AppType = PROCESS_FILENAME;

    // 2.initialize the System
    this->sys = sys;
    // 3.initalize the Memory

    this->SharedMem = new VirtualMemory();
    image_header * PEHeader;
    PEHeader  = (image_header *) (((dos_header *) FileHandler)->e_lfanew + FileHandler);
    Imagebase = PEHeader->optional.image_base; // the imagebase
    this->SharedMem->add_pointer(FileHandler, PEHeader->optional.image_base, PEHeader->optional.size_of_image, MEM_IMAGEBASE);
    for (int i = 0; i < sys->dll_entries; i++)
	{
		if (sys->DLLs[i].vAddr != 0 && this->SharedMem->read_virtual_mem(sys->DLLs[i].vAddr) == 0)
		{
            this->SharedMem->add_pointer(sys->DLLs[i].imagebase, sys->DLLs[i].vAddr, sys->DLLs[i].size, MEM_DLLBASE);
        }
		else
		{
            this->SharedMem->add_pointer(sys->DLLs[i].imagebase, SharedMem->create_memory_address(VMEM_TYPE_DLL), sys->DLLs[i].size, MEM_DLLBASE);
        }
    }
    if (PEHeader->header.characteristics & IMAGE_FILE_DLL) {
        IsDLL = true;
    } else {
        IsDLL = false;
    }
    // 4.fix the import table
    ImportTableFixup(FileHandler);
    nimports = 0;
    // 5.initialize the debugger
    debugger = new AsmDebugger(*this);
    // 6.initialize the PEB
    CreatePEB();
    // 7.initialize the first thread
    nthreads            = 0;
    this->MaxIterations = sys->enVars.MaxIterations;
    CreateThread(PEHeader->optional.address_of_entry_point + PEHeader->optional.image_base);
    // threads[0]->stack->push(threads[0]->mem->get_virtual_pointer(sys->APITable[0].addr));             //pushes the pointer to ExitProcess (some viruses get the kernelbase from it)
    ins = (DISASM_INSTRUCTION *) malloc(sizeof(DISASM_INSTRUCTION)+1000); // preparing the buffer for instructions
}

Process::~Process() {
    for (int i = 0; i < SharedMem->vmem_length; i++) {
        if (!(SharedMem->vmem[i]->flags & MEM_VIRTUALPROTECT)) {
			#ifdef WIN32
				VirtualFree((void *) SharedMem->vmem[i]->rmem, SharedMem->vmem[i]->size, MEM_DECOMMIT);
			#else
				free((void *) SharedMem->vmem[i]->rmem);
			#endif
        }
    }
    free(ins);
	delete SharedMem;
	for (int i =0;i < nthreads;i++)
	{
		delete this->threads[i];
	}
	delete debugger;
}

// this proc copy all the shared memory entries on every thread memory to become visible for each thread
// all the threads will see the changes in the shared memory as the shared pointers in all threads point to the same location in memory
System * Process::getsystem() {
    return sys;
}

DISASM_INSTRUCTION * Process::GetLastIns()
{
    return ins;
}

int Process::CreateThread(DWORD ptr) {
    this->threads[nthreads] = new Thread(ptr, *this);
    // for (int i=0;i<SharedMem->vmem_length;i++){
    // this->threads[nthreads]->mem->add_pointer(SharedMem->vmem[i]->rmem,SharedMem->vmem[i]->vmem,SharedMem->vmem[i]->size,SharedMem->vmem[i]->flags);
    // };
    nthreads++;
    return nthreads - 1;
}

// n--> the Thread number
int Process::emulatecommand(int n) {return 0;}

int Process::emulatecommand()
{
    string str;
    int    Error    = 0; // Emulated Successfully
    int    errorSEH = 0;

Continue_th:

    DWORD Max = 1;
    for (int i = 0; i < Max; i++)
	{
        char * ptr = (char *) SharedMem->read_virtual_mem(this->threads[0]->Eip);
        if (ptr == 0)
		{
            Error = EXP_INVALIDPOINTER;
            break;
        }
        ins = sys->disasm(ins, ptr); // ,str
        if ((ins->hde.flags & F_ERROR) && (ins->hde.flags & F_ERROR_OPERAND))
		{
            Error = EXP_INVALID_OPCODE;
            break;
        }
        bool IsApi = sys->IsApiCall(*threads[0], ins);
        bool bp    = this->debugger->TestBp(*threads[0], ins);

        if (bp && !TiggeredBreakpoint)
		{
            TiggeredBreakpoint = true;
            return EXP_BREAKPOINT;
        } else if (TiggeredBreakpoint)
		{
            TiggeredBreakpoint = false;
        }
        if (IsApi)
		{
            if ((threads[0]->Eip & 0xFFFF0000) == 0xBBBB0000) {
                cout << "HERE2!!!!\n\n\n";
            }
            this->threads[0]->log->addlog(this->threads[0]->Eip);
            this->threads[0]->Eip += ins->hde.len;
            sys->CallToAPI(threads[0], ins);
        } 
		else
		{
            this->threads[0]->log->addlog(this->threads[0]->Eip);
            this->threads[0]->Eip += ins->hde.len;
            Error                  = ins->emu_func(*threads[0], ins);
            if (Error != 0) {
                break;
            }
        }
        MaxIterations -= 1;
        if (MaxIterations == 0)
		{
            return EXP_EXCEED_MAX_ITERATIONS;
        }
    }

    if (Error != EXP_INVALID_OPCODE)
	{
        this->threads[0]->Eip -= ins->hde.len; // because it's added before emulating the instruction
    }
    DWORD fsptr = *SharedMem->read_virtual_mem(threads[0]->GetFS());
    if (SharedMem->read_virtual_mem(fsptr) == 0)
	{
        return Error;
    }
    if (*SharedMem->read_virtual_mem(fsptr) != 0xFFFFFFFF)
	{
        DWORD * ptr     = (DWORD *) this->SharedMem->read_virtual_mem(threads[0]->GetFS());
        DWORD   err_ptr = *ptr;
        DWORD * nextptr = (DWORD *) this->SharedMem->read_virtual_mem(err_ptr); // the next handler
        if ((DWORD) nextptr == 0) {
            return Error;
        }
        threads[0]->generateException(Error);
        *ptr     = *nextptr;                                           // save it
        errorSEH = 1;
        goto Continue_th;
    }
	else
	{
        return Error;
    }
}

// ------------------------------------------------------------------------------------------------------
// /*
int Process::emulate()
{
    string str;
    int    Error    = 0; // Emulated Successfully
    int    errorSEH = 0;

Continue_th:

    DWORD Max = MaxIterations;
    for (int i = 0; i < Max; i++)
	{
        char * ptr = (char *) SharedMem->read_virtual_mem(this->threads[0]->Eip);
        if (ptr == 0)
		{
            Error = EXP_INVALIDPOINTER;
            break;
        }
        ins = sys->disasm(ins, ptr); // ,str
        if ((ins->hde.flags & F_ERROR) && (ins->hde.flags & F_ERROR_OPERAND))
		{
            Error = EXP_INVALID_OPCODE;
            break;
        }
        bool IsApi = sys->IsApiCall(*threads[0], ins);
        bool bp    = this->debugger->TestBp(*threads[0], ins);

        if (bp && !TiggeredBreakpoint)
		{
            TiggeredBreakpoint = true;
            return EXP_BREAKPOINT;
        }
		//This is used to not break on the same bp everytime 
		else if (TiggeredBreakpoint)
		{
            TiggeredBreakpoint = false;
        }
        if (IsApi)
		{
            if ((threads[0]->Eip & 0xFFFF0000) == 0xBBBB0000)
			{
                cout << "HERE2!!!!\n\n\n";
            }
            this->threads[0]->log->addlog(this->threads[0]->Eip);
            this->threads[0]->Eip += ins->hde.len;
            sys->CallToAPI(threads[0], ins);
        } 
		else
		{
            this->threads[0]->log->addlog(this->threads[0]->Eip);
            this->threads[0]->Eip += ins->hde.len;
            Error                  = ins->emu_func(*threads[0], ins);
            if (Error != 0)
			{
                break;
            }
        }
        MaxIterations -= 1;
        if (MaxIterations == 0)
		{
            return EXP_EXCEED_MAX_ITERATIONS;
        }
    }

    if (Error != EXP_INVALID_OPCODE)
	{
        this->threads[0]->Eip -= ins->hde.len; // because it's added before emulating the instruction
    }

	//SEH Part
    DWORD fsptr = *SharedMem->read_virtual_mem(threads[0]->GetFS());

    if (SharedMem->read_virtual_mem(fsptr) == 0)
	{
        return Error;
    }
    if (*SharedMem->read_virtual_mem(fsptr) != 0xFFFFFFFF)
	{
        DWORD * ptr     = (DWORD *) this->SharedMem->read_virtual_mem(threads[0]->GetFS());
        DWORD   err_ptr = *ptr;
        DWORD * nextptr = (DWORD *) this->SharedMem->read_virtual_mem(err_ptr); // the next handler
        if ((DWORD) nextptr == 0)
		{
            return Error;
        }
        threads[0]->generateException(Error);
        *ptr     = *nextptr;                                           // save it
        errorSEH = 1;
        goto Continue_th;
    }
	else
	{
        return Error;
    }
}

// */
// ------------------------------------------------------------------------------------------------------
// /*

int Process::emulate(string LogFile)
{
    string str;
    int    Error    = 0; // Emulated Successfully
    int    errorSEH = 0;
    FILE * x2;

    x2 = fopen(LogFile.c_str(), "w");
Continue_th:

    DWORD Max = MaxIterations;
    for (int i = 0; i < Max; i++) {
        char * ptr = (char *) SharedMem->read_virtual_mem(this->threads[0]->Eip);
        if (ptr == 0) {
            Error = EXP_INVALIDPOINTER;
            break;
        }
		DWORD x = (DWORD)ins;
        ins = sys->disasm(ins, ptr, str);
		if (x != (DWORD)ins) cout << "a DISASTER !!!" << "\n";
        if ((ins->hde.flags & F_ERROR) && (ins->hde.flags & F_ERROR_OPERAND)) {
            Error = EXP_INVALID_OPCODE;
            break;
        }
        bool IsApi = sys->IsApiCall(*threads[0], ins);
        bool bp    = this->debugger->TestBp(*threads[0], ins);
        // **************************************
        string str1 = "EAX = ";
        string str3 = "ESP = ";
        string str4;
        char   buff[50];
        string str2[9];
        string strST[9];
        for (int l = 0; l < 8; l++) {
            sprintf(buff, "%X", threads[0]->Exx[l]);
            str2[l] = buff;
        }
            sprintf(buff, "%X", threads[0]->EFlags);
        str2[8] = buff;
            sprintf(buff, "%X", threads[0]->Eip);
        str4 = buff;
        str4.append(" : ");
        str4.append(str);
        str1.append(str2[0].append((string("  ECX = ")).append(str2[1].append((string("  EDX = ")).append(str2[2])).append((string("  EBX = ")).append(str2[3].append("\n"))))));
        str3.append(str2[4].append((string("  EBP = ")).append(str2[5].append((string("  ESI = ")).append(str2[6])).append((string("  EDI = ")).append(str2[7]))))); // .append("\n")

        str3.append(string(" EFLAGS = ").append(str2[8].append("\n")));
        for (int l = 0; l < 8; l++) {
            float st = threads[0]->ST[l];
            sprintf(buff, "%G", st);
            strST[l] = buff;
        }
        str3.append("ST[0] = ").append(strST[0]).append("  ST[1] = ").append(strST[1]).append("\n");
        fprintf(x2, str1.c_str());
        fprintf(x2, str3.c_str());
        fprintf(x2, str4.c_str());
        fprintf(x2, "  \n\n");

        // ***************************************

        if (bp && !TiggeredBreakpoint) {
            TiggeredBreakpoint = true;
            return EXP_BREAKPOINT;
        } else if (TiggeredBreakpoint) {
            TiggeredBreakpoint = false;
        }
        if (IsApi) {
            this->threads[0]->log->addlog(this->threads[0]->Eip);
            this->threads[0]->Eip += ins->hde.len;
            sys->CallToAPI(threads[0], ins);
        } else {
            this->threads[0]->log->addlog(this->threads[0]->Eip);
            this->threads[0]->Eip += ins->hde.len;
            Error                  = ins->emu_func(*threads[0], ins);
            if (Error != 0) {
                break;
            }
        }
        MaxIterations -= 1;
        if (MaxIterations == 0) {
            fclose(x2);
            return EXP_EXCEED_MAX_ITERATIONS;
        }
    }

    if (Error != EXP_INVALID_OPCODE) {
        this->threads[0]->Eip -= ins->hde.len; // because it's added before emulating the instruction
    }
    DWORD fsptr = *SharedMem->read_virtual_mem(threads[0]->GetFS());
    if (SharedMem->read_virtual_mem(fsptr) == 0) {
            fclose(x2);
        return Error;
    }
    if (*SharedMem->read_virtual_mem(fsptr) != 0xFFFFFFFF) {
        DWORD * ptr     = (DWORD *) this->SharedMem->read_virtual_mem(threads[0]->GetFS());
        DWORD   err_ptr = *ptr;
        DWORD * nextptr = (DWORD *) this->SharedMem->read_virtual_mem(err_ptr); // the next handler
        if ((DWORD) nextptr == 0) {
            fclose(x2);
            return Error;
        }
        threads[0]->generateException(Error);
        *ptr     = *nextptr;                                           // save it
        errorSEH = 1;
        goto Continue_th;
    } else {
            fclose(x2);
        return Error;
    }
}

// ------------------------------------------------------------------------------------------------------
Thread * Process::GetThread(int id)
{
    return this->threads[id];
}

DWORD Process::GetImagebase()
{
    return Imagebase;
}

void Process::CreatePEB()
{
    // Important Strings

    wchar_t * skernel32       = L"Kernel32.dll";
    DWORD     skernel32length = 13 * 2;            // the Size of kernel32.dll unicode string + null wide char
    wchar_t * sntdll          = L"ntdll.dll";
    DWORD     sntdlllength    = 10 * 2;            // the Size of ntdll.dll unicode string + null wide char
    wchar_t * sprogram        = L"program.exe";
    DWORD     sprogramlength  = 12 * 2;            // the Size of program.exe unicode string + null wide char

    // creating the PEB
    DWORD size = sizeof(PEB) + 3 * sizeof(_LDR_DATA_TABLE_ENTRY) + sizeof(_PEB_LDR_DATA);

    size += skernel32length + sntdlllength + sprogramlength; // adding the strings
#ifdef WIN32
    DWORD ptr = (DWORD) VirtualAlloc(NULL, size, MEM_COMMIT, PAGE_READWRITE); // the virtual place
#else
    DWORD ptr = (DWORD) malloc(size); // the virtual place
#endif

    // Copying the Strings
    DWORD strs_ptr = ptr + sizeof(PEB) + 3 * sizeof(_LDR_DATA_TABLE_ENTRY) + sizeof(_PEB_LDR_DATA);
    memcpy((void *) strs_ptr, skernel32, 13 * 2);
    skernel32 = (wchar_t *) strs_ptr;
    size     += 13 * 2;
    strs_ptr += 13 * 2;
    memcpy((void *) strs_ptr, sntdll, 10 * 2);
    sntdll    = (wchar_t *) strs_ptr;
    size     += 10 * 2;
    strs_ptr += 10 * 2;
    memcpy((void *) strs_ptr, sprogram, 12 * 2);
    sprogram  = (wchar_t *) strs_ptr;
    size     += 12 * 2;
    strs_ptr += 12 * 2;
    peb       = (PEB *) ptr;

    // Preparing the Data Table Entry
    _PEB_LDR_DATA         * LoaderData = (_PEB_LDR_DATA *) (ptr + sizeof(PEB));
    _LDR_DATA_TABLE_ENTRY * program    = (_LDR_DATA_TABLE_ENTRY *) (ptr + sizeof(PEB) + sizeof(_PEB_LDR_DATA));
    _LDR_DATA_TABLE_ENTRY * ntdll      = (_LDR_DATA_TABLE_ENTRY *) (ptr + sizeof(PEB) + sizeof(_LDR_DATA_TABLE_ENTRY) + sizeof(_PEB_LDR_DATA));
    _LDR_DATA_TABLE_ENTRY * kernel     = (_LDR_DATA_TABLE_ENTRY *) (ptr + sizeof(PEB) + 2 * sizeof(_LDR_DATA_TABLE_ENTRY) + sizeof(_PEB_LDR_DATA));
    memset(peb, 0, sizeof(PEB) + 3 * sizeof(_LDR_DATA_TABLE_ENTRY) + sizeof(_PEB_LDR_DATA));

    // Create it
    this->SharedMem->add_pointer((DWORD) peb, 0x7FFD5000, size);

    // Fill the PEB with the important formation
    peb->ImageBaseAddress = Imagebase; //

    // Filling the pointers
    peb->LoaderData = (_PEB_LDR_DATA *) SharedMem->get_virtual_pointer((DWORD) LoaderData);

    // filling the entries
    program->DllBase           = Imagebase;
    program->FullDllNameLength = sprogramlength;
    program->BaseDllNameLength = sprogramlength;
    program->BaseDllName       = (char *) SharedMem->get_virtual_pointer((DWORD) sprogram);
    program->FullDllName       = (char *) SharedMem->get_virtual_pointer((DWORD) sprogram);

    ntdll->DllBase           = sys->DLLs[1].vAddr;
    ntdll->FullDllNameLength = sntdlllength;
    ntdll->BaseDllNameLength = sntdlllength;
    ntdll->BaseDllName       = (char *) SharedMem->get_virtual_pointer((DWORD) sntdll);
    ntdll->FullDllName       = (char *) SharedMem->get_virtual_pointer((DWORD) sntdll);

    kernel->DllBase           = sys->DLLs[0].vAddr;
    kernel->FullDllNameLength = skernel32length;
    kernel->BaseDllNameLength = skernel32length;
    kernel->BaseDllName       = (char *) SharedMem->get_virtual_pointer((DWORD) skernel32);
    kernel->FullDllName       = (char *) SharedMem->get_virtual_pointer((DWORD) skernel32);

    // LoaderData
    LoaderData->InLoadOrderModuleList.Flink = SharedMem->get_virtual_pointer((DWORD) program);
    LoaderData->InLoadOrderModuleList.Blink = SharedMem->get_virtual_pointer((DWORD) kernel);

    LoaderData->InMemoryOrderModuleList.Flink = SharedMem->get_virtual_pointer((DWORD) program + 0x8); // Here the FLink is directed to the InMemoryOrderModuleList inside Program data
    LoaderData->InMemoryOrderModuleList.Blink = SharedMem->get_virtual_pointer((DWORD) kernel + 0x8);

    LoaderData->InInitializationOrderModuleList.Flink = SharedMem->get_virtual_pointer((DWORD) ntdll + 0x10); // Here the FLink is directed to the InInitializationOrderModuleList inside ntdll data
    LoaderData->InInitializationOrderModuleList.Blink = SharedMem->get_virtual_pointer((DWORD) kernel + 0x10);

    // Program
    program->InLoadOrderLinks.Flink = SharedMem->get_virtual_pointer((DWORD) ntdll);
    // program->InLoadOrderLinks.Blink=SharedMem->get_virtual_pointer((DWORD)ntdll);

    program->InMemoryOrderLinks.Flink = SharedMem->get_virtual_pointer((DWORD) ntdll + 0x8);
    // program->InMemoryOrderLinks.Blink=SharedMem->get_virtual_pointer((DWORD)ntdll);

    program->InInitializationOrderLinks.Flink = 0;
    program->InInitializationOrderLinks.Blink = 0;
    // ntdll
    ntdll->InLoadOrderLinks.Flink = SharedMem->get_virtual_pointer((DWORD) kernel);
    ntdll->InLoadOrderLinks.Blink = SharedMem->get_virtual_pointer((DWORD) program);

    ntdll->InMemoryOrderLinks.Flink = SharedMem->get_virtual_pointer((DWORD) kernel + 0x8);
    ntdll->InMemoryOrderLinks.Blink = SharedMem->get_virtual_pointer((DWORD) program + 0x8);

    ntdll->InInitializationOrderLinks.Flink = SharedMem->get_virtual_pointer((DWORD) kernel + 0x10);
    // ntdll->InInitializationOrderLinks.Blink=SharedMem->get_virtual_pointer((DWORD)kernel);

    // kernel
    // kernel->InLoadOrderLinks.Flink=SharedMem->get_virtual_pointer((DWORD)program);
    kernel->InLoadOrderLinks.Blink = SharedMem->get_virtual_pointer((DWORD) ntdll);

    // kernel->InMemoryOrderLinks.Flink=SharedMem->get_virtual_pointer((DWORD)program);
    kernel->InMemoryOrderLinks.Blink = SharedMem->get_virtual_pointer((DWORD) ntdll + 0x8);

    // kernel->InInitializationOrderLinks.Flink=SharedMem->get_virtual_pointer((DWORD)program);
    kernel->InInitializationOrderLinks.Blink = SharedMem->get_virtual_pointer((DWORD) ntdll + 0x10);
}

void Process::SkipIt() {
    this->threads[0]->Eip += ins->hde.len;
    ins = sys->disasm(ins, (char *) SharedMem->read_virtual_mem(this->threads[0]->Eip)); // ,str
}
