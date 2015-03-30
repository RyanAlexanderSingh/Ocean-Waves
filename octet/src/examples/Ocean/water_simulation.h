////////////////////////////////////////////////////////////////////////////////
//
// (C) Ryan Singh
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
#include <math.h>

#ifndef WATER_SIMULATION_H_INCLUDED
#define WATER_SIMULATION_H_INCLUDED

#define PI 3.14159265358979323846f  /* pi */

namespace octet {

  /// Scene containing a box with octet.
  class water_simulation : public app {

    inputs inputs;
    UI Game_UI;

    // scene for drawing box
    ref<visual_scene> app_scene;
    ref<wave_mesh> wave_geometry;

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

    static uint32_t make_color(float r, float g, float b) {
      return 0xff000000 + ((int)(r*255.0f) << 0) + ((int)(g*255.0f) << 8) + ((int)(b*255.0f) << 16);
    }

    void setup_camera()
    {
      mat4t &camera_mat = app_scene->get_camera_instance(0)->get_node()->access_nodeToParent();
      camera_mat.translate(0, 150, 150);
      //camera_mat.rotateY(90);
      camera_mat.rotateX(-30);
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

      wave_geometry = new wave_mesh();
      wave_geometry->init();

      mat4t mat;
      mat.loadIdentity();
      mat.translate(0, -0.5f, 0);

      //example terrain geometry
      app_scene->add_shape(
        mat,
        new mesh_terrain(vec3(100.0f, 0.5f, 100.0f), ivec3(100, 1, 100), source),
        new material(new image("assets/grass.jpg")),
        false, 0
        );
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

      //update the geometry
      wave_geometry->update();

      //keep this out of the way -> updates the inputs and the UI
      updateInputsAndUI(vx, vy);
    }

    void updateInputsAndUI(int vx, int vy){
      mat4t &camera = app_scene->get_camera_instance(0)->get_node()->access_nodeToParent();
      //run key_presses loop to check for inputs
      inputs.key_presses(camera);
      //inputs.mouse_control(camera);
      Game_UI.updateUI(vx, vy);
      Game_UI.pop_up_clear();
    }

  };
}
#endif