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

    visual_scene *vs;

    // this is the vertex format used in this sample.
    // we have 12 bytes of float position (x, y, z) and four bytes of color (r, g, b, a)
    struct my_vertex {
      vec3p pos;
      uint32_t color;
    };

    struct example_geometry_source : mesh_terrain::geometry_source {
      mesh::vertex vertex(
        vec3_in bb_min, vec3_in uv_min, vec3_in uv_delta, vec3_in pos
        ) {
        static const vec3 bumps[] = {
          vec3(100, 0, 100), vec3(50, 0, 50), vec3(150, 0, 50)
        };

        float y =
          std::exp((pos - bumps[0]).squared() / (-100.0f)) * 3.0f +
          std::exp((pos - bumps[1]).squared() / (-100.0f)) * 4.0f +
          std::exp((pos - bumps[2]).squared() / (-10000.0f)) * (-20.0f) +
          (15.0f)
          ;

        float dy_dx = std::cos(pos.x() * 0.01f);
        float dy_dz = std::cos(pos.z() * 0.03f);
        vec3 p = bb_min + pos + vec3(0, y, 0);
        vec3 normal = normalize(vec3(dy_dx, 1, dy_dz));
        vec3 uv = uv_min + vec3((float)pos.x(), (float)pos.z(), 0) * uv_delta;
        return mesh::vertex(p, normal, uv);
      }
    };

    example_geometry_source source;

  public:
    wave_mesh(){
      init(); //default call to init the mesh
    }

    //we're going to want an init function
    void init(){
    }

    //update function
    void update(){

    }

  };

}
#endif