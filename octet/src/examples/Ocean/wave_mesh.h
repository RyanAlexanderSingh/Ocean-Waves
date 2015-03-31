///////////////////////////////////////f////////////////////////////////////////
//
// (C) Ryan Singh 2015
//
// Wave Mesh generated using sin waves --> enter generic algorithm name here
//

#ifndef WAVE_MESH_H_INCLUDED
#define WAVE_MESH_H_INCLUDED


#define PI 3.14159265359  /* pi */
#define TWO_PI 6.28318530718

#include <vector>

namespace octet{

  class wave_mesh : public resource{

    // this is the vertex format used in this sample.
    // we have 12 bytes of float position (x, y, z) and four bytes of color (r, g, b, a)
    struct my_vertex {
      vec3p pos;
      uint32_t color;
    };

    //our struct for the sine wave
    struct sine_wave{
      float amplitude;
      float speed;
      vec3 direction;
      float frequency;
      float omega;
    };

    //our array of sine waves
    dynarray<sine_wave> sine_waves;

    ref<visual_scene> the_app;
    size_t mesh_size = 120;

    dynarray<vec3p> points;
    float total_steepness = 1.0f;  // 0: sine wave, 1: maximum value
    float offset = 1.0f;
    int num_of_waves = 1.0f;
    static unsigned long long time_step;

    mesh *water;

    // Calculate mesh vertices using the Gernster Wave Function
    // Does so cumulatively over a given number of sine waves
    // this is used to update the vertices in the update loop
    // and also calculates the wave height
    vec3 compute_gerstner_points(int x, int y){

      //store the Gerstner wave function to a vector
      vec3 wavePosition;

      for (size_t i = 0; i < sine_waves.size(); ++i){
        // calculate each of the points according to Gerstner's wave function yo!
        sine_wave wave = sine_waves[i];
        float steepness = total_steepness / (wave.omega * sine_waves.size());
        float radians = wave.frequency * wave.direction.dot(vec3(x, y, 0.0f)) + time_step * wave.omega;

        //add to our position vector
        wavePosition.x() += steepness * wave.direction.x() * cosf(radians);
        wavePosition.y() += steepness * wave.direction.y() * cosf(radians);
        wavePosition.z() += wave.amplitude * sinf(radians);
      }
      return wavePosition;
    }

    static uint32_t make_color(vec3 col) {
      return 0xff000000 + ((int)(col.x()*255.0f) << 0) + ((int)(col.y()*255.0f) << 8) + ((int)(col.z()*255.0f) << 16);
    }

    // this function converts three floats into a RGBA 8 bit color
    static uint32_t make_color(float r, float g, float b) {
      return 0xff000000 + ((int)(r*255.0f) << 0) + ((int)(g*255.0f) << 8) + ((int)(b*255.0f) << 16);
    }

    //generate the wave simulation by making the sine waves
    void generate_waves(){

      // set base wave attributes; each successive wave will have half / twice the value
      float freq = TWO_PI * 0.02f;
      float phase = 3.0f;
      float ampl = 3.0f;

      for (int i = 0; i < num_of_waves; ++i){
        sine_wave sineWave;
        sineWave.amplitude = ampl * std::pow(1, (i + 1));
        sineWave.direction = (1.0f, 1.0f, 0.0f);
        sineWave.frequency = freq;
        sineWave.omega = phase * freq;
        sine_waves.push_back(sineWave);
      }
    }

    //rebuild the mesh every frame -- we need to recalculate it
    void update_mesh(){
      // these write-only locks give access to the vertices and indices.
      // they will be released at the next } (the end of the scope)
      gl_resource::wolock vl(water->get_vertices());
      my_vertex *vtx = (my_vertex *)vl.u8();
      gl_resource::wolock il(water->get_indices());
      uint32_t *idx = il.u32();

      // make the vertices
      for (size_t i = 0; i != mesh_size; ++i) {
        for (size_t j = 0; j != mesh_size; ++j) {
          vtx->pos = points[j + i * mesh_size];
          vtx->color = (0.0f, 0.2f, 1.0f);
          vtx++;
        }
      }

      // make the triangles
      uint32_t vn = 0;
      for (size_t i = 0; i != mesh_size * (mesh_size - 1); ++i) {
        if (i % mesh_size != mesh_size - 1){
          idx[0] = i;
          idx[1] = i + mesh_size + 1;
          idx[2] = i + 1;
          idx += 3;

          idx[0] = i;
          idx[1] = i + mesh_size;
          idx[2] = i + mesh_size + 1;
          idx += 3;
        }
      }
    }

  public:
    wave_mesh(){}

    //we're going to want an init function
    void init(visual_scene *vs){

      this->the_app = vs;
      param_shader *shader = new param_shader("shaders/default.vs", "shaders/default_solid.fs");
      material *water_material = new material(vec4(0, 0, 1, 1), shader);

      //create a new mesh
      water = new mesh();

      //make the points the size of mesh_size squared
      size_t sq_mesh_size = mesh_size * mesh_size;
      points.resize(sq_mesh_size);

      // allocate vertices and indices into OpenGL buffers
      size_t num_vertices = mesh_size * mesh_size;
      size_t num_indices = (mesh_size - 1) * (mesh_size - 1) * 6;
      water->allocate(sizeof(my_vertex) * num_vertices, sizeof(uint32_t) * num_indices);
      water->set_params(sizeof(my_vertex), num_indices, num_vertices, GL_LINES, GL_UNSIGNED_INT);

      // describe the structure of my_vertex to OpenGL
      water->add_attribute(attribute_pos, 3, GL_FLOAT, 0);
      water->add_attribute(attribute_normal, 3, GL_FLOAT, 0);
      water->add_attribute(attribute_color, 4, GL_UNSIGNED_BYTE, 12, GL_TRUE);

      generate_waves();
      update();

      scene_node *node = new scene_node();
      //add the mesh to the scene
      the_app->add_mesh_instance(new mesh_instance(node, water, water_material));
    }

    //update function to update the points of the mesh
    void update(){
      for (size_t i = 0; i != mesh_size; ++i) {
        for (size_t j = 0; j != mesh_size; ++j) {
          vec3 wavePosition = compute_gerstner_points(j, i);
          points[j + i * mesh_size] = vec3p((0.0f, 0.0f, 0.0f) + vec3(offset * j, -offset * i, 0.0f) + wavePosition);
        }
      }
      ++time_step;
      update_mesh(); //once each point is updated, apply to the mesh
    }
  };
  unsigned long long wave_mesh::time_step = 0;
}
#endif