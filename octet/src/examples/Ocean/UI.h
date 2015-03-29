#ifndef UI_INCLUDED
#define UI_H_INCLUDED

namespace octet {
  
  class UI : public resource{
    
    text_overlay *overlay;
    /** @var  text_overlay *overlay
    *   @brief text overlay for the UI.
    */
    mesh_text *UI_top;
    /** @var   mesh_text *UI_top
    *   @brief text mesh for the top UI.
    */
    mesh_text *UI_bot;
    /** @var   mesh_text *UI_bot
    *   @brief text mesh for the bottom UI.
    */
    mesh_text *UI_popup;
    /** @var  mesh_text *UI_popup
    *   @brief text mesh for the popup UI.
    */
    mesh_text *target;
    /** @var  mesh_text *target
    *   @brief text mesh for the target UI.
    */    
  
  public:
    int pop_up_var = 0;
    bool clear_popup = false;
    float temp_popup_time;
    float total_popup_time;
    int terrain_type = 1;
 
    float water_pert_to = 0.5f;
    float ice_pert_from = 0.85f;    
    float grass_pert_from =0.5f;
    float mountain_pert_from= 0.7f;
    int selected_attrib = 0;   
     
    UI() {
    }   
    void initUI(int vx , int vy )
    {     
      printf(" x %i , y %i", vx, vy);
      overlay = new text_overlay();
      // get the defualt font.
      bitmap_font *font = overlay->get_default_font();
      // create a box containing text (in pixels)
      aabb bb_bot(vec3(0, -550, 0.0f), vec3(vx, 200.0f, 0.0f));
      UI_bot = new mesh_text(font, "", &bb_bot);

      aabb bb_top(vec3(vx * 3 / 8, 200, 0.0f), vec3(vx, 200.0f, 0.0f));
      UI_top = new mesh_text(font, "", &bb_top);

      aabb bb_popup(vec3(vx * 7 / 8, -240, 0.0f), vec3(vx, 200.0f, 0.0f));
      UI_popup = new mesh_text(font, "", &bb_popup);

      aabb bb_target(vec3(vx, -100, 0.0f), vec3(vx, 100.0f, 0.0f));
      target = new mesh_text(font, "+", &bb_target);
      // add the mesh to the overlay.

      overlay->add_mesh_text(UI_bot);
      overlay->add_mesh_text(UI_top);
      overlay->add_mesh_text(UI_popup);
      overlay->add_mesh_text(target);
    }
    /** @fn  void initUI()
    *   @brief This function is used to init the UI variables ie overlay textmesh....
    */  

    void setup_pop_up(int num)
    {
      total_popup_time = 2.0f;
      temp_popup_time = 0.0f;
      pop_up_var = num;
      clear_popup = true;
    }
    void pop_up_clear()
    {
      if (clear_popup)
      {
        if (total_popup_time - temp_popup_time <= 0)
        {
          clear_popup = false;
          pop_up_var = 0;
        }
        else
        {
          temp_popup_time += 0.03f;
        }
      }
    }
    void updateUI(int vx, int vy)
    {
      string current_element;
      switch (terrain_type)
      {
      case 0: current_element = "Random Terrain";
        break;

      case 1: current_element = "Example Heightmap";
        break;

      case 2: current_element = "Heightmap Based On Ken Perlin's Default Permutation Table";
        break;
      }
      UI_bot->clear();
      UI_bot->format(
        //"Current Terrain Type : %s   Press G to regenerate, Press Q To Generate Random Terrain, Press R To Generate Example Heightmap, Press P To Generate Heightmap Based On Ken Perlin's Default Permutation Table", current_element);
      "Current Terrain Type : %s   Press Q to generate a random height map!", current_element);
      // convert it to a mesh.
      UI_bot->update();


      string current_attrib;
      switch (selected_attrib)
      {
      case 0: current_attrib = "ICE From";
        break;      
      case 1: current_attrib = "Mountain From";
        break;      
      case 2: current_attrib = "Grass From";
        break;
      case 3:current_attrib = "Water To";
        break;
      }
      UI_top->clear();
      UI_top->format(
        "Current Attrib : %s  Ice From :%f  Mountain From :%f Grass From :%f Water To: %f ", current_attrib, ice_pert_from, mountain_pert_from, grass_pert_from, water_pert_to);
      // convert it to a mesh.
      UI_top->update();


      UI_popup->clear();
      switch (pop_up_var)
      {
      case 0:UI_popup->format("");
        break;
      case 1:UI_popup->format("Please wait...generating random terrain. This may take a while.");

        break;
      case 2:UI_popup->format("Generating an example heightmap.");

        break;
      case 3:UI_popup->format("Generating a heightmap based on Ken Perlin's default permutation table.");

        break;
      case 4:UI_popup->format("Terrain successfully generated.");
        break;
      }
      UI_popup->update();
      // draw the text overlay
      overlay->render(vx, vy);
    } 
  
  };  
  }
#endif