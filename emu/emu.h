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
 
struct DISASM_INSTRUCTION; 

//all emulation functions


int undefined_opcode(Thread&,DISASM_INSTRUCTION*);
//math
int op_add(Thread&,DISASM_INSTRUCTION*);
int op_or (Thread&,DISASM_INSTRUCTION*);
int op_adc(Thread&,DISASM_INSTRUCTION*);
int op_sbb(Thread&,DISASM_INSTRUCTION*);
int op_and(Thread&,DISASM_INSTRUCTION*);
int op_sub(Thread&,DISASM_INSTRUCTION*);
int op_xor(Thread&,DISASM_INSTRUCTION*);
int op_cmp(Thread&,DISASM_INSTRUCTION*);
int op_test(Thread&,DISASM_INSTRUCTION*);
int op_mov(Thread&,DISASM_INSTRUCTION*);
int op_lea(Thread&,DISASM_INSTRUCTION*);
int op_movzx(Thread&,DISASM_INSTRUCTION*);
int op_movsx(Thread&,DISASM_INSTRUCTION*);
//----------------
int op_inc(Thread&,DISASM_INSTRUCTION*);
int op_dec(Thread&,DISASM_INSTRUCTION*);
int op_not(Thread&,DISASM_INSTRUCTION*);
int op_neg(Thread&,DISASM_INSTRUCTION*);
//----------------
int op_xchg(Thread&,DISASM_INSTRUCTION*);
int op_bswap(Thread&,DISASM_INSTRUCTION*);
int op_xadd(Thread&,DISASM_INSTRUCTION*);
int op_mul(Thread&,DISASM_INSTRUCTION*);
int op_imul1(Thread&,DISASM_INSTRUCTION*);
int op_imul2(Thread&,DISASM_INSTRUCTION*);
int op_imul3(Thread&,DISASM_INSTRUCTION*);
int op_div(Thread&,DISASM_INSTRUCTION*);
int op_idiv(Thread&,DISASM_INSTRUCTION*);
int op_cdq(Thread&,DISASM_INSTRUCTION*);
//----------------
//stack
int op_push(Thread&,DISASM_INSTRUCTION*);
int op_pop (Thread&,DISASM_INSTRUCTION*);
int op_pushad(Thread&,DISASM_INSTRUCTION*);
int op_popad (Thread&,DISASM_INSTRUCTION*);
int op_pushfd(Thread&,DISASM_INSTRUCTION*);
int op_popfd (Thread&,DISASM_INSTRUCTION*);
int op_leave(Thread&,DISASM_INSTRUCTION*);
int op_enter(Thread&,DISASM_INSTRUCTION*);
//------------------
//jumps
int op_jcc (Thread&,DISASM_INSTRUCTION*);
int op_setcc (Thread&,DISASM_INSTRUCTION*);
int op_call(Thread&,DISASM_INSTRUCTION*);
int op_ret (Thread&,DISASM_INSTRUCTION*);
//--------------------
//strings
int op_lods(Thread&,DISASM_INSTRUCTION*);
int op_stos(Thread&,DISASM_INSTRUCTION*);
int op_movs(Thread&,DISASM_INSTRUCTION*);
int op_cmps(Thread&,DISASM_INSTRUCTION*);
int op_scas(Thread&,DISASM_INSTRUCTION*);
//--------------------
//binary
int op_shl(Thread&,DISASM_INSTRUCTION*);
int op_shr(Thread&,DISASM_INSTRUCTION*);
int op_rol(Thread&,DISASM_INSTRUCTION*);
int op_ror(Thread&,DISASM_INSTRUCTION*);
int op_sar(Thread&,DISASM_INSTRUCTION*);
int op_rcl(Thread&,DISASM_INSTRUCTION*);
int op_rcr(Thread&,DISASM_INSTRUCTION*);
//---------------------
//flags
int op_stc(Thread&,DISASM_INSTRUCTION*);
int op_clc(Thread&,DISASM_INSTRUCTION*);
//---------------------
//FPU
int op_faddp(Thread& thread,DISASM_INSTRUCTION* s);
int op_fild(Thread& thread,DISASM_INSTRUCTION* s);
int op_fistp(Thread& thread,DISASM_INSTRUCTION* s);
int op_fimul(Thread& thread,DISASM_INSTRUCTION* s);
int op_fnstenv(Thread &thread, DISASM_INSTRUCTION * s);