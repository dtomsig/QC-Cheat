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

enum weapons {MACHINE_GUN, SHOTGUN, NAIL_GUN, TRIBOLT, ROCKET_LAUNCHER, LIGHTNING_GUN, RAIL_GUN};

static bool valid_target, assist_on;
static cv::Point2i top_lft, target;
static enum weapons current_weapon;
static HBITMAP h_bitmap;
static HDC hdc, h_screen;
static HGDIOBJ old_obj;
static int x_1, y_1, x_2, y_2, scrn_w, scrn_h, fov_w, fov_h;
extern HANDLE game_handle;
extern uintptr_t base_address;


// To ensure that weapon firing occurs after camera rotation, a left click is simulated. The fire
// weapon binding in game must be set to num 2.
void adj_siml_if_firing()
{    
    // Process actions when fire button is pressed.
    if(GetAsyncKeyState(VK_LBUTTON) & 0x8000)
    {            
        if(valid_target && assist_on)
        {
            POINT curs_pos;
            GetCursorPos(&curs_pos);
            if(current_weapon == MACHINE_GUN || current_weapon == LIGHTNING_GUN)
                mouse_event(MOUSEEVENTF_MOVE, target.x, target.y, NULL, NULL);
            else
                rotate_view_screen_offset(target.x, target.y);
        }

        //Structure for the keyboard event
        INPUT ip;
        //Set up the INPUT structure
        ip.type = INPUT_KEYBOARD;
        ip.ki.time = 0;
        ip.ki.wVk = 0; //We're doing scan codes instead
        ip.ki.dwExtraInfo = 0;
        //This let's you do a hardware scan instead of a virtual keypress
        ip.ki.dwFlags = KEYEVENTF_SCANCODE;
        ip.ki.wScan = 0x50; //code character to use(p)
        SendInput(1, &ip, sizeof(INPUT)); // sending the keypress
        ip.ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
        SendInput(1, &ip, sizeof(INPUT)); // sending the upkeypress
        cv::Mat im_with_keypoints;
    }
}


// Returns 1 if two floats have the same sign. Does not support infinite or NaN floats.
// Inline is a common optimization technique that is used for small math functions.
inline int cmp_sign(float a, float b)
{
    return a*b >= 0.0f;
}


void destroy_aimbot()
{
    SelectObject(hdc, old_obj);
    DeleteDC(hdc);
    ReleaseDC(NULL, h_screen);
    DeleteObject(h_bitmap);
}


void init_aimbot()
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

    // Initialize contexts.
    h_screen = GetDC(NULL);
    hdc      = CreateCompatibleDC(h_screen);
    h_bitmap = CreateCompatibleBitmap(h_screen, fov_w, fov_h);
    old_obj  = SelectObject(hdc, h_bitmap);
}


void move_mouse(int amt_x, int amt_y)
{
    mouse_event(MOUSEEVENTF_MOVE, amt_x, amt_y, NULL, NULL);
}


// Rotates the screen by an offset amount, amt_x and amt_y. These are points relative to the center
// of the screen. An amt_x = 100 and amt_y = -100 would rotate the view matrix to face an object
// that is 100 pixels above the center of the screen and 100 pixels to the right of the center of
// the screen. The coordinate system is the coordinate system for a window in Windows where the
// top left pixel has position (0,0).
// https://learn.microsoft.com/en-us/windows/win32/gdi/window-coordinate-system
void rotate_view_screen_offset(int amt_x, int amt_y)
{
    float cur_pitch_angle, cur_yaw_angle, fov_horz, fov_vert, targ_pitch_angle, targ_yaw_angle, 
          view_matrix[4][4];
    uintptr_t addr_view_matrix;
    
    
    if(mem_chain_addr_resolve(&addr_view_matrix, base_address, _view_matrix_chain, 4,
       NULL) == 0)
    {
        return;
    }

    // View matrix is 16 float values so 16 * sizeof(float) must be read. I elected to allocate 
    // stack memory so the entire view matrix gets copied. This reduces the amount of write calls
    // to make the code simpler. The entire view matrix buffer is written instead of specific
    // offsets.
    if(mem_read_buffer(view_matrix, addr_view_matrix, sizeof(float) * 16, NULL) == 0)
        return;

    // View Matrix Mapping (reviewed view matrix and found it to be the identical
    // to layout as described in "FPS Camera" below).
    // https://www.3dgep.com/understanding-the-view-matrix/
    // The yaw angle is a bit tricky. In order to calculate the yaw angle, both the 
    // view_matrix[0][0] and view_matrix[0][2] values [cos(yaw_angle) and sin(yaw_angle)] 
    // must be factored in. This is because yaw angles range from 0 to 360 degrees. Simply
    // taking the acos of view_matrix[0][0] to find the yaw angle will not work. From 0 to 360 
    // degrees, the cosine function will have two degree values that will produce the same value. 
    // E.g. cos(30 degrees) = cos(330 degrees) = 0.86. 
    // Therefore, the sin of the yaw angle must be factored in as well (view matrix[0][2]).
    // The exact degree value can be calculated by looking at these two values.
    // 
    // Since pitch only ranges from -90 to 90 degrees, taking the asin of the pitch value will
    // always result in the correct pitch angle.
    
    // To find the yaw angle, I first calculate acos(view_matrix[0][0]). 
    // If the sign of acos(view_matrix[0][0] is different than view_matrix[0][2], then the angle is 
    // 2PI - acos(view_matrix[0][0]).
    // Again, this is because there are two possible angles associated with value in
    // view_matrix[0][0] as yaw angles range from 0 to 360 degrees.
    
    // View Matrix:
    // {                    cos(yaw_angle),                 0,                   -sin(yaw_angle), 0}
    // { sin(yaw_angle) * sin(pitch_angle),  cos(pitch_angle), cos(yaw_angle) * sin(pitch_angle), 0}
    // { sin(yaw_angle) * cos(pitch_angle), -sin(pitch_angle), cos(pitch_angle) * cos(yaw_angle), 0}
    // {                         UNCHANGED,         UNCHANGED,                         UNCHANGED, 1}

    cur_pitch_angle   = -asinf(view_matrix[2][1]);
    cur_yaw_angle     = acosf(view_matrix[0][0]);
   
    if(cmp_sign(acos(view_matrix[0][0]), -asin(view_matrix[0][2])) == false)
        cur_yaw_angle = 2*PI - cur_yaw_angle;
        
    targ_yaw_angle    = cur_yaw_angle - (2*PI/360.0f)*(120.0f)*amt_x/scrn_w;       
    targ_pitch_angle  = cur_pitch_angle + (2*PI/360.0f)*(89.0f)*amt_y/scrn_h; 

    // Write the correct values to the view_matrix. First they are stored in the stack copy. Then, 
    // the buffer is copied to the game's memory with mem_write_buffer().
    view_matrix[0][0] = cosf(targ_yaw_angle);
    view_matrix[0][2] = -sinf(targ_yaw_angle);
    view_matrix[1][0] = sinf(targ_yaw_angle) * sinf(targ_pitch_angle);
    view_matrix[1][1] = cosf(targ_pitch_angle);
    view_matrix[1][2] = cosf(targ_yaw_angle)*sinf(targ_pitch_angle);
    view_matrix[2][0] = sinf(targ_yaw_angle)*cosf(targ_pitch_angle);
    view_matrix[2][1] = -sinf(targ_pitch_angle);
    view_matrix[2][2] = cosf(targ_pitch_angle)*cosf(targ_yaw_angle);

    // No error checking is done here. That is because it's the last statement and an error
    // would have no impact.
    mem_write_buffer(view_matrix, addr_view_matrix, sizeof(float) * 16, NULL);
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
    // the number of pixels that montiors have. I used uint64_t for the total_x and total_y because
    // in theory a monitor could be 128 million x 1 pixels or 1 x 128 million pixels in size. The
    // maximum number of pixels that can be in a Windows monitor is 128 million. To sum all of the
    // coordinates for this hypothetical monitor you would add (0 + 1 + 2 + 3 .... 128 milion - 1).
    // This is smaller than 2^64 but larger than 2^32.
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
        // The coordinate system is the same as the coordinate system for a window in Windows where
        // the top left is represented by (0,0).
        int targ_x = round((float) total_x / num_pts) - round(fov_w/2.0f);
        int targ_y = round((float) total_y / num_pts) - round(fov_h/2.0f);
        
        target = cv::Point2i(targ_x, targ_y);  
    }

    #ifdef IMG_DEBUG
        cv::imshow("range output", range_output);
        cv::imshow("mat", mat);
        cv::waitKey();
    #endif
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