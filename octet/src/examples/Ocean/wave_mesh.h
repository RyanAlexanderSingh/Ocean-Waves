///////////////////////////////////////f////////////////////////////////////////
//
// (C) Ryan Singh 2015
//
// Wave Mesh generated using sin waves --> enter generic algorithm name here
//

#ifndef WAVE_MESH_H_INCLUDED
#define WAVE_MESH_H_INCLUDED

namespace octet{

  class wave_mesh : public resource{
  
  ref<visual_scene> app_scene;
  public:
    wave_mesh(){}

  };
  
  struct my_vertex {
    vec3p pos;
    vec3p nor;
    uint32_t color;
  };

}
#endif