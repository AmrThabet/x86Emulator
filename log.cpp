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
Log::Log(DWORD firstentry) {
    cur      = 0;
    log[cur] = firstentry;
    cur++;
}

void Log::addlog(DWORD entry) {
    log[cur] = entry;
    cur++;
    if (cur == 10) {
        cur = 0;
    }
}

DWORD Log::getlog(int index) {
    if ((index < 0) || (index > 10)) {
        return 0;
    }
    index = cur - index - 1;
    if (index < 0) {
        index += 10;
    }
    return log[index];
}
