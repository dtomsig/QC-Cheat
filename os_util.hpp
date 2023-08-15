DWORD find_process_id(const std::string &process_name);

uintptr_t get_mdle_begin_addr(DWORD process_id, const std::string &module_name);

int inject_dll(DWORD pid, std::string path);

void mem_read_buffer(uintptr_t begin_addr, void *target_buffer, DWORD num_bytes, HANDLE &h_process);

int mem_read_chain(void *target_buffer, uintptr_t begin_addr, uintptr_t *offset_chain, DWORD num_offsets, 
                   DWORD data_size);

int mem_read_chain_hndl(void *target_buffer, uintptr_t begin_addr, uintptr_t *offset_chain, DWORD num_offsets, 
                        DWORD data_size, HANDLE &h_process);
