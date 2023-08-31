int find_process_id(DWORD &process_id, const std::string &process_name);

uintptr_t get_mdle_begin_addr(DWORD process_id, const std::string &module_name);

int inject_dll(DWORD pid, std::string path);

int mem_chain_addr_resolve(uintptr_t *addr_buffer, uintptr_t begin_addr, uintptr_t *offset_chain, 
                           uintptr_t num_offsets, HANDLE h_process);
                           
int mem_read_buffer(void *target_buffer, uintptr_t addr, uintptr_t num_bytes, 
                     HANDLE h_process);
                     
int mem_write_buffer(void *source_buffer, uintptr_t addr, uintptr_t num_bytes, HANDLE h_process);
