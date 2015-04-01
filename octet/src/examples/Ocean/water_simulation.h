////////////////////////////////////////////////////////////////////////////////
//
// (C) Ryan Singh
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
#include <math.h>

#ifndef WATER_SIMULATION_H_INCLUDED
#define WATER_SIMULATION_H_INCLUDED

namespace octet {

  /// Scene containing a box with octet.
  class water_simulation : public app {

    // scene for drawing box
    ref<visual_scene> app_scene;
    ref<wave_mesh> wave_geometry;
    ref<camera_instance> camera;

    //only using this for the skybox (think about moving this)
    collada_builder loader;

    void create_skybox(){

      mesh_sphere *skybox = new mesh_sphere(vec3(0, 0, 0), 1000);
      material *mat = new material(new image("assets/skybox.jpg"));
      mat4t location;
      app_scene->add_shape(location, skybox, mat, false);
    }

  public:
    /// this is called when we construct the class before everything is initialised.
    water_simulation(int argc, char **argv) : app(argc, argv) {
    }

    /// this is called once OpenGL is initialized
    void app_init() {
      app_scene = new visual_scene();
      app_scene->set_world_gravity(btVector3(0, 0, 0));

      //this one works 
      light *_light = new light();
      light_instance *li = new light_instance();
      scene_node *node = new scene_node();
      app_scene->add_child(node);
      node->translate(vec3(-100, 100, -100));
      node->rotate(45, vec3(1, 0, 0));
      node->rotate(180, vec3(0, 1, 0));
      _light->set_color(vec4(1, 1, 1, 1));
      _light->set_kind(atom_directional);
      li->set_node(node);
      li->set_light(_light);
      app_scene->add_light_instance(li);

      //working on this one

      app_scene->create_default_camera_and_lights();
      app_scene->get_camera_instance(0)->set_far_plane(10000);
      camera = app_scene->get_camera_instance(0);
      camera->get_node()->access_nodeToParent().translate(0, 150, 0);

      int vx = 0, vy = 0;
      get_viewport_size(vx, vy);

      //create our wave geometry object
      wave_geometry = new wave_mesh();
      wave_geometry->init(app_scene);

      create_skybox();
    }

    /// this is called to draw the world
    void draw_world(int x, int y, int w, int h) {
      int vx = 0, vy = 0;
      get_viewport_size(vx, vy);
      app_scene->begin_render(vx, vy);

      //update the geometry
      wave_geometry->update();

      // update matrices. assume 30 fps.
      app_scene->update(1.0f / 30);
      // draw the scene
      app_scene->render((float)vx / vy);

      //keep this out of the way -> updates the inputs and the UI
      mat4t &camera = app_scene->get_camera_instance(0)->get_node()->access_nodeToParent();
      keyboard_inputs();
      mouse_inputs();

    }

#pragma region inputs
    void keyboard_inputs(){

      //1-4 number keys on the keyboard
      for (int i = 1; i <= 3; ++i){
        char c = i + '0'; //ascii value
        if (is_key_going_down(c)){
          std::string txtfile = "../../../assets/wave_configs/wave_confign.txt";
          txtfile.replace(40, 1, std::to_string(i)); //just inject the number of the key into the string and read that text file
          wave_geometry->open_file(txtfile); //load up text file
        }
      }

      if (is_key_down('F') && is_key_down(key_up)){
        wave_geometry->increment_freq();
      }
      if (is_key_down('F') && is_key_down(key_down)){
        wave_geometry->decrement_freq();
      }
      if (is_key_down('A') && is_key_down(key_up)){
        wave_geometry->increment_ampli();
      }
      if (is_key_down('A') && is_key_down(key_down)){
        wave_geometry->decrement_ampli();
      }
      if (is_key_down('S') && is_key_down(key_up)){
        wave_geometry->increment_speed();
      }
      if (is_key_down('S') && is_key_down(key_down)){
        wave_geometry->decrement_speed();
      }
      if (is_key_down('Q') && is_key_down(key_up)){
        wave_geometry->increment_steepness();
      }
      if (is_key_down('Q') && is_key_down(key_down)){
        wave_geometry->decrement_steepness();
      }
      if (is_key_going_down('7')){
        wave_geometry->wireframe_mode_on();
      }
      if (is_key_going_down('8')){
        wave_geometry->wireframe_mode_off();
      }

      if (is_key_down(key_esc)){
        exit(0);
      }

      if (is_key_down(key::key_shift) && is_key_down('W'))
      {
        camera->get_node()->access_nodeToParent().translate(0, 0, -5);
      }
      if (is_key_down(key::key_shift) && is_key_down('S'))
      {
        camera->get_node()->access_nodeToParent().translate(0, 0, 5);
      }
      if (is_key_down(key::key_shift) && is_key_down('A'))
      {
        camera->get_node()->access_nodeToParent().translate(-5, 0, 0);
      }
      if (is_key_down(key::key_shift) && is_key_down('D'))
      {
        camera->get_node()->access_nodeToParent().translate(5, 0, 0);
      }
    }

    void mouse_inputs(){
      //mouse control using x and y pos of mouse
      int x, y;
      get_mouse_pos(x, y);
      int vx, vy;
      get_viewport_size(vx, vy);

      mat4t modelToWorld;

      mat4t &camera_mat = camera->get_node()->access_nodeToParent();
      modelToWorld.loadIdentity();
      modelToWorld[3] = vec4(camera_mat.w().x(), camera_mat.w().y(), camera_mat.w().z(), 1);
      modelToWorld.rotateY((float)-x*2.0f);
      if (vy / 2 - y < 70 && vy / 2 - y > -70)
        modelToWorld.rotateX((float)vy / 2 - y);
      if (vy / 2 - y >= 70)
        modelToWorld.rotateX(70);
      if (vy / 2 - y <= -70)
        modelToWorld.rotateX(-70);
      camera_mat = modelToWorld;//apply to the node
    }

#pragma endregion

  };
}
#endif