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
#define __APIS__ 2

//class Thread{};
//APIs Emulation
//-------------
int GetProcAddress_emu(Thread* thread,DWORD* Args);
int GetModuleHandleA_emu(Thread* thread,DWORD* Args);
int LoadLibraryA_emu(Thread* thread,DWORD* Args);
int VirtualAlloc_emu(Thread* thread,DWORD* Args);
int VirtualFree_emu(Thread* thread,DWORD* Args);
int VirtualProtect_emu(Thread* thread,DWORD* Args);
int SetUnhandledExceptionFilter_emu(Thread* thread,DWORD* Args);
