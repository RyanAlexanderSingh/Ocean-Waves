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

    UI Game_UI;

    // scene for drawing box
    ref<visual_scene> app_scene;
    ref<wave_mesh> wave_geometry;

    void setup_camera()
    {
      mat4t &camera_mat = app_scene->get_camera_instance(0)->get_node()->access_nodeToParent();
      camera_mat.translate(0, 150, 0);
    }

  public:
    /// this is called when we construct the class before everything is initialised.
    water_simulation(int argc, char **argv) : app(argc, argv) {
    }

    /// this is called once OpenGL is initialized
    void app_init() {
      app_scene = new visual_scene();
      app_scene->create_default_camera_and_lights();
      app_scene->get_camera_instance(0)->set_far_plane(10000);
      setup_camera();

      int vx = 0, vy = 0;
      get_viewport_size(vx, vy);
      Game_UI.initUI(vx, vy);

      //create our wave geometry object
      wave_geometry = new wave_mesh();
      wave_geometry->init(app_scene);
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
      updateUI(vx, vy);
      keyboard_inputs(camera);
      mouse_inputs(camera);
    }

    #pragma region inputs
    void keyboard_inputs(mat4t &camera){

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
        camera.translate(0, 0, -5);
      }
      if (is_key_down(key::key_shift) && is_key_down('S'))
      {
        camera.translate(0, 0, 5);
      }
      if (is_key_down(key::key_shift) && is_key_down('A'))
      {
        camera.translate(-5, 0, 0);
      }
      if (is_key_down(key::key_shift) && is_key_down('D'))
      {
        camera.translate(5, 0, 0);
      }
    }

    void updateUI(int vx, int vy){
      Game_UI.updateUI(vx, vy);
      Game_UI.pop_up_clear();
    }

    void mouse_inputs(mat4t &camera){
      //mouse control using x and y pos of mouse
      int x, y;
      get_mouse_pos(x, y);
      int vx, vy;
      get_viewport_size(vx, vy);

      mat4t modelToWorld;

      modelToWorld.loadIdentity();
      modelToWorld[3] = vec4(camera.w().x(), camera.w().y(), camera.w().z(), 1);
      modelToWorld.rotateY((float)-x*2.0f);
      if (vy / 2 - y < 70 && vy / 2 - y > -70)
        modelToWorld.rotateX((float)vy / 2 - y);
      if (vy / 2 - y >= 70)
        modelToWorld.rotateX(70);
      if (vy / 2 - y <= -70)
        modelToWorld.rotateX(-70);
      camera = modelToWorld;//apply to the node
    }

    #pragma endregion

  };
}
#endif