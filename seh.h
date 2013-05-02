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


#define SEH_MAGIC 0xBBBBBBBB

#define SIZEOF_387_REGS      80
#define MAXIMUM_EXTENSION    512

//Some exception codes

//Read or write memory violation
#define MEM_ACCESS 0xC0000005   

//Divide by zero
#define DIV_ZERO_EXCEPTION 0xC0000094   

//Divide overflow
#define DIV_OFLOW 0xC0000095   

//The stack went beyond the maximum available size
#define STACK_OVERFLOW 0xC00000FD   

//Violation of a guard page in memory set up using Virtual Alloc
#define GUARD_ERROR 0x80000001   


#define CONTINUABLE 0
#define NON_CONTINUABLE 1
#define STACK_UNWINDING 2

#ifndef WIN32
struct FLOATING_SAVE_AREA {
   DWORD   ControlWord;
   DWORD   StatusWord;
   DWORD   TagWord;
   DWORD   ErrorOffset;
   DWORD   ErrorSelector;
   DWORD   DataOffset;
   DWORD   DataSelector;
   byte    RegisterArea[SIZEOF_387_REGS];
   DWORD   Cr0NpxState;
};


struct CONTEXT {

   DWORD ContextFlags;

   DWORD   Dr0;
   DWORD   Dr1;
   DWORD   Dr2;
   DWORD   Dr3;
   DWORD   Dr6;
   DWORD   Dr7;

   FLOATING_SAVE_AREA FloatSave;

   DWORD   SegGs;
   DWORD   SegFs;
   DWORD   SegEs;
   DWORD   SegDs;

   DWORD   Edi;   //0x9C
   DWORD   Esi;   //0xA0
   DWORD   Ebx;   //0xA4
   DWORD   Edx;   //0xA8
   DWORD   Ecx;   //0xAC
   DWORD   Eax;   //0xB0
   DWORD   Ebp;   //0xB4
   DWORD   Eip;   //0xB8
   DWORD   SegCs;
   DWORD   EFlags;
   DWORD   Esp;
   DWORD   SegSs;
   byte   ExtendedRegisters[MAXIMUM_EXTENSION];

};


#else
#include <windows.h>
#endif

#define MAXIMUM_PARMS 15

struct EMU_EXCEPTION_RECORD {
   DWORD exceptionCode;
   DWORD exceptionFlags;
   DWORD exceptionRecord;  //struct _EXCEPTION_RECORD *ExceptionRecord
   DWORD exceptionAddress;
   DWORD numberParameters;
   DWORD exceptionInformation[MAXIMUM_PARMS];
};

struct EMU_EXCEPTION_POINTERS {
   EMU_EXCEPTION_RECORD *exceptionRecord;
   CONTEXT *contextRecord;
};

struct ERR {
   DWORD nextErr;  //struct _ERR *nextErr;
   DWORD handler;  //pointer to handler
};   
