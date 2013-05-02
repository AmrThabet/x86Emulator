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
DWORD processToContext(Thread* thread) {
   CONTEXT* ctx=(CONTEXT*)malloc(sizeof(CONTEXT));
   byte* ptr = (byte*) ctx;
   DWORD addr, i;
   DWORD ctx_size = (sizeof(CONTEXT) + 3) & ~3;  //round up to next DWORD
   memset(ctx, 0, sizeof(ctx));  ;
   
   ctx->Eax = thread->Exx[0];
   ctx->Ecx = thread->Exx[1];
   ctx->Edx = thread->Exx[2];
   ctx->Ebx = thread->Exx[3];
   ctx->Esp = thread->Exx[4];
   ctx->Ebp = thread->Exx[5];
   ctx->Esi = thread->Exx[6];
   ctx->Edi = thread->Exx[7];
   ctx->Eip = thread->Eip;  //use address at which exception occurred
   ctx->EFlags = thread->EFlags;
   ctx->SegFs = thread->GetFS();
   addr = thread->Exx[4] -= ctx_size;
   for (i = 0; i < sizeof(CONTEXT); i++) {
      API_WRITE_MEM(addr++,1,(unsigned char*)ptr++);
   }
   free(ctx);
   return thread->Exx[4];  
};

//Copy from CONTEXT structure into CPU state for Windows Exception Handling
//Note that the global ctx struct is the only place that Debug and Floating
//point registers are currently defined
int contextToCpu(Thread* thread) {
   CONTEXT* ctx=(CONTEXT*)malloc(sizeof(CONTEXT));
   byte *ptr = (byte*) ctx;
   DWORD addr, i;
   DWORD ctx_size = (sizeof(CONTEXT) + 3) & ~3;  //round up to next DWORD
   addr = thread->Exx[4];
   char* s;
   DWORD* readptr = NULL;
   API_READ_MEM(readptr,addr);
   s=(char*)readptr;
   for (i = 0; i < sizeof(CONTEXT); i++) {
      *ptr++ = *s++;
   }
   thread->Exx[4] += ctx_size;
   thread->Exx[0] = ctx->Eax;
   thread->Exx[1] = ctx->Ecx;
   thread->Exx[2] = ctx->Edx;
   thread->Exx[3] = ctx->Ebx;
   thread->Exx[4] = ctx->Esp;
   thread->Exx[5] = ctx->Ebp;
   thread->Exx[6] = ctx->Esi;
   thread->Exx[7] = ctx->Edi;
   thread->Eip = ctx->Eip;  //use address at which exception occurred
   thread->EFlags = ctx->EFlags;
   free(ctx);
};
int popExceptionRecord(Thread* thread,EXCEPTION_RECORD* rec) {
   byte *ptr = (byte*) rec;
   DWORD addr, i;
   DWORD rec_size = (sizeof(EMU_EXCEPTION_RECORD) + 3) & ~3;  //round up to next DWORD
   addr = thread->Exx[4];
   char* s;
   DWORD* readptr;
   API_READ_MEM(readptr,addr);
   s=(char*)readptr;
   for (i = 0; i < sizeof(EMU_EXCEPTION_RECORD); i++) {
      *ptr++ = (byte) *s++;
   }
   thread->Exx[4] += rec_size;
};

DWORD pushExceptionRecord(Thread* thread,EMU_EXCEPTION_RECORD *rec) {
   byte *ptr = (byte*) rec;
   DWORD addr, i;
   DWORD rec_size = (sizeof(EMU_EXCEPTION_RECORD) + 3) & ~3;  //round up to next DWORD
   addr = thread->Exx[4] -= rec_size;
   for (i = 0; i < sizeof(EMU_EXCEPTION_RECORD); i++) {
      API_WRITE_MEM( addr++,1,(unsigned char*)ptr++);
   }
   return thread->Exx[4];
};

int Thread::doException(DWORD record) {
     EMU_EXCEPTION_RECORD* rec=(EMU_EXCEPTION_RECORD*)record;
   DWORD* ptr;
   SEH_READ_MEM(ptr,this->GetFS());
   DWORD err_ptr = *ptr;
   SEH_READ_MEM(ptr,err_ptr+4);
   DWORD handler = *ptr;  //err->handler
   DWORD ctx_ptr = processToContext(this);
   DWORD rec_ptr = pushExceptionRecord(this,rec);
   stack->push(ctx_ptr);
   stack->push(err_ptr);       //err_ptr == fsBase??
   stack->push(rec_ptr);
   stack->push(SEH_MAGIC);             //handler return address
//need to execute exception handler here setup flag to trap ret
//set eip to start of exception handler and resume fetching
   this->Eip = handler;
   log->addlog(Eip);
}
 
void Thread::sehReturn() {
   EXCEPTION_RECORD rec;
   //need to check eax here to see if exception was handled
   //or if it needs to be kicked up to next SEH handler
   
   this->Exx[4] += 3 * 4;  //clear off exception pointers
   
   popExceptionRecord(this,&rec);

   contextToCpu(this);
   log->addlog(Eip);
   //eip is now restored to pre exception location
   
   //need to fake an iret here
   //doInterruptReturn();  //this clobbers EIP, CS, EFLAGS
   //so restore them here from ctx values
}
void Thread::generateException(DWORD code) {
   if (seh_enable) {
      EMU_EXCEPTION_RECORD rec;
      rec.exceptionCode = code;
      rec.exceptionFlags = CONTINUABLE;   //nothing sophisticated here
      rec.exceptionRecord = 0;   //NULL
      rec.exceptionAddress = Eip;
      rec.numberParameters = 0;
      doException((DWORD)&rec);
   };
};
