#include <Windows.h>
#include <iostream>
#include "aimbot.hpp"
#include "chams.hpp"
#include "minhook/MinHook.h" 
#include "os_util.hpp"
#include "offsets.hpp"

#define AIMBOT_TGL 0001


uintptr_t base_address;
static int AIMBOT_STATE;

int cheat_init();
DWORD __stdcall run_cheat_loop(LPVOID);
void set_error_state();


BOOL __stdcall DllMain(HINSTANCE hModule, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
            if(cheat_init() == 0)
                return FALSE;
            DisableThreadLibraryCalls(hModule);
            CreateThread(NULL, 0, install_chams, NULL, 0, NULL);
            CreateThread(NULL, 20971520, run_cheat_loop, NULL, 0, NULL);
            break;

        case DLL_PROCESS_DETACH:
            CreateThread(NULL, 0, disable_chams, NULL, 0, NULL);
            break;
    }
    
    return TRUE;
}


int cheat_init()
{
    DWORD process_id;

    find_process_id(process_id, _base_executable);
    if(process_id == 0)
        return 0;

    base_address = get_mdle_begin_addr(process_id, _base_module);
    if(base_address == 0)
        return 0;

    return 1;
}


DWORD __stdcall run_cheat_loop(LPVOID)
{
    while(true)
    {
        if((GetAsyncKeyState(VK_NEXT) & 0x0001))
        {
            AIMBOT_STATE = AIMBOT_STATE ^ AIMBOT_TGL;
            
            //Actions taken as aimbot is turned on.
            if(AIMBOT_STATE == 0001)
                init_aimbot();
            else
                destroy_aimbot();
        }

        if(AIMBOT_STATE == 0001)
        {
            scan_set_weapon();
            scan_set_target();
            adj_siml_if_firing();
        }
    }
    
    return 1;
}

