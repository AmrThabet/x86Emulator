#ifndef __OS__
#define __OS__  1
struct FileMapping{
            unsigned long hFile;
            unsigned long hMapping;
            unsigned long BaseAddress;
            unsigned long FileLength;
};
//unsigned long LoadProcess(string);
unsigned long GetTime();
FileMapping* OpenFile(const char*);
FileMapping* OpenFile2(const char*);
FileMapping* CreateNewFile(const char* Filename,unsigned long size);
unsigned long CloseFile(FileMapping*);
#endif