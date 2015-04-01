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

    inputs inputs;
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

      inputs.init(this);

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
      updateInputsAndUI(vx, vy);

      keyboard_inputs();
    }

    void keyboard_inputs(){
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
      if (is_key_down('5')){
        wave_geometry->increment_omega();
      }
      if (is_key_down('6')){
        wave_geometry->decrement_omega();
      }
      if (is_key_down('S') && is_key_down(key_up)){
        wave_geometry->increment_speed();
      }
      if (is_key_down('S') && is_key_down(key_down)){
        wave_geometry->decrement_speed();
      }
      if (is_key_going_down('7')){
        wave_geometry->wireframe_mode_on();
      }
      if (is_key_going_down('8')){
        wave_geometry->wireframe_mode_off();
      }
    }

    void updateInputsAndUI(int vx, int vy){
      mat4t &camera = app_scene->get_camera_instance(0)->get_node()->access_nodeToParent();
      //run key_presses loop to check for inputs
      inputs.key_presses(camera);
      inputs.mouse_control(camera);
      Game_UI.updateUI(vx, vy);
      Game_UI.pop_up_clear();
    }

  };
}
#endif