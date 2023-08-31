#include <iostream>
#include "offsets.hpp"

std::string     _base_executable             = "QuakeChampions.exe"                                ;
std::string     _base_module                 = "QuakeChampions.exe"                                ;

uintptr_t       _view_matrix_chain[]         = {0x02B38380, 0x568, 0x8, 0xA0}                      ;

uintptr_t       _weapon_selection_chain[]    = {0x02B07AF0, 0xC8, 0x2F8, 0x50, 0x88, 0x50, 0x188, 
                                                0x188}                                             ;