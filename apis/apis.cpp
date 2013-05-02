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

int undefined_api(Thread * thread, DWORD * Args) {
    return 0;
}

int System::define_dll(char * name, char * path, DWORD vAddr)
{
	if (name == NULL || path == NULL || vAddr == NULL)return 0;
    if (GetDllIndex(name))
	{
        return GetDllIndex(name);
    }
    DLLs[dll_entries].name  = name;
    DLLs[dll_entries].vAddr = vAddr;

    string s = "";

    s.append(path);
    s.append(name);
    DLLs[dll_entries].imagebase = PELoader(s);
    // cout << (int*)DLLs[dll_entries].imagebase << "\n";
    if (DLLs[dll_entries].imagebase == 0)
	{
        return 0;
    }
    DWORD          FileHandler = DLLs[dll_entries].imagebase;
    image_header * PEHeader;
    PEHeader               = (image_header *) (((dos_header *) FileHandler)->e_lfanew + FileHandler);
    DLLs[dll_entries].size = PEHeader->optional.size_of_image;
    dll_entries++;
    return dll_entries - 1;
}

int System::define_api(char * name, DLL * lib, DWORD args, int (* emu_func)(Thread *, DWORD *))
{
	if (name == NULL || lib == NULL || emu_func == NULL) return 0;
    for (int i = 0; i < api_entries; i++)
	{
        if (!strcmp(to_lower_case(APITable[i].name).c_str(), to_lower_case(name).c_str()))
		{
            APITable[i].lib      = lib;
            APITable[i].emu_func = emu_func;
            return i;
        }
    }
    APITable[api_entries].name     = name;
    APITable[api_entries].lib      = lib;
    APITable[api_entries].args     = args;
    APITable[api_entries].addr     = GetAPI(APITable[api_entries].name, lib->imagebase);
    APITable[api_entries].emu_func = emu_func;
    api_entries++;
    return api_entries - 1;
}

// ---------------------------------------------------------------------------------------------------
void System::init_apis(char * path)
{
    // define the dlls
    define_dll("kernel32.dll", path, enVars.kernel32);
    define_dll("ntdll.dll",    path, enVars.ntdll);
    define_dll("user32.dll",   path, enVars.user32);

    // the defined apis
    define_api("GetProcAddress",              &DLLs[0], 2,  GetProcAddress_emu);
    define_api("GetModuleHandleA",            &DLLs[0], 1,  GetModuleHandleA_emu);
    define_api("LoadLibraryA",                &DLLs[0], 1,  LoadLibraryA_emu);
    define_api("VirtualAlloc",                &DLLs[0], 4,  VirtualAlloc_emu);
    define_api("VirtualFree",                 &DLLs[0], 3,  VirtualFree_emu);
    define_api("VirtualProtect",              &DLLs[0], 4,  VirtualProtect_emu);
    define_api("SetUnhandledExceptionFilter", &DLLs[0], 1,  SetUnhandledExceptionFilter_emu);

    // undefined apis
	///*
    define_api("ExitProcess",                 &DLLs[0], 1,  undefined_api);
    define_api("MessageBoxA",                 &DLLs[2], 4,  undefined_api);
    define_api("GetCommandLineA",             &DLLs[0], 0,  undefined_api);
    define_api("CreateProcessA",              &DLLs[0], 10, undefined_api);
    define_api("lstrlenA",                    &DLLs[0], 1,  undefined_api);
    define_api("GetTickCount",                &DLLs[0], 0,  undefined_api);
    define_api("GetCurrentProcess",           &DLLs[0], 0,  undefined_api);
    define_api("GetCurrentProcessId",         &DLLs[0], 0,  undefined_api);
    define_api("GetCurrentThread",            &DLLs[0], 0,  undefined_api);
    define_api("GetStartupInfoA",             &DLLs[0], 1,  undefined_api);
    define_api("GetKeyboardType",             &DLLs[2], 1,  undefined_api);
    define_api("GetModuleFileNameA",          &DLLs[0], 2,  undefined_api);
    define_api("ReadFile",                    &DLLs[0], 5,  undefined_api);
    define_api("WriteFile",                   &DLLs[0], 5,  undefined_api);
    define_api("CreateFileA",                 &DLLs[0], 7,  undefined_api);
    define_api("GetFileSize",                 &DLLs[0], 2,  undefined_api);
    define_api("SetFilePointer",              &DLLs[0], 4,  undefined_api);
    define_api("SetEndOfFile",                &DLLs[0], 2,  undefined_api);
    define_api("GetLocaleInfoA",              &DLLs[0], 4,  undefined_api);
    define_api("IsCharUpper",                 &DLLs[0], 1,  undefined_api);
    define_api("GetLastError",                &DLLs[0], 0,  undefined_api);
    define_api("GetKeyState",                 &DLLs[2], 1,  undefined_api);
    define_api("GetFocus",                    &DLLs[2], 0,  undefined_api);
    define_api("GetForegroundWindow",         &DLLs[0], 0,  undefined_api);
    define_api("GetDC",                       &DLLs[0], 1,  undefined_api);
    define_api("GetCursorPos",                &DLLs[0], 1,  undefined_api);
    define_api("GetCursor",                   &DLLs[0], 0,  undefined_api);
    define_api("lstrcmpA",                    &DLLs[0], 2,  undefined_api);
    define_api("lstrcmpiA",                   &DLLs[0], 2,  undefined_api);
    define_api("ZwSetInformationProcess",     &DLLs[1], 4,  undefined_api);
    define_api("ZwQueryInformationProcess",   &DLLs[1], 5,  undefined_api);
	//*/
    // ZwSetInformationProcess

    // */
}

int System::CallToAPI(Thread * thread, DISASM_INSTRUCTION * s)
{
    DWORD retPtr = thread->stack->pop();

    // cout << "\nCalling an API ......\n---------------------\n";
    // if(s->other >0 )cout << APITable[s->other-1].name << "\n";
    if (s->other > 0)
	{
        int n = s->other - 1;
        if (APITable[n].args > 0)
		{
            DWORD * args = (DWORD *) malloc(APITable[n].args * 4);
            memset(args, 0, APITable[n].args * 4);
            for (int i = 0; i < APITable[n].args; i++)
			{
                args[i] = thread->stack->pop();
                // cout << i << "     Argument : " << (int*)args[i] <<"       "<< (int*)thread->Exx[4] << "\n";
            }
            thread->Exx[0] = APITable[n].emu_func(thread, args);
			free(args);
            // cout << (int*)thread->Exx[0] << "\n";
        }
    }
    thread->Eip = retPtr;
	return 0;
}

bool System::IsApiCall(Thread& thread, DISASM_INSTRUCTION * s)
{
	if (s == NULL)return false;

    // first we will search for the pointer to find it's an API or not
    DWORD ptr   = thread.Eip;
    int entry = 0;

    if ((ptr & 0xFFFF0000) == 0xBBBB0000)
	{
        return true;
    }
    for (int i = 0; i < thread.mem->vmem_length; i++)
	{
        if ((ptr >= thread.mem->vmem[i]->vmem) && (ptr <= (thread.mem->vmem[i]->vmem + thread.mem->vmem[i]->size))) {
            ptr  -= thread.mem->vmem[i]->vmem;
            ptr  += thread.mem->vmem[i]->rmem;
            entry = i;
            break;
        }
    }
    // it's an API let's get more information
    if (thread.mem->vmem[entry]->flags == MEM_DLLBASE)
	{
        s->flags |= API_CALL;
        // let's search for the api
        for (int i = 0; i < api_entries; i++)
		{
            if (APITable[i].addr == ptr)
			{
                s->other = i + 1; // because zero mean undefined :)
                break;
            }
        }
        return true;
    }
	else
	{
        return false;
    }
}

unsigned long System::GetAPI(char * func, unsigned long dll)
{
	if (func == NULL || dll ==  NULL)return 0;

    DWORD   hKernelModule;
    DWORD   dwFuncOffset;
    DWORD   dwNameOrdOffset;
    DWORD   dwTemp, dwOffsetPE, dwOffsetExport;
    int     i = 0;
    DWORD   dwNumberOfNames, dwNamesOffset;
    DWORD * dwNameRVAs;
    DWORD * dwFuncRVAs;
    short * dwNameOrdRVAs;
    bool    bApiFound;
    char    lpszApiName[255];

    // cout << func << "\n";
    hKernelModule = dll;
    dwOffsetPE     = *(DWORD *) ((DWORD) hKernelModule + 0x3C);
    dwOffsetExport = *(DWORD *) ((DWORD) hKernelModule + dwOffsetPE + 0x78);

    dwNumberOfNames = *(DWORD *) ((DWORD) hKernelModule + dwOffsetExport + 0x18);

    dwFuncOffset    = *(DWORD *) (hKernelModule + dwOffsetExport + 0x1C);
    dwNamesOffset   = *(DWORD *) ((DWORD) hKernelModule + dwOffsetExport + 0x20);
    dwNameOrdOffset = *(DWORD *) ((DWORD) hKernelModule + dwOffsetExport + 0x24);
    dwNameRVAs      = (DWORD *) (hKernelModule + dwNamesOffset);
    dwFuncRVAs      = (DWORD *) (hKernelModule + dwFuncOffset);
    dwNameOrdRVAs   = (short *) (hKernelModule + dwNameOrdOffset);
    bApiFound       = false;

    for (i = 0; i < dwNumberOfNames; i++)
	{
        if (!strcmp(((DWORD) hKernelModule + (char *) dwNameRVAs[i]), func)) {
            bApiFound = true;
            break;
        }
    }

    if (!bApiFound)
	{
        return 0;
    }
    i = dwNameOrdRVAs[i];
    return dll + dwFuncRVAs[i];
}

char * System::GetAPIbyAddress(unsigned long ptr, unsigned long dll) {
    DWORD   hKernelModule;
    DWORD   dwFuncOffset;
    DWORD   dwNameOrdOffset;
    DWORD   dwTemp, dwOffsetPE, dwOffsetExport;
    int     i = 0;
    int     l = 0;
    DWORD   dwNumberOfNames, dwNamesOffset;
    DWORD * dwNameRVAs;
    DWORD * dwFuncRVAs;
    short * dwNameOrdRVAs;
    bool    bApiFound;
    char    lpszApiName[255];

    hKernelModule = dll;
    if (dll == 0) {
        return 0;
    }
    // cout << (int*)dll << "\n";
    dwOffsetPE     = *(DWORD *) ((DWORD) hKernelModule + 0x3C);
    dwOffsetExport = *(DWORD *) ((DWORD) hKernelModule + dwOffsetPE + 0x78);

    dwNumberOfNames = *(DWORD *) ((DWORD) hKernelModule + dwOffsetExport + 0x18);

    dwFuncOffset    = *(DWORD *) (hKernelModule + dwOffsetExport + 0x1C);
    dwNamesOffset   = *(DWORD *) ((DWORD) hKernelModule + dwOffsetExport + 0x20);
    dwNameOrdOffset = *(DWORD *) ((DWORD) hKernelModule + dwOffsetExport + 0x24);
    dwNameRVAs      = (DWORD *) (hKernelModule + dwNamesOffset);
    dwFuncRVAs      = (DWORD *) (hKernelModule + dwFuncOffset);
    dwNameOrdRVAs   = (short *) (hKernelModule + dwNameOrdOffset);

    bApiFound = false;

    for (i = 0; i < dwNumberOfNames; i++) {
        if ((dwFuncRVAs[i]) == (ptr - dll)) {
            bApiFound = true;
            break;
        }
    }
    if (!bApiFound) {
        return 0;
    }
    for (l = 0; l < dwNumberOfNames; l++) {
        if (i == dwNameOrdRVAs[l]) {
            i = l;
            break;
        }
    }
    return (char *) (dll + dwNameRVAs[i]);
}

char * System::GetTiggeredAPI(Thread &thread)
{
    int   entry = 0;
    DWORD ptr   = thread.Eip;

    for (int i = 0; i < thread.mem->vmem_length; i++)
	{
        if ((ptr >= thread.mem->vmem[i]->vmem) && (ptr <= (thread.mem->vmem[i]->vmem + thread.mem->vmem[i]->size))) {
            ptr  -= thread.mem->vmem[i]->vmem;
            ptr  += thread.mem->vmem[i]->rmem;
            entry = i;
            break;
        }
    }
    DWORD  dllbase = thread.mem->vmem[entry]->rmem;
    char * c       = GetAPIbyAddress(ptr, dllbase);
    return c;
}

// -----------------------------------------------------------------------------------------------------------------------
unsigned long System::GetDllBase(char * s)
{
	if (s == NULL)return 0;
    string str = to_lower_case((char *) s);

    str = str.substr(0, str.size() - 1); // sometimes converted wrong (from char* to string)
    for (int i = 0; i < dll_entries; i++) {
        string name = DLLs[i].name;
        if (!strcmp(str.c_str(), name.c_str())) {
            return DLLs[i].imagebase;
        }
        if (!strcmp(str.c_str(), name.substr(0, str.size()).c_str())) {
            return DLLs[i].imagebase;
        }
    }
    return 0;
}

unsigned long System::GetDllIndex(char * s)
{
	if (s == NULL)return 0;
    string str = to_lower_case((char *) s);

    for (int i = 0; i < dll_entries; i++) {
        string name = DLLs[i].name;
        if (!strcmp(str.c_str(), name.c_str())) {
            return i;
        }
        if (!strcmp(str.c_str(), name.substr(0, str.size()).c_str())) {
            return i;
        }
    }
    return 0;
}
