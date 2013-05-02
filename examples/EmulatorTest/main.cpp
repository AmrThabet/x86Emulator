#include "../../x86emu.h"

#include <string.h>
#include <windows.h>
//#include <iostream.h>

using namespace std;

unsigned long GetTime(){
    return GetTickCount();     
};
static unsigned char test[64] = {
0x8B, 0xCB, 0x8B, 0xC3, 0xC1, 0xF9, 0x05, 0x83,
0xE0, 0x1F, 0x8B, 0x0C, 0x8D, 0x40, 0xC2, 0x43,
0x00, 0x8D, 0x04, 0xC0, 0x8D, 0x04, 0x81, 0xEB,
0x05, 0xB8, 0x30, 0x6B, 0x42, 0x00, 0xF6, 0x40,
0x04, 0x20, 0x74, 0x0D, 0x6A, 0x02, 0x6A, 0x00,
0x53, 0xE8, 0x48, 0x52, 0x00, 0x00, 0x83, 0xC4,
0x0C, 0x8B, 0x46, 0x08, 0x8A, 0x4D, 0x08, 0x88,
0x08, 0xEB, 0x14, 0x6A, 0x01, 0x8D, 0x45, 0x08};


int main(int argc, char *argv[])
{
	EnviromentVariables* vars;
    System* sys;
    Process* process;
	DISASM_INSTRUCTION ins;
	vars = (EnviromentVariables*)malloc(sizeof(EnviromentVariables)); 
     memset( vars,0,sizeof(EnviromentVariables));
     
     vars->dllspath="C:\\Windows\\System32\\"; 
     vars->kernel32=(DWORD)GetModuleHandleA("kernel32.dll");
     vars->user32=(DWORD)LoadLibraryA("user32.dll");
     vars->MaxIterations=100000000;                   //100 Million Instructions per encryption layer
     
     sys=new System(vars);
	 string str;
	 bytes * s ;
	 for (int i = 0;i < 64; i+= s->length)
	 {
		 sys->disasm(&ins,(char*)&test[i],str);
		 cout << str.c_str() << "\n";
		 s = sys->assembl(&ins);
		 cout << hex << (int*)s->s[0] << "  " << (int*)s->s[1] << "  " << (int*)s->s[2] << "  " << (int*)s->s[3] << "  " << (int*)s->s[4] << "  " << (int*)s->s[5] << "\n";
		 s = sys->assembl(str);
		 cout << hex << (int*)s->s[0] << "  " << (int*)s->s[1] << "  " << (int*)s->s[2] << "  " << (int*)s->s[3] << "  " << (int*)s->s[4] << "  " << (int*)s->s[5] << "\n";
	 }
}
/*
  Usage : 01.exe Xorer_sample.exe
*/
unsigned char XorerSignature[96] = {
                                0x64, 0xA1, 0x00, 0x00, 0x00, 0x00, 0x50, 0x64,
                                0x89, 0x25, 0x00, 0x00, 0x00, 0x00, 0x83, 0xEC,
                                0x58, 0x53, 0x56, 0x57, 0x89, 0x65, 0xE8
                 };
int main2(int argc, char *argv[])
{  
     //The Main Variables
     ///*
     int YourNumber = 0x000001EB;
     for (short i=0x3030;i<0x7A7A;i++){
         for (short l=0x3030;l<0x7A7A;l++){
         unsigned char* n = (unsigned char*)&i;
         unsigned char* m = (unsigned char*)&l;
             if (((i * l)& 0xFFFF)==YourNumber){
                    //cout << (int*)i << "       " << (int*)l<< "\n";
                 for(int s=0;s<2;s++){
                       if (!(((n[s] > 0x30 && n[s] < 0x39) || \
                              (n[s] > 0x41 && n[s] < 0x5A) || \
                              (n[s] > 0x61 && n[s] < 0x7A)) && \
                             ((m[s] > 0x30 && m[s] < 0x39) || \
                              (m[s] > 0x41 && m[s] < 0x5A) || \
                              (m[s] > 0x61 && m[s] < 0x7A)))) 
                            goto Not_Yet;
                 }
                 cout << (int*)i << "       " << (int*)l << "        " << (int*)((l*i) & 0xFFFF)<< "\n";
             }
             
Not_Yet:
        continue;              
         }  
     };
     //return EXIT_SUCCESS;
     //*/
     EnviromentVariables* vars;
     System* sys;
     Process* process;
     DWORD FileHandler;
     image_header* PEHeader;
     DWORD Imagesize;
     DWORD TimeElapsed;
     image_section_header* sections;
     char* PhysicalEip;
     int nSections;                              //Number of sections
     
     if(argc == 1)
     {
           cout << "Usage : 01.exe Xorer_sample.exe\n";
           return EXIT_SUCCESS;
     }
     
     vars = (EnviromentVariables*)malloc(sizeof(EnviromentVariables)); 
     memset( vars,0,sizeof(EnviromentVariables));
     
     vars->dllspath="C:\\Windows\\System32\\"; 
     vars->kernel32=(DWORD)GetModuleHandleA("kernel32.dll");
     vars->user32=(DWORD)LoadLibraryA("user32.dll");
     vars->MaxIterations=100000000;                   //100 Million Instructions per encryption layer
     
     sys=new System(vars);
//----------------------------------------------------------------------------------------
// Creating The Process and Adding the Breakpoints
     
      try{ 
         process=new Process(sys,string(argv[1]));
     }catch(int x){
            cout << "Error : File name not found\n";
            return EXIT_SUCCESS;
     };
     
     try{
         process->debugger->AddBp("__isdirty(eip)");//__isdirty(eip)
         //process->debugger->AddBp("__lastmodified(0) >= 0x10001000");
     }catch(...){
		 cout << process->debugger->GetLastError().c_str() << "\n";  
           return EXIT_SUCCESS;
     }
     
//----------------------------------------------------------------------------------------
//Emulating
    //PROCESS_DATA_ENTRY s;
   cout << "\nDecrypting The File ... \n\n";        
   TimeElapsed =GetTime();                             //The Emulation Time
   int x=process->emulate();//"test.txt"
   cout << (int*)x << "\n";
   ReconstructImportTable(process);
   PEDump(process->GetThread(0)->Eip,process,"test.exe"); //Dumping The File
    
 //----------------------------------------------------------------------------------------
 //Scan The Infected File
 
    TimeElapsed=GetTime()-TimeElapsed;                 //The Emulation Time 
    cout << "Scanning The File ...\n\n";
     
    //Getting The PE Header Information
    
    FileHandler =(DWORD)process->SharedMem->read_virtual_mem(process->GetImagebase());
    PEHeader = (image_header*)(((dos_header*)FileHandler)->e_lfanew + FileHandler);
    Imagesize = PEHeader->optional.size_of_image;
    
    //Scanning The Memory
    
    for (char* ptr = (char*)FileHandler; ptr <(char*)(FileHandler+Imagesize);ptr++){
      for (int i = 0; i < 16; i++){
          if((ptr[i]& 0xff) != (XorerSignature[i]& 0xff))  goto NextElement;   // not equal to the signature ... continue searching
      };      
      //Now the signature is equal ... and the virus detected
      
      cout << "This File is Infected with Win32/Xorer\n\n";  
      cout << "The Operation ended successfully at "<<"Time (sec) = " << TimeElapsed <<" msecs\n\n";
      delete process;
      delete sys;
      sys=new System(vars);
      cout << "YES\n";
      return EXIT_SUCCESS;
NextElement:;             
    };
    
    //it's not infected
    
    cout << "This File is Not Infected\n";
    return EXIT_SUCCESS;

} 



int main3(int argc, char *argv[])
{  
      //First we will create the Environment Variables 
     //the Environment Variables is just some parameters or setting will be passed to the System 
     EnviromentVariables* vars= (EnviromentVariables*)malloc(sizeof(EnviromentVariables)); 
     memset( vars,0,sizeof(EnviromentVariables));
     //this variable should be adjusted to make the system perform well. this path is the path to the folder that contain 
     //the important dlls which are ("kernel32.dll","ntdll.dll","user32.dll")
     vars->dllspath="C:\\Windows\\System32\\"; 
     //here we will set the Imagebase of the kernel32.dll and user32.dll
     vars->kernel32=(DWORD)GetModuleHandleA("kernel32.dll");
     vars->user32=(DWORD)LoadLibraryA("user32.dll");
     //there's other variables but we can ignore the right now 
    //now we will create the new system 
     System* sys=new System(vars);
     //sys->define_dll("gdi32.dll",vars->dllspath,0x75DE0000);
	 cout << "Running\n";
		 //now We Will create a new Process
     Process* c; 
      try{ 
		  c=new Process(sys,"C:\\upx01.exe");      //process take two parameters system & the program filename
     }catch(int x){ 
            cout << "Error : File name not found\n";
     }; 
     cout << "Running\n";
     cout << (int*)c->GetThread(0)->Eip << "\n";
     //Adding new breakpoint is an easy task
     try{ 
		c->debugger->AddBp("__isdirty(eip)");
     }catch(int x){  
                cout << x << "\n";   
				cout << c->debugger->GetLastError().c_str()<< "\n";           
     };   
     //there's two commands to emulate c->emulate() & emulatecommand(int) 
	 cout << "Running\n";
     int x=c->emulate(string("test.txt"));//
     if (x!=EXP_BREAKPOINT){		//there are other exceptions like invalid pointer and so on return an error for them
        cout << "Error = " << x << "\n";
		cout << (int*)c->GetThread(0)->Eip << "\n";
     }else{
     //Dump the PE file here 
	 cout << "The File Emulated Successfully\n";
     ReconstructImportTable(c); 
     PEDump(c->GetThread(0)->Eip,c,"test.exe");
     } 
     c->SharedMem->write_virtual_mem(0x4033b0,4,(unsigned char*)&c);
     system("PAUSE");
	 ExitProcess(0);
     return EXIT_SUCCESS;
} 