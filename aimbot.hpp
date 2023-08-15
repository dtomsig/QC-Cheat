DWORD WINAPI create_mouse_proc(LPVOID);

void adj_if_firing();

void destroy_aimbot();

int init_aimbot();

void move_mouse(int amt_x, int amt_y);

void rotate_view_screen_offset(int amt_x, int amt_y);

void scan_set_target();

void scan_set_weapon();