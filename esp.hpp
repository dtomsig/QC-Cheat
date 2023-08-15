#include <Windowsnumerics.h>

struct entity;

int calc_plyr_dist_to_entity(Windows::Foundation::Numerics::float3 &pos);

int esp_init(HANDLE h_process);

int itr_entity(int pos, std::string &entity_name, float ent_pos[2], int &dist);
                                 
void refresh_esp();

void refresh_entities();

void refresh_local_player();

bool world_to_screen(Windows::Foundation::Numerics::float3 &origin, 
                     Windows::Foundation::Numerics::float2 &out);
                                                