#include <iostream>
#include <string>
#include <Windows.h>
#include <TlHelp32.h>

#include "os_util.hpp"

DWORD find_process_id(const std::string &process_name)
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
        return process_info.th32ProcessID;
    }

    while(Process32Next(processes_snapshot, &process_info))
    {
        if(!process_name.compare(process_info.szExeFile))
        {
            CloseHandle(processes_snapshot);
            return process_info.th32ProcessID;
        }
    }

    CloseHandle(processes_snapshot);
    return 0;
}


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



void mem_read_buffer(uintptr_t begin_addr, void *target_buffer, DWORD num_bytes, HANDLE &h_process)
{
    ReadProcessMemory(h_process, (void *) begin_addr, target_buffer, num_bytes, NULL);
    return;    
}



int mem_read_chain(void *target_buffer, uintptr_t begin_addr, uintptr_t *offset_chain, 
                   DWORD num_offsets, DWORD data_size)
{
    uintptr_t new_addr, offset_addr;
    size_t read_data;
    
    if(num_offsets == 0)
        return 0;
    
    new_addr = begin_addr;
    for(int i = 0; i < num_offsets; i++)
    {
        uintptr_t offset_addr = new_addr + offset_chain[i];
        
        // Returns if there is an overflow.
        if(offset_addr < new_addr)
            return 0;
        if(ReadProcessMemory(GetCurrentProcess(), (LPCVOID) offset_addr, &read_data, 
           sizeof(size_t *), NULL) == false)
            return 0;
        new_addr = (uintptr_t) read_data;
    }
    memcpy(target_buffer, &read_data, data_size);
    return 1;    
}



int mem_read_chain_hndl(void *target_buffer, uintptr_t begin_addr, uintptr_t *offset_chain, 
                        DWORD num_offsets,  DWORD data_size,  HANDLE &h_process)
{
    uintptr_t new_addr, offset_addr;
    size_t read_data;
    
    if(num_offsets == 0)
        return 0;
    
    new_addr = begin_addr;
    for(int i = 0; i < num_offsets; i++)
    {
        uintptr_t offset_addr = new_addr + offset_chain[i];
        
        // Returns if there is an overflow.
        if(offset_addr < new_addr)
            return 0;
    
        if(ReadProcessMemory(h_process, (LPCVOID) offset_addr, &read_data, sizeof(size_t *), NULL)
           == false)
            return 0;
        new_addr = (uintptr_t) read_data;
    }

    memcpy(target_buffer, &read_data, data_size);
    return 1;    
}


int mem_write_chain(void *source_buffer, uintptr_t begin_addr, uintptr_t *offset_chain, 
                    DWORD num_offsets, DWORD data_size)
{
    uintptr_t write_addr;
    
    // By calling mem_read_chain with num_offsets = num_offsets - 1, write_addr will contain the
    // address value of the target memory location. Calling mem_read_chain with 
    // num_offsets = num_offsets will read the value stored at the memory location, not its address. 
    if(mem_read_chain(&write_addr, begin_addr, offset_chain, num_offsets - 1, 
       sizeof(uintptr_t)) == 0)
        return 0;
    
    if(WriteProcessMemory(GetCurrentProcess(), (LPVOID) write_addr, (LPCVOID) source_buffer, 
           sizeof(size_t *), NULL) == false)
        return 0;
    return 1;    
}