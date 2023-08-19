#include <iostream>
#include <math.h>
#include <vector>
#include <windows.h>

#include "aimbot.hpp"
#include "opencv2/opencv.hpp"
#include "offsets.hpp"
#include "os_util.hpp"

#define FOV_PERCENT_X 0.3
#define FOV_PERCENT_Y 0.3
#define PI 3.14159265

static bool valid_target, assist_on;
static cv::Point2i top_lft, target;               
static HBITMAP h_bitmap;
static HDC hdc, h_screen;
static HGDIOBJ old_obj;
static int x_1, y_1, x_2, y_2, scrn_w, scrn_h, fov_w, fov_h, weapon_scan_ctr;
extern HANDLE game_handle;
enum weapons {MACHINE_GUN, SHOTGUN, NAIL_GUN, TRIBOLT, ROCKET_LAUNCHER, LIGHTNING_GUN, RAIL_GUN};
extern uintptr_t base_address; 
static enum weapons current_weapon;


void adj_if_firing()
{    
    // Process actions when fire button is pressed.   
    if(GetAsyncKeyState(VK_LBUTTON) & 0x8000)
    {     
        INPUT mouse_input;
        INPUT keybd_inputs[2];
        ZeroMemory(&mouse_input, sizeof(mouse_input));
        ZeroMemory(keybd_inputs, sizeof(keybd_inputs));    
        
        if(valid_target && assist_on)
        {
            POINT curs_pos;
            GetCursorPos(&curs_pos);
            move_mouse(target.x, target.y);
        }
                
        if(current_weapon == SHOTGUN || current_weapon == RAIL_GUN)
            Sleep(200);
        
        #ifdef IMG_DEBUG
        cv::imshow("range output", range_output);
        cv::imshow("mat", mat);
        cv::waitKey();
        #endif
    }            
}


void destroy_aimbot()
{
    SelectObject(hdc, old_obj);
    DeleteDC(hdc);
    ReleaseDC(NULL, h_screen);
    DeleteObject(h_bitmap);
}


int init_aimbot()
{
    // Get screen dimensions.
    x_1 = GetSystemMetrics(SM_XVIRTUALSCREEN);
    y_1 = GetSystemMetrics(SM_YVIRTUALSCREEN);
    x_2 = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    y_2 = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    scrn_w = x_2 - x_1;
    scrn_h = y_2 - y_1;
    fov_w  = round(scrn_w * FOV_PERCENT_X);
    fov_h  = round(scrn_h * FOV_PERCENT_Y);
    top_lft = cv::Point2f(scrn_w/2 - fov_w/2, scrn_h/2 - fov_h/2);
e
    // Initialize contexts.
    h_screen = GetDC(NULL);
    hdc      = CreateCompatibleDC(h_screen);
    h_bitmap = CreateCompatibleBitmap(h_screen, fov_w, fov_h);
    old_obj  = SelectObject(hdc, h_bitmap);
    
    if(h_screen == NULL || hdc == NULL || h_bitmap == NULL || old_obj == NULL)
        return 1;
    return 0;
}


void move_mouse(int amt_x, int amt_y)
{
    mouse_event(MOUSEEVENTF_MOVE, amt_x, amt_y, NULL, NULL);
}


void rotate_view_screen_offset(int amt_x, int amt_y)
{
  /*  float cur_pitch_rotation, targ_pitch_rotation, cur_yaw_rotation, targ_yaw_rotation, 
          cur_pitch_angle;
    
    cur_pitch_angle = acos(cur_pitch_rotation);
    cur_yaw_angle = asin(cur_yaw_rotation);
    
    cur_pitch_rotation = sin(cur_pitch_angle + PI * amt_y / scrn_h * 0.5);
    cur_yaw_rotation = cos(c*/
 //   if(mem_read_chain(&cur_pitch_rotation, base_address, sizeof(float))
    //18 is positive cosine of pitch_rotation
    //44 is negative cosine of pitch_rotation
    //7 is sin of pitch rotation
    //10 is sin of pitch rotation
 //   if(mem_read_chain(&cur_weapon, base_address, _weapon_selection_chain, 8, 4) == 0)
  //      return;
}


void scan_set_target()
{
    // Load screenshot of screen into bitmap file.
    BOOL b_ret = BitBlt(hdc, 0, 0, fov_w, fov_h, h_screen, top_lft.x, top_lft.y, SRCCOPY);

    cv::Mat mat(fov_h, fov_w, CV_8UC4);

    BITMAPINFOHEADER bi = {sizeof(bi), fov_w, -fov_h, 1, 32, BI_RGB};
    GetDIBits(hdc, h_bitmap, 0, fov_h, mat.data, (BITMAPINFO*)&bi, DIB_RGB_COLORS);
    
    // Filter out so only magenta colored points are present.
    cv::Mat hsv_img, range_output;
    cv:cvtColor(mat, hsv_img, cv::COLOR_BGR2HSV, 0);
    cv::inRange(hsv_img, cv::Scalar(145, 95, 65), cv::Scalar(160 , 255, 200), range_output);

    // Find the average of all points that match the color model in order to calculate a target.
    std::vector<cv::Point> trg_points;
    findNonZero(range_output, trg_points);
    
    // It is safe to use a 32 bit int for number of points. 4294967296 (2^32) is much larger than 
    // the number of pixels that montiors have. Similarily, total_x and total_y are comfortable
    // sizes to represent the total x and y coordinate value of all pixels in a monitor.
    uint32_t num_pts = trg_points.size();
    uint64_t total_x = 0, total_y = 0;

    for(int i = 0; i < num_pts; i++)
    {
        total_x += trg_points.at(i).x;
        total_y += trg_points.at(i).y;
    }
        
    if(num_pts == 0)
        valid_target = false;
    else
    {
        valid_target = true;
        POINT curs_pos;
        GetCursorPos(&curs_pos);
        int targ_x = round((float) total_x / num_pts) - round(fov_w/2.0f);
        int targ_y = round((float) total_y / num_pts) - round(fov_h/2.0f);
        
        target = cv::Point2i(targ_x, targ_y);  
    }
}


void scan_set_weapon()
{ 
    int cur_weapon;
    uintptr_t addr_cur_weapon;
    
    if(mem_chain_addr_resolve(&addr_cur_weapon, base_address, _weapon_selection_chain, 8,
       NULL) == 0)
    {
        return;
    }

    if(mem_read_buffer(&cur_weapon, addr_cur_weapon, sizeof(cur_weapon), NULL) == 0)
        return;

    switch(cur_weapon)
    {
        case 1:
            current_weapon = MACHINE_GUN;
            assist_on = true;
            break;
        case 2:
            current_weapon = SHOTGUN;
            assist_on = true;
            break;
        case 3:
            current_weapon = NAIL_GUN;
            assist_on = false;
            break;
        case 4:
            current_weapon = TRIBOLT;
            assist_on = false;
            break;
        case 5:
            current_weapon = ROCKET_LAUNCHER;
            assist_on = false;
            break;
        case 6:
            current_weapon = LIGHTNING_GUN;
            assist_on = true;
            break;
        case 7:
            current_weapon = RAIL_GUN;
            assist_on = true;
            break;                               
   }
}



 

