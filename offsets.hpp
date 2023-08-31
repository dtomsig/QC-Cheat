#include <iostream>

extern std::string     _base_executable                                   ;
extern std::string     _base_module                                       ;

/* pattern: 24 FE 3A 00 28 DB 71 02 24 FE 3A 00 58 FE 3A 00 */
extern uintptr_t       _view_matrix_chain[]                               ;

/* pattern: 20 0D 98 02 00 CE 0F 00 22 CE 0F 00 98 0D 98 02 */
extern uintptr_t       _weapon_selection_chain[]                          ;