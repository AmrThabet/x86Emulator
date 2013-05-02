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
#include "../pe.h"
#include <string.h>

#include "os.h"

#ifdef WIN32
#include <iostream>
#include <windows.h>

using namespace std;

// unsigned long LoadProcess(string filename){
// unsigned long n=(unsigned long)LoadLibraryA (filename.c_str());
// return n;
// };
unsigned long GetTime() {
    return (unsigned long) GetTickCount();
}

FileMapping * OpenFile(const char * Filename) {
    HANDLE        hFile;
    HANDLE        hMapping;
    unsigned long BaseAddress;
    DWORD         FileLength;

    hFile = CreateFileA(Filename,
                        GENERIC_READ,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        0);
    if (hFile == INVALID_HANDLE_VALUE) {
        return 0;
    }
    hMapping = CreateFileMappingW(hFile,
                                  NULL,
                                  PAGE_READONLY,
                                  0,
                                  0,
                                  NULL);
    if (hMapping == 0) {
        CloseHandle(hFile);
        return 0;
    }
    BaseAddress = (unsigned long) MapViewOfFile(hMapping,
                                                FILE_MAP_READ,
                                                0,
                                                0,
                                                0);
    if (hMapping == 0) {
        UnmapViewOfFile(hMapping);
        CloseHandle(hFile);
        return 0;
    }
    FileMapping * hMap = (FileMapping *) malloc(sizeof(FileMapping));
    hMap->hFile       = (unsigned long) hFile;
    hMap->hMapping    = (unsigned long) hMapping;
    hMap->BaseAddress = (unsigned long) BaseAddress;
    hMap->FileLength  = (unsigned long) GetFileSize(hFile,
                                                    NULL);
    return hMap;
}

// ------------------------------------------------------------------------------
FileMapping * CreateNewFile(const char * Filename, unsigned long size) {
    HANDLE        hFile;
    HANDLE        hMapping;
    unsigned long BaseAddress;
    DWORD         FileLength;

    hFile = CreateFileA(Filename,
                        GENERIC_READ | GENERIC_WRITE,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL,
                        CREATE_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL,
                        0);
    if (hFile == INVALID_HANDLE_VALUE) {
        return 0;
    }
    hMapping = CreateFileMappingW(hFile,
                                  NULL,
                                  PAGE_READWRITE,
                                  0,
                                  size,
                                  NULL);
    if (hMapping == 0) {
        CloseHandle(hFile);
        return 0;
    }
    BaseAddress = (unsigned long) MapViewOfFile(hMapping,
                                                FILE_MAP_ALL_ACCESS,
                                                0,
                                                0,
                                                0);
    if (hMapping == 0) {
        UnmapViewOfFile(hMapping);
        CloseHandle(hFile);
        return 0;
    }
    FileMapping * hMap = (FileMapping *) malloc(sizeof(FileMapping));
    hMap->hFile       = (unsigned long) hFile;
    hMap->hMapping    = (unsigned long) hMapping;
    hMap->BaseAddress = (unsigned long) BaseAddress;
    hMap->FileLength  = (unsigned long) size;
    return hMap;
}

// ------------------------------------------------------------------------------
unsigned long CloseFile(FileMapping * hMap) {
    UnmapViewOfFile((void *) hMap->BaseAddress);
    CloseHandle((void *) hMap->hMapping);
    CloseHandle((void *) hMap->hFile);
	return 0;
}

#else
#include <cstdlib>
#include <fcntl.h>
#include <fstream.h>
#include <iostream.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

unsigned long GetTime() {
    return 0;
}

FileMapping * OpenFile(const char * Filename) {
    char          c;
    int           file_in;
    FileMapping * s = (FileMapping *) malloc(sizeof(FileMapping));

    memset(s, 0, sizeof(FileMapping));
    file_in = open(Filename, O_RDONLY);
    // Getting the size
    int count = 0;

    while (1) {
        int n = read(file_in, &c, 1);
        if (n == 0) {
            break;
        }
        count++;
    }
    s->FileLength = count;
    char * buffer = (char *) malloc(count);
    s->BaseAddress = (unsigned long) buffer;
    s->hMapping    = 0; // File was opened for read
    close(file_in);
    file_in = open(Filename, O_RDONLY);
    read(file_in, buffer, count);
    close(file_in);
    return s;
}

FileMapping * CreateNewFile(const char * Filename, unsigned long size) {
    int           file_out;
    FileMapping * s       = (FileMapping *) malloc(sizeof(FileMapping));
    char          del[90] = "rm -rf ";
    int           delsize = strlen(del);

    strcpy(&del[delsize], (char *) Filename);
    system(del);
    file_out       = open(Filename, O_RDWR | O_CREAT);
    s->FileLength  = size;
    s->BaseAddress = (unsigned long) 0;
    s->hMapping    = 1;             // File was opened for write
    s->hFile       = (unsigned long) file_out;
    return s;
}

unsigned long CloseFile(FileMapping * hMap) {
    if (hMap->hMapping == 1) { // file opened for write
        int file_out = (int) hMap->hFile;
        int n        = write(file_out, (char *) hMap->BaseAddress, hMap->FileLength);
        // cout << (int*)n << "          "<< (int*)hMap->FileLength << "\n";
        close(file_out);
    }
}

#endif
