#include <iostream>

void clean_up_device_d3d();

void clean_up_render_target();

bool create_device_d3d(HWND hWnd);

void create_window(HINSTANCE &instance, INT cmd_show);

void create_render_target();

void create_window(std::string window_name, HINSTANCE &h_instance);

int get_esp_state();

bool render_frame_begin();

void render_frame_end();

void render_frame_gui();

bool render_init();

void set_gui_background();

void set_gui_foreground();

void set_render_window_top();

int set_target_window(const std::string &window_name);