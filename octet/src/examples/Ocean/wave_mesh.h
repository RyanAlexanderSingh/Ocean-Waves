///////////////////////////////////////f////////////////////////////////////////
//
// (C) Ryan Singh 2015
//
// Based off Chapter 1. Effective Water Simulation from Physical Models http://http.developer.nvidia.com/GPUGems/gpugems_ch01.html
// Using the Gerstner wave function 
//

#ifndef WAVE_MESH_H_INCLUDED
#define WAVE_MESH_H_INCLUDED


#define PI 3.14159265359f  /* PI */
#define TWO_PI 6.28318530718f /* TWO PI */

#include <random>
#include <fstream>

namespace octet{

  class wave_mesh : public resource{

    // this is the vertex format used in this sample.
    // we have 12 bytes of float position (x, y, z) and four bytes of color (r, g, b, a)
    struct my_vertex {
      vec3p pos;
      vec3p norm;
      uint32_t color;
    };

    //our struct for the sine wave
    struct sine_wave{
      float amplitude;
      float speed;
      float frequency;
      float steepness;
      vec3 direction;
    };

    //our array of sine waves
    dynarray<sine_wave> sine_waves;
    dynarray<std::string> params;
    ref<visual_scene> the_app;
    mesh *water;

    float freq_ = 0.0f, ampli_ = 0.0f, speed_ = 0.0f, steepness_ = 0.0f;
    int num_of_waves = 1.0f; //change this
    size_t mesh_size = 120; //size of our mesh
    unsigned long long time_step = 0; //the simulation could go on for a really long time

    random rand; // random number for wave pos

    vec3 gerstner_wave_function(int x_pos, int y_pos){
      //store the Gerstner wave function to a vector
      vec3 wavePosition;

      //for each sine wave
      for (unsigned i = 0; i < sine_waves.size(); ++i){
        sine_wave wave = sine_waves[i];

        float angle = (wave.frequency * wave.direction.dot(vec3(x_pos, y_pos, 0.0f))) + wave.speed * time_step;

        //add to our position vector
        wavePosition.x() += (wave.steepness * wave.amplitude) * wave.direction.x() * cosf(angle);
        wavePosition.y() += (wave.steepness * wave.amplitude) * wave.direction.y() * cosf(angle);
        wavePosition.z() += wave.amplitude * sinf(angle);
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

      //open default file
      open_file("../../../assets/wave_configs/wave_config1.txt");

      //create the sine waves and give the parameters some default behaviours
      for (int i = 0; i < num_of_waves; ++i){
        sine_wave sineWave;
        sineWave.amplitude = ampli_;
        sineWave.speed = speed_;
        sineWave.frequency = freq_;
        sineWave.steepness = steepness_;
        sineWave.direction = vec3(rand.get(-1.0f, 1.0f), rand.get(-1.0f, 1.0f), 0.0f),
        sine_waves.push_back(sineWave); //add to dynarray
      }
    }

  public:
    wave_mesh(){}

    //we're going to want an init function
    void init(visual_scene *vs){

      this->the_app = vs;
      param_shader *shader = new param_shader("shaders/default.vs", "shaders/default_solid.fs");
      material *water_material = new material(vec4(0.3f, 0.6f, 1.0f, 1), shader);

      //create a mesh object
      water = new mesh();

      // allocate vertices and indices into OpenGL buffers
      size_t num_vertices = mesh_size * mesh_size;
      size_t num_indices = (mesh_size - 1) * (mesh_size - 1) * 6;
      water->allocate(sizeof(my_vertex) * num_vertices, sizeof(uint32_t) * num_indices);
      water->set_params(sizeof(my_vertex), num_indices, num_vertices, GL_TRIANGLES, GL_UNSIGNED_INT);

      // describe the structure of my_vertex to OpenGL
      water->add_attribute(attribute_pos, 3, GL_FLOAT, 0);
      water->add_attribute(attribute_normal, 3, GL_FLOAT, 0);
      water->add_attribute(attribute_color, 4, GL_UNSIGNED_BYTE, 12, GL_TRUE);

      //generate our default waves ->> reading in first text file with default params
      generate_waves();

      scene_node *node = new scene_node();
      node->rotate(90.0f, vec3(1.0, 0.0f, 0.0f)); //need to rotate it to be forward facing
      //add the mesh to the scene
      the_app->add_mesh_instance(new mesh_instance(node, water, water_material));
    }


    //need to update the points each frame
    void update(){

      ++time_step; //update our time step

      // these write-only locks give access to the vertices and indices.
      // they will be released at the next } (the end of the scope)
      gl_resource::wolock vl(water->get_vertices());
      my_vertex *vtx = (my_vertex *)vl.u8();
      gl_resource::wolock il(water->get_indices());
      uint32_t *idx = il.u32();

      // make the vertices
      for (size_t i = 0; i != mesh_size; ++i) {
        for (size_t j = 0; j != mesh_size; ++j) {
          vec3 wavePosition = gerstner_wave_function(j, i);
          vtx->pos = vec3p(vec3(1.0f * j, -1.0f * i, 0.0f) + wavePosition);
          //vtx->color = (0.0f, 0.3f, 1.0f);
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

    //just to clean up before we load a new file
    void open_file(std::string txtfile){
      std::ifstream file(txtfile);
      std::string config_name;
      //if exists
      if (file.is_open()){
        printf("\nreading in wave configuration file...\n");
        std::getline(file, config_name);

        std::string newline;
        while (std::getline(file, newline)){

          if (newline.compare("Amplitude:") == 0){
            std::getline(file, newline);
            ampli_ = std::stof(newline); //convert string to float
            printf("Amplitude: %f\n", ampli_);
          }
          if (newline.compare("Frequency:") == 0){
            std::getline(file, newline);
            freq_ = std::stof(newline); //string to float
            printf("Frequency: %f\n", freq_);
          }
          if (newline.compare("Speed:") == 0){
            std::getline(file, newline);
            speed_ = std::stof(newline); //string to float
            printf("Speed: %f\n", speed_);
          }
          if (newline.compare("Steepness:") == 0){
            std::getline(file, newline);
            steepness_ = std::stof(newline); //string to float
            printf("Steepness: %f\n", steepness_);
          }
        }
        file.close(); //we don't need the file anymore
        printf("%s file has been read\n", config_name.c_str()); //print the name of the file
      }
      else{
        printf("failed to load configuration file...using default params\n");
        //basic default parameters for our sine wave (smooth waves)
        freq_ = 0.4f;
        ampli_ = 0.68f;
        speed_ = 0.3f;
        steepness_ = 1.0f; //max steepness or we'll see some looping (remember that)
      }

      for (unsigned i = 0; i < sine_waves.size(); ++i){
        sine_waves[i].amplitude = ampli_;
        sine_waves[i].frequency = freq_;
        sine_waves[i].speed = speed_;
        sine_waves[i].steepness = steepness_;
      }
     
    }

    //dont need this visible at all times (pragma region)
#pragma region input_functions
    void increment_freq(){
      for (int i = 0; i < sine_waves.size(); ++i){
        sine_waves[i].frequency += 0.01f;

      }
      printf("Frequency: %f\n", sine_waves[0].frequency);
    }
    void decrement_freq(){
      for (int i = 0; i < sine_waves.size(); ++i){
        sine_waves[i].frequency -= 0.01f;
      }
      printf("Frequency: %f\n", sine_waves[0].frequency);
    }
    void increment_ampli(){
      for (int i = 0; i < sine_waves.size(); ++i){
        sine_waves[i].amplitude += 0.01f;
      }
      printf("Amplitude: %f\n", sine_waves[0].amplitude);
    }
    void decrement_ampli(){
      for (int i = 0; i < sine_waves.size(); ++i){
        sine_waves[i].amplitude -= 0.01f;
      }
      printf("Amplitude: %f\n", sine_waves[0].amplitude);
    }
    void increment_speed(){
      for (int i = 0; i < sine_waves.size(); ++i){
        sine_waves[i].speed += 0.01f;
      }
      printf("Speed: %f\n", sine_waves[0].speed);
    }
    void decrement_speed(){
      for (int i = 0; i < sine_waves.size(); ++i){
        sine_waves[i].speed -= 0.01f;
      }
      printf("Speed: %f\n", sine_waves[0].speed);
    }
    void increment_steepness(){
      for (int i = 0; i < sine_waves.size(); ++i){
        sine_waves[i].steepness += 0.01f;
      }
      printf("Steepness: %f\n", sine_waves[0].steepness);
    }
    void decrement_steepness(){
      for (int i = 0; i < sine_waves.size(); ++i){
        sine_waves[i].steepness -= 0.01f;
      }
      printf("Steepness: %f\n", sine_waves[0].steepness);
    }
    void wireframe_mode_on(){
      water->set_mode(1);
      printf("Wireframe mode ON\n");
    }
    void wireframe_mode_off(){
      water->set_mode(4);
      printf("Wireframe mode OFF\n");
    }
#pragma endregion
  };
}
#endif