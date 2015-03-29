////////////////////////////////////////////////////////////////////////////////
//
// (C) Ryan Singh and Himanshu Chablani
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//

#ifndef TERRAIN_GEN_H_INCLUDED
#define TERRAIN_GEN_H_INCLUDED

#include<iostream>
#include<fstream>

namespace octet {

  /// Scene containing a box with octet.
  class terrain_gen : public app {

    //bmp_image img;
    perlin pn;
    inputs inputs;
    bmp_image img_gen;
    UI Game_UI;
    sky_box Sky_Box;


    // scene for drawing box
    ref<visual_scene> app_scene;

    struct my_vertex {
      vec3p pos;
      vec3p nor;
      uint32_t color;
    };
    static uint32_t make_color(float r, float g, float b) {
      return 0xff000000 + ((int)(r*255.0f) << 0) + ((int)(g*255.0f) << 8) + ((int)(b*255.0f) << 16);
    }
    void setup_camera()
    {
      mat4t &camera_mat = app_scene->get_camera_instance(0)->get_node()->access_nodeToParent();
      camera_mat.translate(150, 150, 150);
      camera_mat.rotateY(-90);
      camera_mat.rotateX(0);
    }
    void input_keys()
    {
      if (inputs.Q_KEY()){
        Game_UI.setup_pop_up(1);
        Game_UI.terrain_type = 0;
        app_scene->pop_mesh_instance();
        generate(false, true);
        Game_UI.setup_pop_up(4);
      }
      //if (inputs.R_KEY()){
      //  Game_UI.setup_pop_up(2);
      //  Game_UI.terrain_type = 1;
      //  app_scene->pop_mesh_instance();
      //  generate(true, false);
      //  Game_UI.setup_pop_up(4);
      //}
      //if (inputs.P_KEY()){
      //  Game_UI.setup_pop_up(3);
      //  Game_UI.terrain_type = 2;
      //  app_scene->pop_mesh_instance();
      //  generate(false, false);
      //  Game_UI.setup_pop_up(4);
      //}

      if (inputs.G_KEY())
      {
        switch (Game_UI.terrain_type)
        {
        case 0:
          Game_UI.setup_pop_up(1);
          Game_UI.terrain_type = 0;
          app_scene->pop_mesh_instance();
          generate(false, true);
          Game_UI.setup_pop_up(4);
          break;
        case  1:
          Game_UI.setup_pop_up(2);
          Game_UI.terrain_type = 1;
          app_scene->pop_mesh_instance();
          generate(true, false);
          Game_UI.setup_pop_up(4);
          break;
        case  2:
          Game_UI.setup_pop_up(3);
          Game_UI.terrain_type = 2;
          app_scene->pop_mesh_instance();
          generate(false, false);
          Game_UI.setup_pop_up(4);
          break;
        }

      }

      if (is_key_going_up(key::key_right))
      {
        if (Game_UI.selected_attrib == 3)
          Game_UI.selected_attrib = 0;
        else
          Game_UI.selected_attrib++;
      }
      if (is_key_going_up(key::key_left))
      {
        if (Game_UI.selected_attrib == 0)
          Game_UI.selected_attrib = 3;
        else
          Game_UI.selected_attrib--;
      }

    }
  public:
    /// this is called when we construct the class before everything is initialised.
    terrain_gen(int argc, char **argv) : app(argc, argv) {
    }
    /// this is called once OpenGL is initialized
    void app_init() {
      app_scene = new visual_scene();
      app_scene->create_default_camera_and_lights();
      app_scene->get_camera_instance(0)->set_far_plane(10000);
      setup_camera();
      Sky_Box.InitDayNightCycle(app_scene);

      int vx = 0, vy = 0;
      get_viewport_size(vx, vy);
      Game_UI.initUI(vx, vy);

      inputs.init(this);
      img_gen.init();
      generate(false, false);
    }
    /// this is called to draw the world
    void draw_world(int x, int y, int w, int h) {
      int vx = 0, vy = 0;
      get_viewport_size(vx, vy);
      app_scene->begin_render(vx, vy);
      // update matrices. assume 30 fps.
      app_scene->update(1.0f / 30);
      // draw the scene
      app_scene->render((float)vx / vy);

      mat4t &camera = app_scene->get_camera_instance(0)->get_node()->access_nodeToParent();
      //run key_presses loop to check for inputs
      inputs.key_presses(camera);
      inputs.mouse_control(camera);

      Sky_Box.DayNightCycle(app_scene);
      Game_UI.updateUI(vx, vy);
      Game_UI.pop_up_clear();
      input_keys();
    }
    void make_bmp_file(ofstream *s)
    {
      s->put(char(66));//b
      s->put(char(77));//m
      for (int i = 0; i < 8; i++){
        s->put(char(0));//8 spaces

      }
      s->put(char(54));//6

      for (int i = 0; i < 3; i++){
        s->put(char(0));//3 spaces
      }
      s->put(char(40));//(
      for (int i = 0; i < 3; i++){
        s->put(char(0));//3 spaces
      }
      s->put(char(height_image % 256));
      s->put(char((height_image >> 8) % 256));
      s->put(char((height_image >> 16) % 256));
      s->put(char((height_image >> 24) % 256));
      s->put(char(width_image % 256));
      s->put(char((width_image >> 8) % 256));
      s->put(char((width_image >> 16) % 256));
      s->put(char((width_image >> 24) % 256));
      s->put(char(1));//start of header
      s->put(char(0));//space
      s->put(char(24));//cancle 
      for (int i = 0; i < 25; i++){
        s->put(char(0));//25 spaces
      }
    }
    void generate(bool from_image, bool random_seed){

      //the mesh generatiion
      param_shader *shader = new param_shader("shaders/default.vs", "shaders/simple_color.fs");
      material *red = new material(vec4(0.6f, 0.298f, 0.0f, 1.0f), shader);
      mesh *terrain = new mesh();
      // allocate vertices and indices into OpenGL buffers
      size_t num_vertices = height_image * width_image;
      size_t num_indices = 6 * (num_vertices - height_image) - width_image;
      terrain->allocate(sizeof(my_vertex)* num_vertices, sizeof(uint32_t)* num_indices);
      terrain->set_params(sizeof(my_vertex), num_indices, num_vertices, GL_TRIANGLES, GL_UNSIGNED_INT);
      // describe the structure of my_vertex to OpenGL
      terrain->add_attribute(attribute_pos, 3, GL_FLOAT, 0);
      terrain->add_attribute(attribute_normal, 3, GL_FLOAT, 12);
      terrain->add_attribute(attribute_color, 4, GL_UNSIGNED_BYTE, 24, GL_TRUE);
      gl_resource::wolock vl(terrain->get_vertices());
      my_vertex *vtx = (my_vertex *)vl.u8();
      gl_resource::wolock il(terrain->get_indices());
      uint32_t *idx = idx = il.u32();
      //end of init


      float min = 0.0f, max = 0.0f;
      float *image = new float[height_image*width_image];
      ofstream grey;
      ofstream s;
      ifstream grey_read("Height_Map.bmp", std::ios::in | std::ios::binary);

      if (!from_image)
      {
      pn.fill_image(image, min, max, 16);      
      s.open("colour_map.bmp", ofstream::binary);
      if (!(s.is_open())){
        printf("Cannot open the file...");
        exit(0);
      }
      make_bmp_file(&s);
      
      grey.open("perlin_noise_grey.bmp", ofstream::binary);
      if (!(grey.is_open())){
        printf("Cannot open the file...");
        exit(0);
      }
      make_bmp_file(&grey);
      }
      
      if (grey_read.is_open()&&from_image)
      {
        char *line = new char[54];
        grey_read.read(line,54);
        printf("The line was %s",line);
        if (line!="BM")
        {
        printf("The file is corrupt");        
       // exit(0);
        }
      }
      // Visit every pixel of the image and assign a color generated with Perlin noise
      for (int i = 0; i < height_image; ++i) {     // z
        for (int j = 0; j < width_image; ++j) {  // x
          if (!from_image)
          {
            float vert_height = *(image + i*width_image + j)*100.0f;
            vtx->pos = vec3p((float)j, vert_height, (float)i);
            vtx->nor = vec3p((float)j, vert_height, (float)i);
            vec3 mesh_colour = img_gen.create_colour(*(image + i*width_image + j), min, max);
            vtx->color = make_color(mesh_colour.x(), mesh_colour.y(), mesh_colour.z());
            s.put(char(mesh_colour.z() * 255));
            s.put(char(mesh_colour.y() * 255));
            s.put(char(mesh_colour.x() * 255));
            grey.put(char(vert_height * 255 / 100));
            grey.put(char(vert_height * 255 / 100));
            grey.put(char(vert_height * 255 / 100));
          }
          else
          {
            char aux;
            grey_read.read(&aux,1);
            grey_read.read(&aux, 1);
            grey_read.read(&aux, 1);

            float vert_height = (float)((aux*100.0f)/255.0f);
            vtx->pos = vec3p((float)j, vert_height, (float)i);
            vtx->nor = vec3p((float)j, vert_height, (float)i);
            vec3 mesh_colour = img_gen.create_colour(*(image + i*width_image + j), min, max);
            vtx->color = make_color(mesh_colour.x(), mesh_colour.y(), mesh_colour.z());
          }
          vtx++;
        }
      }
      //make triangles          
      int tempvert = 0;
      for (int i = 0; i < height_image - 1; ++i) {
        for (int j = 0; j < width_image - 1; ++j) {

          idx[0] = tempvert;
          idx[1] = tempvert + 1;
          idx[2] = tempvert + width_image + 1;
          idx += 3;

          idx[0] = tempvert;
          idx[1] = tempvert + width_image + 1;
          idx[2] = tempvert + width_image;
          idx += 3;

          tempvert++;
        }
        tempvert++;
      }

      scene_node *node = new scene_node();
      app_scene->add_child(node);
      app_scene->add_mesh_instance(new mesh_instance(node, terrain, red));
      s.close();
      grey.close();
      delete[]image;
    }


  };
}
#endif