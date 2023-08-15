#include <math.h>
#include <Windows.h>
#include <Windowsnumerics.h>
#include <iostream>

#include "esp.hpp"
#include "offsets.hpp"
#include "os_util.hpp"

using namespace Windows::Foundation::Numerics;
uintptr_t        base_addr                                                                         ;
float3           camera_loc                                                                        ;
entity           **entities               = new  entity*                                           ;
DWORD            entity_count             = 0                                                      ;
std::string      *entity_filters                                                                   ;
float3           fov                                                                               ;
HANDLE           proc                                                                              ;
float3           player_pos               = float3()                                               ;

struct entity
{
    std::string name;
    float3 pos;	
};
//Player pos is filled with data such that player_pos[0] = x, player_pos[1] = y, player_pos[2] = z

int calc_plyr_dist_to_entity(float3 &pos)
{
	return (int) sqrt(pow((player_pos.x - pos.x), 2) + pow((player_pos.y - pos.y), 2) +
                      pow((player_pos.z - pos.z), 2));
}


int esp_init(HANDLE h_process)
{
    proc = h_process;
    DWORD process_id = find_process_id(_base_executable);   
    base_addr = get_mdle_begin_addr(process_id, _base_module);
    return 1;
}


int itr_entity(int pos, std::string &entity_name, float ent_pos[2], int &dist)
{
	if(pos >= entity_count)
		return -1;
	
	entity ent = *(entities[pos]);
	float2 ent_2d_pos;
	
	if(world_to_screen(ent.pos, ent_2d_pos) == false)
		return 0;
	
	entity_name = ent.name;
	ent_pos[0] = ent_2d_pos.x;
	ent_pos[1] = ent_2d_pos.y;
	
	dist = calc_plyr_dist_to_entity(ent.pos);
	return 1;
}


void refresh_esp()
{
    refresh_local_player();       
}
    
    
void refresh_entities()
{
    /*
    DWORD num_entities = (DWORD) mem_read_chain(base_addr, entity_chain, num_entity_chain_len, 
                                                h_process);
	
	for(int i = 0; i < entity_count; i++)
        delete entities[i];
	
	entity_count = 0;
	delete entities;
	entities = new entity*[num_entities];

    for(int i = 0; i < num_entities; i++)
    {
        /*DWORD obj_name_chain[4] = {(DWORD) (0x20 + (i * 0x8)), 0x10, 0x30, 0x60};
        intptr_t obj_name = mem_read_chain(entity_buffer, obj_name_chain, 4, h_process);
			
	    mem_read_buffer(main_camera + 0X2E4, &view_matrix, 64, h_process);

        char obj_name_str[111];
        obj_name_str[110] = '\0';
        
        mem_read_buffer(obj_name, obj_name_str, 110, h_process);

        if(strstr(obj_name_str, "stone-ore.prefab"))
        {
            float vector_buffer[3];
            
            DWORD pos_container_chain[6] = {(DWORD) (0x20 + (i * 0x8)), 0x10, 0x30, 0x30, 0x8, 
                                                          0x38};
            uintptr_t pos_container = mem_read_chain(entity_buffer, pos_container_chain, 
                                                         6, h_process);
                                                 
            mem_read_buffer(pos_container + 0x90, vector_buffer, 12, h_process);
            
			entity *new_entity = new entity{"Stone Ore", float3(vector_buffer[0], 
			                                vector_buffer[1] + 2, vector_buffer[2])};
											
											
	        entities[entity_count] = new_entity;
			entity_count += 1;
        }
		
		if(strstr(obj_name_str, "LocalPlayer"))
        {      
            DWORD pos_container_chain[6] = {(DWORD) (0x20 + (i * 0x8)), 0x10, 0x30, 0x30, 0x8, 
                                                          0x38};
            uintptr_t pos_container = mem_read_chain(entity_buffer, pos_container_chain, 
                                                         6, h_process);
                                                 
            mem_read_buffer(pos_container + 0x90, &player_pos, 12, h_process);
        }*/
    
}


void refresh_local_player()
{
    uintptr_t x_off_array[] = _player_pos_chain_x, y_off_array[] = _player_pos_chain_y, 
              z_off_array[] = _player_pos_chain_z;
              
    mem_read_chain(base_addr, x_off_array, 2, 4, &player_pos.x, proc);   
    mem_read_chain(base_addr, y_off_array, 2, 4, &player_pos.y, proc);
    mem_read_chain(base_addr, z_off_array, 2, 4, &player_pos.z, proc);
}


bool world_to_screen(float3 &origin, float2 &out)
{/*
    float3x3 v_transformed
    float3x3 v_transformed = {vDelta.Dot({ tempMatrix.m[1][0], tempMatrix.m[1][1], tempMatrix.m[1][2] }),
			                 vDelta.Dot({ tempMatrix.m[2][0], tempMatrix.m[2][1], tempMatrix.m[2][2] }),
			               v Delta.Dot({ tempMatrix.m[0][0], tempMatrix.m[0][1], tempMatrix.m[0][2] })};

		if (vTransformed.z < 1.f)
			vTransformed.z = 1.f;

		screenPos.x = winCenter.x + vTransformed.x * (winCenter.x / tanf(this->FOV * M_PI / 360)) / vTransformed.z;
		screenPos.y = winCenter.y - vTransformed.y * (winCenter.x / tanf(this->FOV * M_PI / 360)) / vTransformed.z;

		if (screenPos.x > winSize.x || screenPos.x < 0.f || screenPos.y > winSize.y || screenPos.y < 0.f)
			return false;

		return true;*/
    return true;
}


/*
bool world_to_screen_unity(float3 &origin, float2 &out)
{
    float4x4 temp;
    
    temp = transpose(view_matrix);
    float3 translation_vector = float3(temp.m41, temp.m42, temp.m43);
    float3 up = float3(temp.m21, temp.m22, temp.m23);
    float3 right = float3(temp.m11, temp.m12, temp.m13);
  
    float w = dot(translation_vector, origin) + temp.m44;
    
    if (w < 0.098f)
    	return false;
     
    float y = dot(up, origin) + temp.m24;
    float x = dot(right, origin) + temp.m14;
     
    out.x = (1920 / 2) * (1.f + x / w);
    out.y = (1080 / 2) * (1.f - y / w);
     
    return true;
}*/
    