#include <iostream>
#include <string>
#include <Windows.h>
#include <TlHelp32.h>

#include "os_util.hpp"

// A DWORD is enough storage to contain any process id. The process_id is passed by reference 
// because any value of a DWORD can represent a process id. Therefore, there would be no way
// to differentiate between success and failure if returned from the function.
int find_process_id(DWORD &process_id, const std::string &process_name)
{
    PROCESSENTRY32 process_info;
    process_info.dwSize = sizeof(process_info);

    HANDLE processes_snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
    
    if(processes_snapshot == INVALID_HANDLE_VALUE) 
        return 0;

    Process32First(processes_snapshot, &process_info);
    if(!process_name.compare(process_info.szExeFile))
    {
        CloseHandle(processes_snapshot);
        process_id = process_info.th32ProcessID;
        return 1;
    }

    while(Process32Next(processes_snapshot, &process_info))
    {
        if(!process_name.compare(process_info.szExeFile))
        {
            CloseHandle(processes_snapshot);
            process_id = process_info.th32ProcessID;
            return 1;
        }
    }
    CloseHandle(processes_snapshot);
    return 0;
}


// 0x0 cannot be used as an address in a windows program. Therefore, returning an address value of 0
// indicates failure.
uintptr_t get_mdle_begin_addr(DWORD process_id, const std::string &module_name) 
{ 
    HANDLE module_handle; 
    MODULEENTRY32 me32; 
 
    module_handle = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, process_id); 
    
    if(module_handle == INVALID_HANDLE_VALUE ) 
        return 0;

    me32.dwSize = sizeof(MODULEENTRY32); 
 
    if(!Module32First(module_handle, &me32)) 
    { 
        CloseHandle(module_handle);     
        return 0; 
    } 
 
    do 
    {
        if(!module_name.compare(me32.szModule))
        {    
            CloseHandle(module_handle); 
            return (uintptr_t) me32.modBaseAddr;    
        }
    } 
    while(Module32Next(module_handle, &me32)); 
    
    CloseHandle(module_handle);
    return 0;
}


int inject_dll(DWORD process_id, std::string path)
{  
    const char *c_path = path.c_str();
    int len = strlen(c_path);
    
    
    HANDLE ph; // process handle
    HANDLE rt; // remote thread
    LPVOID rb; // remote buffer

    // handle to kernel32 and pass it to GetProcAddress
    HMODULE hKernel32 = GetModuleHandle("Kernel32");
    LPVOID lb = GetProcAddress(hKernel32, "LoadLibraryA");
  
    ph = OpenProcess(PROCESS_ALL_ACCESS, FALSE, process_id);
    
    //Allocate memory buffer for remote process.
    rb = VirtualAllocEx(ph, NULL, len, (MEM_RESERVE | MEM_COMMIT), PAGE_EXECUTE_READWRITE);

    //Copy dll between processes.
    WriteProcessMemory(ph, rb, c_path, len, NULL);

    //Create new thread in process.
    rt = CreateRemoteThread(ph, NULL, 0, (LPTHREAD_START_ROUTINE)lb, rb, 0, NULL);
    CloseHandle(ph);
    return 1;
}


// This function gets the memory address at the end of a cheat engine pointer chain. It does not
// get the underlying memory value. mem_read_buffer must be used to read the data at the address.

// The largest uintptr_t can also contain largest memory address in a program. Therefore,
// the number of offsets in a chain can also be represented by a uintptr_t.
// Also, the largest uintptr_t can represent the largest amount of data in an architecture.

// h_process is an optional argument that can be set to null indicating that dll_injection was used
// and a handle is not necessary.
int mem_chain_addr_resolve(uintptr_t *addr_buffer, uintptr_t begin_addr, uintptr_t *offset_chain, 
                           uintptr_t num_offsets, HANDLE h_process)
{
    uintptr_t new_addr, offset_addr;
    
    if(num_offsets == 0)
        return 0;
    
    new_addr = begin_addr;
    for(int i = 0; i < num_offsets; i++)
    {
        offset_addr = new_addr + offset_chain[i];
        
        // Returns if there is an overflow.
        if(offset_addr < new_addr)
            return 0;
        
        if(h_process == NULL)
        {
            if(mem_read_buffer(&new_addr, offset_addr, sizeof(uintptr_t), NULL) == 0)
                return 0;
        }
        else 
        {            
            if(mem_read_buffer(&new_addr, offset_addr, sizeof(uintptr_t), h_process) == 0)
                return 0;
        }
    }
    (*addr_buffer) = offset_addr;
    return 1;    
}


// num_bytes is a uintptr_t because uintptr_t can represent the largest memory address. This is
// also the largest amount of data that can be read.

// h_process is an optional parameter that is used if the cheat is running outside of the process
// space (i.e. "external cheat"). HANDLE is a windows pointer type. If h_process is NULL,
// data will be read from the process space in which mem_read_buffer is run (injected cheats like
// dll injection).
int mem_read_buffer(void *target_buffer, uintptr_t addr, uintptr_t num_bytes, HANDLE h_process)
{
    HANDLE cur_process = h_process;
    
    if(h_process == NULL)
        cur_process = GetCurrentProcess();
    
    if(ReadProcessMemory(cur_process, (LPCVOID) addr, target_buffer, num_bytes, 
       NULL) == false)
        return 0;
    return 1;    
}


int mem_write_buffer(void *source_buffer, uintptr_t addr, uintptr_t num_bytes, HANDLE h_process)
{
    HANDLE cur_process = h_process;
    
    if(h_process == NULL)
        cur_process = GetCurrentProcess();
    
    if(WriteProcessMemory(cur_process, (LPVOID) addr, source_buffer, num_bytes, 
       NULL) == false)
        return 0;
    return 1;    
}