#include "gui_use.h"
#include "gui_lay.h"
#include "gui_ltype.h"
#include "gui_file.h"
#include "gui_xy.h"
#include "gui_script.h"

extern const char *dxf_seed_2007;

int gui_tools_win (gui_obj *gui){

	if (nk_begin_titled(gui->ctx, "contextual_tools", gui->ctx_tools_title,
    nk_rect(gui->next_win_x, gui->next_win_y, gui->next_win_w, gui->next_win_h),
		NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
		NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE)){
    
      /*
		static enum Tool_group {
			GRP_PLACE,
			GRP_MODIFY,
			GRP_DIM,
			GRP_XDATA,
		} tool_grp = GRP_PLACE;
    */
		
		//nk_layout_row_static(gui->ctx, 28, 28, 6);
		
		
		
    #if(0)
		nk_layout_row_dynamic(gui->ctx, 10, 1); /*space*/
			
		/* Tools collections - Place, Modify and Dimensions */
		nk_style_push_vec2(gui->ctx, &gui->ctx->style.window.spacing, nk_vec2(0,5));
		nk_layout_row_begin(gui->ctx, NK_STATIC, 20, 4);
		if (gui_tab (gui, _l("Place"), tool_grp == GRP_PLACE)) tool_grp = GRP_PLACE;
		if (gui_tab (gui, _l("Modify"), tool_grp == GRP_MODIFY)) tool_grp = GRP_MODIFY;
		if (gui_tab (gui, _l("Dimensions"), tool_grp == GRP_DIM)) tool_grp = GRP_DIM;
		//if (gui_tab (gui, _l("XData"), too_grp == GRP_XDATA)) too_grp = GRP_XDATA;
		nk_style_pop_vec2(gui->ctx);
		nk_layout_row_end(gui->ctx);
		
				
		nk_layout_row_dynamic(gui->ctx, 10, 1); /*space*/
		
		struct nk_vec2 wid_pos = nk_widget_position(gui->ctx);
		struct nk_vec2 win_pos = nk_window_get_position(gui->ctx);
		struct nk_rect win_cont = nk_window_get_content_region(gui->ctx);
	
		nk_layout_row_dynamic(gui->ctx, win_cont.h - (wid_pos.y - win_pos.y), 1);
		if (nk_group_begin(gui->ctx, "especific", NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {
    #endif
			gui_select_info (gui);
			gui_line_info (gui);
			gui_pline_info (gui);
			gui_circle_info (gui);
			gui_rect_info (gui);
			gui_text_info (gui);
			gui_mtext_info (gui);
			gui_move_info (gui);
			gui_dupli_info (gui);
			gui_scale_info (gui);
			gui_rotate_info (gui);
			gui_mirror_info (gui);
			gui_insert_info (gui);
			gui_block_info (gui);
			gui_hatch_info (gui);
			gui_paste_info (gui);
			gui_ed_text_info (gui);
			gui_spline_info (gui);
			gui_point_info (gui);
			gui_ellip_info (gui);
			gui_image_info (gui);
			
			gui_ed_attr_info (gui);
			gui_attrib_info (gui);
			gui_expl_info (gui);
			gui_measure_info (gui);
			gui_find_info (gui);
			gui_prop_info (gui);
			gui_txt_prop_info (gui);
			gui_vertex_info (gui);
			
			gui_dim_linear_info (gui);
			gui_dim_angular_info (gui);
			gui_dim_radial_info (gui);
			gui_dim_ordinate_info (gui);
			
			gui_zoom_info (gui);
      gui_view_info (gui);
			
			/* execute scripts in dynamic mode*/
			gui_script_dyn(gui);
      
      gui_sphere_info (gui);
      gui_cylinder_info (gui);
      gui_cone_info (gui);
      gui_wedge_info (gui);
      gui_torus_info (gui);
      gui_pyramid_info (gui);
      gui_slab_info (gui);
      gui_slice_info (gui);
      gui_rotate_3d_info (gui);
      gui_extrude_info (gui);
      gui_extrude_path_info (gui);
      gui_revolve_info (gui);
      gui_union_info (gui);
      gui_subtract_info (gui);
      gui_intersection_info (gui);
			
			//nk_group_end(gui->ctx);
		//}
	}
	nk_end(gui->ctx);
	return 1;
}

int gui_main_win(gui_obj *gui){
	int i;
  
  static enum Ribbon_group {
    RIB_FILE,
    RIB_DRAWING,
    RIB_VIEW,
    RIB_MANAGER,
    RIB_3D,
  } ribbon_grp = RIB_DRAWING;
  
  const int rib_w[5] = {800, 1000, 1250, 1400, 1600};
	
	if (nk_begin(gui->ctx, "Main", nk_rect(2, 2, gui->win_w - 4, RIB_H - 5),
    NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)){
    /* dynamic width for ribbon and fixed width for common tools and properties */
		nk_layout_row_template_begin(gui->ctx, 148);
		nk_layout_row_template_push_dynamic(gui->ctx); /* ribbon */
    nk_layout_row_template_push_static(gui->ctx, 120); /* common tools */
		nk_layout_row_template_push_static(gui->ctx, 400); /* properties */
		nk_layout_row_template_end(gui->ctx);
    
    
    if (nk_group_begin(gui->ctx, "Main_Ribbon", NK_WINDOW_BORDER)) {
      
      /* Ribbon collections */
      nk_style_push_vec2(gui->ctx, &gui->ctx->style.window.spacing, nk_vec2(0,5));
      nk_layout_row_begin(gui->ctx, NK_STATIC, 20, 5);
      
      if (gui_tab (gui, _l("File"), ribbon_grp == RIB_FILE)) ribbon_grp = RIB_FILE;
      
      if (gui->win_w > rib_w[RIB_VIEW]) {
        if (gui_tab (gui, _l("Home"), ribbon_grp == RIB_DRAWING)) ribbon_grp = RIB_DRAWING;
      }
      else {
        if (gui_tab (gui, _l("Drawing"), ribbon_grp == RIB_DRAWING)) ribbon_grp = RIB_DRAWING;
        if (gui_tab (gui, _l("View"), ribbon_grp == RIB_VIEW)) ribbon_grp = RIB_VIEW;
      }
      if (gui->win_w < rib_w[RIB_MANAGER]) {
        if (gui_tab (gui, _l("Managers"), ribbon_grp == RIB_MANAGER)) ribbon_grp = RIB_MANAGER;
      }
      if (gui->win_w < rib_w[RIB_3D]) {
        if (gui_tab (gui, _l("3D"), ribbon_grp == RIB_3D)) ribbon_grp = RIB_3D;
      }
      nk_style_pop_vec2(gui->ctx);
      nk_layout_row_end(gui->ctx);
      
      /* first line */
      //nk_layout_row_begin(gui->ctx, NK_STATIC, ICON_SIZE + 12, 9);
      nk_layout_row_begin(gui->ctx, NK_STATIC, 102, 20);
      
      
      /* file tools*/
      if(ribbon_grp == RIB_FILE){ // || gui->win_w > rib_w[ribbon_grp]){
        //nk_layout_row_begin(gui->ctx, NK_STATIC, 102, 9);
        //nk_layout_row_static(gui->ctx, ICON_SIZE + 4, ICON_SIZE + 4, 14);
        nk_layout_row_push(gui->ctx, (ICON_SIZE + 4 + 4) + 13);
        if (nk_group_begin(gui->ctx, "_filetools", NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {
          nk_layout_row_static(gui->ctx, 28, 28, 1);
        
          if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_NEW]))){
            if (gui->changed){
              gui->discard_changes = 1;
              gui->desired_action = FILE_NEW;
              gui->hist_action = HIST_NONE;
            }
            else{
              gui->action = FILE_NEW;
              gui->hist_new = 0; /* not add to history entries */
            }
          }
          if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_OPEN]))){
            gui->action = FILE_OPEN;
            //gui->show_app_file = 1;
            gui->show_open = 1;
            
            gui->curr_path[0] = 0;
            
            gui->path_ok = 0;
            gui->hist_new = 1;
          }
          if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_SAVE]))){
            gui->action = FILE_SAVE;
            //gui->show_app_file = 1;
            gui->show_save = 1;
            gui->path_ok = 0;
            gui->hist_new = 1;
          }
          nk_group_end(gui->ctx);
        }
        /* print/export tools*/
        nk_layout_row_push(gui->ctx, (ICON_SIZE + 4 + 4) + 13);
        if (nk_group_begin(gui->ctx, "_exporttools", NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {
          nk_layout_row_static(gui->ctx, 28, 28, 1);
          //nk_label(gui->ctx, "  ", NK_TEXT_RIGHT); /* simple space */
          if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_PRINT]))){
            gui->show_print = 1;
          }
          if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_EXPORT]))){
            gui->show_export = 1;
          }
          
          nk_group_end(gui->ctx);
        }
      
        nk_layout_row_push(gui->ctx, (ICON_SIZE + 4 + 4) + 13);
        if (nk_group_begin(gui->ctx, "_configtools", NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {
          nk_layout_row_static(gui->ctx, 28, 28, 1);
          /* config tools*/
          if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_SCRIPT1]))){
            
            gui->show_script = 1;
          }
          if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_GEAR]))){
            gui->show_config = 1;
          }
          
          /* Help - about CadZinho*/
          if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_HELP]))){
            gui->show_app_about = 1;
          }
          
          nk_group_end(gui->ctx);
        }
      
        //nk_layout_row_end(gui->ctx);
      }
      
      if(ribbon_grp == RIB_DRAWING || (gui->win_w > rib_w[RIB_DRAWING] &&
        gui->win_w > rib_w[ribbon_grp] && ribbon_grp != RIB_FILE)){
        //nk_layout_row_begin(gui->ctx, NK_STATIC, 102, 9);
        
        /* file tools*/
        nk_layout_row_push(gui->ctx, 3*(ICON_SIZE + 4 + 4) + 13);
        if (nk_group_begin(gui->ctx, "_place2d", NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {
          nk_layout_row_static(gui->ctx, 28, 28, 3);
          if (gui_sel_b (gui, gui->svg_bmp[SVG_LINE], gui->modal == LINE)){
            gui->modal = LINE;
            strncpy(gui->ctx_tools_title, _l("Line"), DXF_MAX_CHARS);
            gui->step = 0;
          }
          if (gui_sel_b (gui, gui->svg_bmp[SVG_PLINE], gui->modal == POLYLINE)){
            gui->modal = POLYLINE;
            strncpy(gui->ctx_tools_title, _l("Polyline"), DXF_MAX_CHARS);
            gui->step = 0;
          }
          if (gui_sel_b (gui, gui->svg_bmp[SVG_SPLINE], gui->modal == SPLINE)){
            gui->modal = SPLINE;
            strncpy(gui->ctx_tools_title, _l("Spline"), DXF_MAX_CHARS);
            gui->step = 0;
          }
          
          if (gui_sel_b (gui, gui->svg_bmp[SVG_RECT], gui->modal == RECTANGLE)){
            gui->modal = RECTANGLE;
            strncpy(gui->ctx_tools_title, _l("Rectangle"), DXF_MAX_CHARS);
            gui->step = 0;
          }
          
          if (gui_sel_b (gui, gui->svg_bmp[SVG_CIRCLE], gui->modal == CIRCLE)){
            gui->modal = CIRCLE;
            strncpy(gui->ctx_tools_title, _l("Circle"), DXF_MAX_CHARS);
            gui->step = 0;
          }
          if (gui_sel_b (gui, gui->svg_bmp[SVG_ELIPSE], gui->modal == ELLIPSE)){
            gui->modal = ELLIPSE;
            strncpy(gui->ctx_tools_title, _l("Ellipse"), DXF_MAX_CHARS);
            gui->step = 0;
          }
          if (gui_sel_b (gui, gui->svg_bmp[SVG_POINT], gui->modal == SINGLE_POINT)){
            gui->modal = SINGLE_POINT;
            strncpy(gui->ctx_tools_title, _l("Single Point"), DXF_MAX_CHARS);
            gui->step = 0;
          }
          
          if (gui_sel_b (gui, gui->svg_bmp[SVG_HATCH], gui->modal == HATCH)){
            gui->modal = HATCH;
            strncpy(gui->ctx_tools_title, _l("Hatch"), DXF_MAX_CHARS);
            gui->step = 0;
          }
          
          if (gui_sel_b (gui, gui->svg_bmp[SVG_IMAGE], gui->modal == IMAGE)){
            gui->modal = IMAGE;
            strncpy(gui->ctx_tools_title, _l("Image"), DXF_MAX_CHARS);
            gui->step = 0;
          }
          
          nk_group_end(gui->ctx);
        }
        
        
        /* file tools*/
        nk_layout_row_push(gui->ctx, (ICON_SIZE + 4 + 4) + 13);
        if (nk_group_begin(gui->ctx, "_inserts", NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {
          nk_layout_row_static(gui->ctx, 28, 28, 1);
          
          if (gui_sel_b (gui, gui->svg_bmp[SVG_BOOK], gui->modal == INSERT)){
            gui->modal = INSERT;
            strncpy(gui->ctx_tools_title, _l("Insert"), DXF_MAX_CHARS);
            gui->step = 0;
          }
          
          if (gui_sel_b (gui, gui->svg_bmp[SVG_TAG], gui->modal == ADD_ATTRIB)){
            gui->modal = ADD_ATTRIB;
            strncpy(gui->ctx_tools_title, _l("Add Tag"), DXF_MAX_CHARS);
            gui->step = 0;
            sel_list_clear (gui);
          }
          if (gui_sel_b (gui, gui->svg_bmp[SVG_TAG_E], gui->modal == ED_ATTR)){
            gui->modal = ED_ATTR;
            strncpy(gui->ctx_tools_title, _l("Edit Tag"), DXF_MAX_CHARS);
            gui->step = 0;
            sel_list_clear (gui);
          }
          
          nk_group_end(gui->ctx);
        }
        
        
        /* file tools*/
        nk_layout_row_push(gui->ctx, 2*(ICON_SIZE + 4 + 4) + 13);
        if (nk_group_begin(gui->ctx, "_text", NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {
          nk_layout_row_static(gui->ctx, 28, 28, 2);
          if (gui_sel_b (gui, gui->svg_bmp[SVG_TEXT], gui->modal == TEXT)){
            gui->modal = TEXT;
            strncpy(gui->ctx_tools_title, _l("Text"), DXF_MAX_CHARS);
            gui->step = 0;
          }
          if (gui_sel_b (gui, gui->svg_bmp[SVG_I_TEXT], gui->modal == MTEXT)){
            gui->modal = MTEXT;
            strncpy(gui->ctx_tools_title, _l("Inteli Text"), DXF_MAX_CHARS);
            gui->step = 0;
          }
          if (gui_sel_b (gui, gui->svg_bmp[SVG_TEXT_E], gui->modal == ED_TEXT)){
            gui->modal = ED_TEXT;
            strncpy(gui->ctx_tools_title, _l("Edit Text"), DXF_MAX_CHARS);
            gui->step = 0;
            sel_list_clear (gui);
          }
          if (gui_sel_b (gui, gui->svg_bmp[SVG_TEXT_STY], gui->modal == TXT_PROP)){
            gui->modal = TXT_PROP;
            strncpy(gui->ctx_tools_title, _l("Text Properties"), DXF_MAX_CHARS);
            gui->step = 0;
          }
          if (gui_sel_b (gui, gui->svg_bmp[SVG_FIND], gui->modal == FIND)){
            gui->modal = FIND;
            strncpy(gui->ctx_tools_title, _l("Find/Replace"), DXF_MAX_CHARS);
            gui->step = 0;
          }
          nk_group_end(gui->ctx);
        }
        
        
        /* file tools*/
        nk_layout_row_push(gui->ctx, 3*(ICON_SIZE + 4 + 4) + 13);
        if (nk_group_begin(gui->ctx, "_modify", NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {
          nk_layout_row_static(gui->ctx, 28, 28, 3);
        
          if (gui_sel_b (gui, gui->svg_bmp[SVG_MOVE], gui->modal == MOVE)){
            gui->modal = MOVE;
            strncpy(gui->ctx_tools_title, _l("Move"), DXF_MAX_CHARS);
            gui->step = 0;
          }
          if (gui_sel_b (gui, gui->svg_bmp[SVG_DUPLI], gui->modal == DUPLI)){
            gui->modal = DUPLI;
            strncpy(gui->ctx_tools_title, _l("Duplicate"), DXF_MAX_CHARS);
            gui->step = 0;
          }
          if (gui_sel_b (gui, gui->svg_bmp[SVG_SCALE], gui->modal == SCALE)){
            gui->modal = SCALE;
            strncpy(gui->ctx_tools_title, _l("Scale"), DXF_MAX_CHARS);
            gui->step = 0;
          }
          if (gui_sel_b (gui, gui->svg_bmp[SVG_ROT], gui->modal == ROTATE)){
            gui->modal = ROTATE;
            strncpy(gui->ctx_tools_title, _l("Rotate"), DXF_MAX_CHARS);
            gui->step = 0;
          }
          if (gui_sel_b (gui, gui->svg_bmp[SVG_MIRROR], gui->modal == MIRROR)){
            gui->modal = MIRROR;
            strncpy(gui->ctx_tools_title, _l("Mirror"), DXF_MAX_CHARS);
            gui->step = 0;
          }
          if (gui_sel_b (gui, gui->svg_bmp[SVG_EXPLODE], gui->modal == EXPLODE)){
            gui->modal = EXPLODE;
            strncpy(gui->ctx_tools_title, _l("Explode"), DXF_MAX_CHARS);
            gui->step = 0;
          }
          if (gui_sel_b (gui, gui->svg_bmp[SVG_EDIT], gui->modal == VERTEX)){
            gui->modal = VERTEX;
            strncpy(gui->ctx_tools_title, _l("Edit Vertex"), DXF_MAX_CHARS);
            gui->step = 0;
          }
          
          if (gui_sel_b (gui, gui->svg_bmp[SVG_STYLE], gui->modal == PROP)){
            gui->modal = PROP;
            strncpy(gui->ctx_tools_title, _l("Element Properties"), DXF_MAX_CHARS);
            gui->step = 0;
          }
          if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_TRASH]))){
            gui->action = DELETE;
            gui->step = 0;
          }
          nk_group_end(gui->ctx);
        }
        
        
        /* dimmensions */
        nk_layout_row_push(gui->ctx, 2*(ICON_SIZE + 4 + 4) + 13);
        if (nk_group_begin(gui->ctx, "_dimmensions", NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {
          nk_layout_row_static(gui->ctx, 28, 28, 2);
          
          if (gui_sel_b (gui, gui->svg_bmp[SVG_DIM_LINEAR], gui->modal == DIM_LINEAR)){
            gui->modal = DIM_LINEAR;
            strncpy(gui->ctx_tools_title, _l("Linear Dimmension"), DXF_MAX_CHARS);
            gui->step = 0;
          }
          if (gui_sel_b (gui, gui->svg_bmp[SVG_DIM_ANGULAR], gui->modal == DIM_ANGULAR)){
            gui->modal = DIM_ANGULAR;
            strncpy(gui->ctx_tools_title, _l("Angular Dimmension"), DXF_MAX_CHARS);
            gui->step = 0;
          }
          if (gui_sel_b (gui, gui->svg_bmp[SVG_DIM_RADIUS], gui->modal == DIM_RADIUS)){
            gui->modal = DIM_RADIUS;
            strncpy(gui->ctx_tools_title, _l("Radial Dimmension"), DXF_MAX_CHARS);
            gui->step = 0;
          }
          if (gui_sel_b (gui, gui->svg_bmp[SVG_DIM_ORDINATE], gui->modal == DIM_ORDINATE)){
            gui->modal = DIM_ORDINATE;
            strncpy(gui->ctx_tools_title, _l("Ordinate Dimmension"), DXF_MAX_CHARS);
            gui->step = 0;
          }
          
          nk_group_end(gui->ctx);
        }

      }
      
      
      
      
      
      
      
      
      if(ribbon_grp == RIB_VIEW || (gui->win_w > rib_w[RIB_VIEW] &&
        gui->win_w > rib_w[ribbon_grp] && ribbon_grp != RIB_FILE)){
        /* zoom tools*/
        nk_layout_row_push(gui->ctx, 2*(ICON_SIZE + 4 + 4) + 13);
        if (nk_group_begin(gui->ctx, "_zoomtools", NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {
          nk_layout_row_static(gui->ctx, 28, 28, 2);
          if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_ZOOM_P]))){
            gui->action = VIEW_ZOOM_P;
          }
          if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_ZOOM_M]))){
            gui->action = VIEW_ZOOM_M;
          }
          if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_ZOOM_W]))){
            gui->modal = ZOOM;
            strncpy(gui->ctx_tools_title, _l("Zoom"), DXF_MAX_CHARS);
            gui->step = 0;
          }
          if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_ZOOM_A]))){
            gui->action = VIEW_ZOOM_EXT;
          }
          if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_BRUSH]))){
            gui->action = REDRAW;
          }
          nk_group_end(gui->ctx);
        }
      
        /* pan tools*/
        nk_layout_row_push(gui->ctx, 2*(ICON_SIZE + 4 + 4) + 13);
        if (nk_group_begin(gui->ctx, "_pantools", NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {
          nk_layout_row_static(gui->ctx, 28, 28, 2);
          if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_UP]))){
            gui->action = VIEW_PAN_U;
          }
          if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_DOWN]))){
            gui->action = VIEW_PAN_D;
          }
          if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_LEFT]))){
            gui->action = VIEW_PAN_L;
          }
          if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_RIGTH]))){
            gui->action = VIEW_PAN_R;
          }
          nk_group_end(gui->ctx);
        }
        /* 3d view tools*/
        nk_layout_row_push(gui->ctx, 2*(ICON_SIZE + 4 + 4) + 13);
        if (nk_group_begin(gui->ctx, "_3dviewtools", NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {
          nk_layout_row_static(gui->ctx, 28, 28, 2);
          if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_VIEW_TOP]))){
            gui->alpha = 0.0;
            gui->beta = 0.0;
            gui->gamma = 0.0;gui->ofs_z = 0.0;
            gui_calc_view_rot (gui);
            
          }
          if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_VIEW_FRONT]))){
            gui->alpha = 0.0;
            gui->beta = 0.0;
            gui->gamma = 90.0;gui->ofs_y = 0.0;
            gui_calc_view_rot (gui);
            
          }
          if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_VIEW_RIGHT]))){
            gui->alpha = 90.0;
            gui->beta = 0.0;
            gui->gamma = 90.0;gui->ofs_x = 0.0;
            gui_calc_view_rot (gui);
            
          }
          if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_VIEW_ISOMETRIC]))){
            gui->alpha = 45.0;
            gui->beta = 0.0;
            gui->gamma = 90.0 - 35.264;
            gui_calc_view_rot (gui);
          }
          if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_VIEW_ROTATE]))){
            gui->modal = VIEW_ROTATE;
            strncpy(gui->ctx_tools_title, _l("Rotate View"), DXF_MAX_CHARS);
            gui->step = 0;
            
          }
          nk_group_end(gui->ctx);
        }
      
      }
      
      
      if(ribbon_grp == RIB_MANAGER || (gui->win_w > rib_w[RIB_MANAGER] &&
        gui->win_w > rib_w[ribbon_grp] && ribbon_grp != RIB_FILE)){
        /* managers*/
        nk_layout_row_push(gui->ctx, 2*(ICON_SIZE + 4 + 4) + 13);
        if (nk_group_begin(gui->ctx, "_managerstools", NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {
          nk_layout_row_static(gui->ctx, 28, 28, 2);
          if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_LAYERS]))){
            gui->show_lay_mng = 1;
          }
          if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_FONT]))){
            gui->show_tstyles_mng = 1;
          }
          if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_LTYPE]))){
            gui->show_ltyp_mng = 1;
          }
          if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_PUZZLE]))){
            gui->show_blk_mng = 1;
          }
          if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_DIM_CONFIG]))){
            gui->show_dim_mng = 1;
          }
          nk_group_end(gui->ctx);
        }
      }
      
      
      
      
      
      if(ribbon_grp == RIB_3D || 
        (gui->win_w > rib_w[RIB_3D] && ribbon_grp != RIB_FILE)){
        /* solids */
        nk_layout_row_push(gui->ctx, 3*(ICON_SIZE + 4 + 4) + 13);
        if (nk_group_begin(gui->ctx, "_solidstools", NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {
          nk_layout_row_static(gui->ctx, 28, 28, 3);
        
          if (gui_sel_b (gui, gui->svg_bmp[SVG_3D_SPHERE], gui->modal == SPHERE)){
            gui->modal = SPHERE;
            strncpy(gui->ctx_tools_title, _l("Sphere"), DXF_MAX_CHARS);
            gui->step = 0;
          }
          
          if (gui_sel_b (gui, gui->svg_bmp[SVG_3D_CYLINDER], gui->modal == CYLINDER)){
            gui->modal = CYLINDER;
            strncpy(gui->ctx_tools_title, _l("Cylinder"), DXF_MAX_CHARS);
            gui->step = 0;
          }
          
          if (gui_sel_b (gui, gui->svg_bmp[SVG_3D_CONE], gui->modal == CONE)){
            gui->modal = CONE;
            strncpy(gui->ctx_tools_title, _l("Cone"), DXF_MAX_CHARS);
            gui->step = 0;
          }
          
          if (gui_sel_b (gui, gui->svg_bmp[SVG_3D_SLAB], gui->modal == SLAB)){
            gui->modal = SLAB;
            strncpy(gui->ctx_tools_title, _l("Slab"), DXF_MAX_CHARS);
            gui->step = 0;
          }
          
          if (gui_sel_b (gui, gui->svg_bmp[SVG_3D_WEDGE], gui->modal == WEDGE)){
            gui->modal = WEDGE;
            strncpy(gui->ctx_tools_title, _l("Wedge"), DXF_MAX_CHARS);
            gui->step = 0;
          }
          
          if (gui_sel_b (gui, gui->svg_bmp[SVG_3D_PYRAMID], gui->modal == PYRAMID)){
            gui->modal = PYRAMID;
            strncpy(gui->ctx_tools_title, _l("Pyramid"), DXF_MAX_CHARS);
            gui->step = 0;
          }
          
          if (gui_sel_b (gui, gui->svg_bmp[SVG_3D_TORUS], gui->modal == TORUS)){
            gui->modal = TORUS;
            strncpy(gui->ctx_tools_title, _l("Torus"), DXF_MAX_CHARS);
            gui->step = 0;
          }
          nk_group_end(gui->ctx);
        }
        
        /* extrude */
        nk_layout_row_push(gui->ctx, (ICON_SIZE + 4 + 4) + 13);
        if (nk_group_begin(gui->ctx, "_extrudetools", NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {
          nk_layout_row_static(gui->ctx, 28, 28, 1);
          if (gui_sel_b (gui, gui->svg_bmp[SVG_3D_EXTRUDE], gui->modal == EXTRUDE)){
            gui->modal = EXTRUDE;
            strncpy(gui->ctx_tools_title, _l("Extrude"), DXF_MAX_CHARS);
            gui->step = 0;
          }
          
          if (gui_sel_b (gui, gui->svg_bmp[SVG_3D_EXTRUDE_PATH], gui->modal == EXTRUDE_PATH)){
            gui->modal = EXTRUDE_PATH;
            strncpy(gui->ctx_tools_title, _l("Extrude path"), DXF_MAX_CHARS);
            gui->step = 0;
          }
          
          if (gui_sel_b (gui, gui->svg_bmp[SVG_3D_REVOLVE], gui->modal == REVOLVE)){
            gui->modal = REVOLVE;
            strncpy(gui->ctx_tools_title, _l("Revolve"), DXF_MAX_CHARS);
            gui->step = 0;
          }
          nk_group_end(gui->ctx);
        }
        
        /* 3D booleans */
        nk_layout_row_push(gui->ctx, (ICON_SIZE + 4 + 4) + 13);
        if (nk_group_begin(gui->ctx, "_3dbooltools", NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {
          nk_layout_row_static(gui->ctx, 28, 28, 1);
          if (gui_sel_b (gui, gui->svg_bmp[SVG_3D_UNION], gui->modal == UNION)){
            gui->modal = UNION;
            strncpy(gui->ctx_tools_title, _l("Union"), DXF_MAX_CHARS);
            gui->step = 0;
          }
          
          if (gui_sel_b (gui, gui->svg_bmp[SVG_3D_SUBTRACT], gui->modal == SUBTRACT)){
            gui->modal = SUBTRACT;
            strncpy(gui->ctx_tools_title, _l("Subtract"), DXF_MAX_CHARS);
            gui->step = 0;
          }
          
          if (gui_sel_b (gui, gui->svg_bmp[SVG_3D_INTERSECTION], gui->modal == INTERSECTION)){
            gui->modal = INTERSECTION;
            strncpy(gui->ctx_tools_title, _l("Intersection"), DXF_MAX_CHARS);
            gui->step = 0;
          }
          nk_group_end(gui->ctx);
        }
        
        
        
        /* 3D transforms */
        nk_layout_row_push(gui->ctx, (ICON_SIZE + 4 + 4) + 13);
        if (nk_group_begin(gui->ctx, "_3dtranstools", NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {
          nk_layout_row_static(gui->ctx, 28, 28, 1);
          
          if (gui_sel_b (gui, gui->svg_bmp[SVG_3D_SLICE], gui->modal == SLICE)){
            gui->modal = SLICE;
            strncpy(gui->ctx_tools_title, _l("Slice"), DXF_MAX_CHARS);
            gui->step = 0;
          }
          
          if (gui_sel_b (gui, gui->svg_bmp[SVG_3D_ROTATE], gui->modal == ROTATE_3D)){
            gui->modal = ROTATE_3D;
            strncpy(gui->ctx_tools_title, _l("Rotate 3D"), DXF_MAX_CHARS);
            gui->step = 0;
          }
          
          nk_group_end(gui->ctx);
        }
      }
      
      nk_layout_row_end(gui->ctx);
      
      nk_group_end(gui->ctx);
    }
    if (nk_group_begin(gui->ctx, "Common tools", NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {
      
      nk_layout_row_dynamic(gui->ctx, 38, 1);
      //nk_layout_row_static(gui->ctx, ICON_SIZE + 4, ICON_SIZE + 4, 3);
      
      /* clipboard tools*/
      //nk_layout_row_push(gui->ctx, 3*(ICON_SIZE + 4 + 4) + 13);
      if (nk_group_begin(gui->ctx, "_clipboard", NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {
        nk_layout_row_static(gui->ctx, ICON_SIZE + 4, ICON_SIZE + 4, 3);
        
        
        if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_COPY]))){
          gui->action = YANK;
        }
        if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_CUT]))){
          gui->action = CUT;
        }
        if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_PASTE]))){
          gui->action = START_PASTE;
          
        }
        
        nk_group_end(gui->ctx);
      }
      
      /* undo/redo tools*/
      //nk_layout_row_push(gui->ctx, 2*(ICON_SIZE + 4 + 4) + 13);
      if (nk_group_begin(gui->ctx, "_undo-redo", NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {
        nk_layout_row_static(gui->ctx, ICON_SIZE + 4, ICON_SIZE + 4, 2);
        
        if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_UNDO]))){
          gui->action = UNDO;
        }
        if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_REDO]))){
          gui->action = REDO;
        }
        nk_group_end(gui->ctx);
      }
      if (nk_group_begin(gui->ctx, "_sel_meas_plug", NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {
      
        nk_layout_row_static(gui->ctx, ICON_SIZE + 4, ICON_SIZE + 4, 3);
        if (gui_sel_b (gui, gui->svg_bmp[SVG_CURSOR], gui->modal == SELECT)){
          gui->modal = SELECT;
          strncpy(gui->ctx_tools_title, _l("Select"), DXF_MAX_CHARS);
          gui->step = 0;
        }
        if (gui_sel_b (gui, gui->svg_bmp[SVG_RULER], gui->modal == MEASURE)){
          gui->modal = MEASURE;
          strncpy(gui->ctx_tools_title, _l("Measure"), DXF_MAX_CHARS);
          gui->step = 0;
        }
        if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_TOOL]))){
          gui->show_plugins = 1;
        }
        nk_group_end(gui->ctx);
      }
      
      nk_group_end(gui->ctx);
    }
		if (nk_group_begin(gui->ctx, "Current_properties", NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {
      //nk_layout_row_dynamic(gui->ctx, 5, 1); /* only a blank space */
      
      /* second line */
      //nk_layout_row_begin(gui->ctx, NK_STATIC, 20, 8);
      nk_layout_row(gui->ctx, NK_STATIC, 23, 2, (float[]){90, 290});
      static char text[64], transp_txt[64];
      int text_len;
        
      /*layer*/
      //nk_layout_row_push(gui->ctx, 60);
      nk_label(gui->ctx, _l("Layer: "), NK_TEXT_RIGHT);
      //nk_layout_row_push(gui->ctx, 200);
      layer_prop(gui);
      
      
      /*color picker */
      int c_idx = gui->color_idx;
      if (c_idx >255){
        c_idx = gui->drawing->layers[gui->layer_idx].color;
        if (c_idx >255){
          c_idx = 0;
        }
      }
      /* fill the tiny bitmap with selected color*/
      bmp_fill(gui->color_img, dxf_colors[c_idx]);
      
      /* print the name (number) of color */
      if (gui->color_idx == 0){
        text_len = snprintf(text, 63, "%s", _l("By Block"));
      }
      else if (gui->color_idx < 256){
        text_len = snprintf(text, 63, "%d", gui->color_idx);
      }
      else{
        text_len = snprintf(text, 63, "%s", _l("By Layer"));
      }
      //nk_layout_row_push(gui->ctx, 70);
      nk_label(gui->ctx, _l("Color: "), NK_TEXT_RIGHT);
      //nk_layout_row_push(gui->ctx, 80);
      if (nk_combo_begin_image_label(gui->ctx, text, nk_image_ptr(gui->color_img), nk_vec2(290,525))){
        nk_layout_row_dynamic(gui->ctx, 20, 2);
        if (nk_button_label(gui->ctx, _l("By Layer"))){
          gui->color_idx = 256;
          gui->action = COLOR_CHANGE;
          nk_combo_close(gui->ctx);
        }
        if (nk_button_label(gui->ctx, _l("By Block"))){
          gui->color_idx = 0;
          gui->action = COLOR_CHANGE;
          nk_combo_close(gui->ctx);
        }
        nk_layout_row_static(gui->ctx, 15, 22, 10);
        nk_label(gui->ctx, " ", NK_TEXT_RIGHT); /* for padding color alingment */
        
        for (i = 1; i < 256; i++){
          struct nk_color b_color = {
            .r = dxf_colors[i].r,
            .g = dxf_colors[i].g,
            .b = dxf_colors[i].b,
            .a = dxf_colors[i].a
          };
          if(nk_button_color(gui->ctx, b_color)){
            gui->color_idx = i;
            gui->action = COLOR_CHANGE;
            nk_combo_close(gui->ctx);
            break;
          }
        }
        
        nk_combo_end(gui->ctx);
      }
      //nk_layout_row_end(gui->ctx);
      
      //nk_layout_row_begin(gui->ctx, NK_STATIC, 20, 8);
      
      /*line type*/
      //nk_layout_row_push(gui->ctx, 100);
      nk_label(gui->ctx, _l("Line type: "), NK_TEXT_RIGHT);
      //nk_layout_row_push(gui->ctx, 200);
      ltype_prop(gui);
      
      /* line weight */
      //nk_layout_row_push(gui->ctx, 120);
      nk_label(gui->ctx, _l("Line weight: "), NK_TEXT_RIGHT);
      //nk_layout_row_push(gui->ctx, 120);
      
      char * lw_descr = (char *)dxf_lw_descr[gui->lw_idx];
      if(gui->lw_idx == DXF_LW_LEN) lw_descr = _l("By Layer");
      if(gui->lw_idx == DXF_LW_LEN + 1) lw_descr = _l("By Block");
      
      if (nk_combo_begin_label(gui->ctx, lw_descr, nk_vec2(290,300))){
        nk_layout_row_dynamic(gui->ctx, 25, 2);
        if (nk_button_label(gui->ctx, _l("By Layer"))){
          gui->lw_idx = DXF_LW_LEN;
          gui->action = LW_CHANGE;
          nk_combo_close(gui->ctx);
        }
        if (nk_button_label(gui->ctx, _l("By Block"))){
          gui->lw_idx = DXF_LW_LEN + 1;
          gui->action = LW_CHANGE;
          nk_combo_close(gui->ctx);
        }
        nk_layout_row_dynamic(gui->ctx, 17, 1);
        for (i = 0; i < DXF_LW_LEN; i++){
          if (nk_button_label(gui->ctx, dxf_lw_descr[i])){
            gui->lw_idx = i;
            gui->action = LW_CHANGE;
            nk_combo_close(gui->ctx);
            break;
          }
        }
        nk_combo_end(gui->ctx);
      }
      
      static int transp_value = 0;
      static char transp_b[64];
      
      nk_layout_row(gui->ctx, NK_STATIC, 23, 2, (float[]){120, 260});
      if (gui->transparency >= 0 && gui->transparency <= 100)
        snprintf(transp_txt, 63, "%d%%", gui->transparency);
      else if (gui->transparency == -1)
        snprintf(transp_txt, 63, _l("By Block"));
      else snprintf(transp_txt, 63, _l("By Layer"));
      nk_label(gui->ctx, _l("Transparency: "), NK_TEXT_RIGHT);
      
      if (nk_combo_begin_label(gui->ctx, transp_txt, nk_vec2(260, 80))){
        nk_layout_row_dynamic(gui->ctx, 25, 2);
        if (nk_button_label(gui ->ctx, _l("By Layer"))){
          gui->transparency = -2;
          gui->action = TRANSP_CHANGE;
          nk_combo_close(gui->ctx);
        }
        if (nk_button_label(gui->ctx, _l("By Block"))){
          gui->transparency = -1;
          gui->action = TRANSP_CHANGE;
          nk_combo_close(gui->ctx);
        }
        nk_layout_row(gui->ctx, NK_DYNAMIC, 40, 2, (float[]){0.6, 0.4});
        //nk_progress(gui->ctx, &transp_value, 100, NK_MODIFIABLE);
        nk_slider_int(gui->ctx, 0, &transp_value, 100, 1);
        //transp_value = nk_propertyi(gui->ctx, _l("#Transp %"), 0, transp_value, 100, 1, 1.0);
        snprintf(transp_b, 63, _l("Use: %d%%"), transp_value);
        if (nk_button_label(gui ->ctx, transp_b)){
          gui->transparency = transp_value;
          gui->action = TRANSP_CHANGE;
          nk_combo_close(gui->ctx);
        }
        nk_combo_end(gui->ctx);
      }
      
      
      
      //nk_layout_row_end(gui->ctx);
      
      /*------------ end second line --------------*/
      nk_group_end(gui->ctx);
		}
		if (gui->show_app_about){
			/* About Cadzinho */
			const char* site = "https://github.com/zecruel/CadZinho";
			static struct nk_rect s = {250, 150, 400, 210};
			if (nk_popup_begin(gui->ctx, NK_POPUP_STATIC, _l("About"), NK_WINDOW_CLOSABLE|NK_WINDOW_NO_SCROLLBAR, s)){
				nk_layout_space_begin(gui->ctx, NK_STATIC, 50, 2);
				nk_layout_space_push(gui->ctx, nk_rect(100, 0, 48, 48));
				nk_image(gui->ctx, nk_image_ptr(gui->i_cz48));
				
				nk_layout_space_push(gui->ctx, nk_rect(150, 10, 150, 30));
				nk_style_push_font(gui->ctx, &(gui->alt_font_sizes[FONT_HUGE])); /* change font to huge*/
				nk_label(gui->ctx, "CadZinho", NK_TEXT_CENTERED);
				nk_style_pop_font(gui->ctx); /* return to the default font*/
				nk_layout_space_end(gui->ctx);
				
				nk_layout_space_begin(gui->ctx, NK_STATIC, 40, 2);
				nk_layout_space_push(gui->ctx, nk_rect(80, 2, 250, 24));
				nk_label(gui->ctx, _l("By Ezequiel Rabelo de Aguiar"), NK_TEXT_LEFT);
				
				nk_layout_space_push(gui->ctx, nk_rect(340, 0, 24, 24));
				nk_image(gui->ctx, nk_image_ptr(gui->svg_bmp[SVG_BRAZIL]));
				nk_layout_space_end(gui->ctx);
				
				nk_layout_row_dynamic(gui->ctx, 20, 1);
				nk_label(gui->ctx, _l("CadZinho is licensed under the MIT License."),  NK_TEXT_CENTERED);
				if (nk_button_label(gui->ctx, site)){
					opener(site);
				}
				nk_layout_row_dynamic(gui->ctx, 20, 4);
				nk_label(gui->ctx, _l("Build for: "),  NK_TEXT_RIGHT);
				nk_label(gui->ctx, operating_system(),  NK_TEXT_LEFT);
				nk_label(gui->ctx, _l("Version: "),  NK_TEXT_RIGHT);
				nk_label(gui->ctx, CZ_VERSION,  NK_TEXT_LEFT);
				nk_popup_end(gui->ctx);
				
			} else gui->show_app_about = nk_false;
		}
		
		if (gui->discard_changes){
			if (nk_popup_begin(gui->ctx, NK_POPUP_STATIC, _l("Discard changes"), NK_WINDOW_CLOSABLE|NK_WINDOW_NO_SCROLLBAR, nk_rect(200, 100, 370, 100))){
				//nk_layout_row_dynamic(gui->ctx, 20, 2);
				nk_layout_row_template_begin(gui->ctx, 23);
				nk_layout_row_template_push_static(gui->ctx, 24);
				nk_layout_row_template_push_dynamic(gui->ctx);
				nk_layout_row_template_end(gui->ctx);
				nk_image(gui->ctx, nk_image_ptr(gui->svg_bmp[SVG_WARNING]));
				nk_label(gui->ctx, _l("Changes in drawing will be lost"), NK_TEXT_LEFT);
				nk_layout_row_dynamic(gui->ctx, 20, 2);
				if ((nk_button_label(gui->ctx, _l("Discard"))) && (gui->show_file_br != 1)) {
					gui->action = gui->desired_action;
					gui->discard_changes = 0;
					gui->desired_action = NONE;
					if (gui->hist_action == HIST_PREV){
						/* get previous position in history, considering as circular buffer */
						int pos = (gui->drwg_hist_pos - 1 + gui->drwg_hist_head) % DRWG_HIST_MAX;
						/* update position */
						gui->drwg_hist_pos --;
						gui->drwg_hist_wr = gui->drwg_hist_pos + 1; /* next write position */
						gui->hist_new = 0; /* not change history entries */
					}
					else if (gui->hist_action == HIST_NEXT){
						/* get next position in history, considering as circular buffer */
						int pos = (gui->drwg_hist_pos + 1 + gui->drwg_hist_head) % DRWG_HIST_MAX;
						/* update position */
						gui->drwg_hist_pos ++;
						gui->drwg_hist_wr = gui->drwg_hist_pos + 1; /* next write position */
						gui->hist_new = 0; /* not change history entries */
					}
					else if (gui->hist_action == HIST_ADD){
						gui->hist_new = 1;
					}
					gui->hist_action = HIST_NONE;
					nk_popup_close(gui->ctx);
				}
				if (nk_button_label(gui->ctx, _l("Cancel"))) {
					gui->discard_changes = 0;
					gui->desired_action = NONE;
					gui->path_ok = 0;
					gui->hist_new = 0;
					gui->hist_action = HIST_NONE;
					nk_popup_close(gui->ctx);
				}
				
				nk_popup_end(gui->ctx);
			}
			else {
				gui->discard_changes = 0;
			}
		}
		
	} 
	nk_end(gui->ctx);
	
		
	
	return 1;
	
}

int gui_bottom_win (gui_obj *gui){
	
	static int en_attr = 1;
	
	/* interface to the user visualize and enter coordinates distances*/
	if (nk_begin(gui->ctx, "POS", nk_rect(2, gui->win_h - 92, gui->win_w - 4, 90),
	NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR))
	{
		char text[64];
		nk_layout_row_begin(gui->ctx, NK_STATIC, 55, 5);
		nk_layout_row_push(gui->ctx, 420);
		
		/* interface to the user visualize and enter coordinates and distances*/
		gui_xy(gui);
		
		
		/*----------- attractors --------------*/
		nk_layout_row_push(gui->ctx, 16*(28) + 20);
		if (nk_group_begin(gui->ctx, "attractors", NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {
			
			nk_style_push_vec2(gui->ctx, &gui->ctx->style.window.spacing, nk_vec2(0,0));
			nk_layout_row_begin(gui->ctx, NK_STATIC, 28, 16);
			/* enable/disable attractors */
			if (gui_tab_img (gui, gui->svg_bmp[SVG_MAGNET], en_attr, 28)){
				en_attr = !en_attr;
			}
			/* Buttons to select attractor mode*/
			if (gui_tab_img (gui, gui->svg_bmp[SVG_ATRC_END],
				gui->curr_attr_t & ATRC_END, 28)){
				gui->curr_attr_t ^= ATRC_END;
			}
			if (gui_tab_img (gui, gui->svg_bmp[SVG_ATRC_MID],
				gui->curr_attr_t & ATRC_MID, 28)){
				gui->curr_attr_t ^= ATRC_MID;
			}
			if (gui_tab_img (gui, gui->svg_bmp[SVG_ATRC_CENTER],
				gui->curr_attr_t & ATRC_CENTER, 28)){
				gui->curr_attr_t ^= ATRC_CENTER;
			}
			if (gui_tab_img (gui, gui->svg_bmp[SVG_ATRC_QUAD],
				gui->curr_attr_t & ATRC_QUAD, 28)){
				gui->curr_attr_t ^= ATRC_QUAD;
			}
			if (gui_tab_img (gui, gui->svg_bmp[SVG_ATRC_INTER],
				gui->curr_attr_t & ATRC_INTER, 28)){
				gui->curr_attr_t ^= ATRC_INTER;
			}
			if (gui_tab_img (gui, gui->svg_bmp[SVG_ATRC_AINT],
				gui->curr_attr_t & ATRC_AINT, 28)){
				gui->curr_attr_t ^= ATRC_AINT;
			}
			if (gui_tab_img (gui, gui->svg_bmp[SVG_ATRC_EXT],
				gui->curr_attr_t & ATRC_EXT, 28)){
				gui->curr_attr_t ^= ATRC_EXT;
			}
			if (gui_tab_img (gui, gui->svg_bmp[SVG_ATRC_INS],
				gui->curr_attr_t & ATRC_INS, 28)){
				gui->curr_attr_t ^= ATRC_INS;
			}
			if (gui_tab_img (gui, gui->svg_bmp[SVG_ATRC_NODE],
				gui->curr_attr_t & ATRC_NODE, 28)){
				gui->curr_attr_t ^= ATRC_NODE;
			}
			if (gui_tab_img (gui, gui->svg_bmp[SVG_ATRC_OCENTER],
				gui->curr_attr_t & ATRC_OCENTER, 28)){
				gui->curr_attr_t ^= ATRC_OCENTER;
			}
			if (gui_tab_img (gui, gui->svg_bmp[SVG_ATRC_PAR],
				gui->curr_attr_t & ATRC_PAR, 28)){
				gui->curr_attr_t ^= ATRC_PAR;
			}
			if (gui_tab_img (gui, gui->svg_bmp[SVG_ATRC_PERP],
				gui->curr_attr_t & ATRC_PERP, 28)){
				gui->curr_attr_t ^= ATRC_PERP;
			}
			if (gui_tab_img (gui, gui->svg_bmp[SVG_ATRC_TAN],
				gui->curr_attr_t & ATRC_TAN, 28)){
				gui->curr_attr_t ^= ATRC_TAN;
			}
			if (gui_tab_img (gui, gui->svg_bmp[SVG_ATRC_CTRL],
				gui->curr_attr_t & ATRC_CTRL, 28)){
				gui->curr_attr_t ^= ATRC_CTRL;
			}
			if (gui_tab_img (gui, gui->svg_bmp[SVG_ATRC_ANY],
				gui->curr_attr_t & ATRC_ANY, 28)){
				gui->curr_attr_t ^= ATRC_ANY;
			}
			
			nk_style_pop_vec2(gui->ctx);
			nk_layout_row_end(gui->ctx);
			
			nk_group_end(gui->ctx);
		}
		/*-------------------------------*/
		
		nk_layout_row_push(gui->ctx, 2*(ICON_SIZE + 4 + 4) + 13);
		
		if (nk_group_begin(gui->ctx, "history", NK_WINDOW_NO_SCROLLBAR)) {
			nk_layout_row_static(gui->ctx, ICON_SIZE + 4, ICON_SIZE + 4, 2);
			
			if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_PREV]))){
				if (gui->drwg_hist_size > 1 && gui->drwg_hist_pos > 0 /* verify if not at begining */
          && (wait_open == 0)) /* check if exists a pendent operation */
        {
					/* get previous position in history, considering as circular buffer */
					int pos = (gui->drwg_hist_pos - 1 + gui->drwg_hist_head) % DRWG_HIST_MAX;
					/* get path from history */
					gui->curr_path[0] = 0;
					strncpy (gui->curr_path, gui->drwg_hist[pos], DXF_MAX_CHARS);
					gui->path_ok = 1;
					
					/* open file */
					if (gui->changed){
						gui->discard_changes = 1;
						gui->desired_action = FILE_OPEN;
						gui->hist_action = HIST_PREV;
					}
					else{
						/* update position */
						gui->drwg_hist_pos --;
						gui->drwg_hist_wr = gui->drwg_hist_pos + 1; /* next write position */
						gui->hist_new = 0; /* not change history entries */
						gui->action = FILE_OPEN;
					}
				}
				
			}
			if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_NEXT]))){
				if (gui->drwg_hist_pos < gui->drwg_hist_size - 1 &&
				gui->drwg_hist_size > 1 /* verify if not at end */
        && (wait_open == 0)) /* check if exists a pendent operation */
      {
					/* get next position in history, considering as circular buffer */
					int pos = (gui->drwg_hist_pos + 1 + gui->drwg_hist_head) % DRWG_HIST_MAX;
					/* get path from history */
					gui->curr_path[0] = 0;
					strncpy (gui->curr_path, gui->drwg_hist[pos], DXF_MAX_CHARS);
					gui->path_ok = 1;
					
					if (gui->changed){
						gui->discard_changes = 1;
						gui->desired_action = FILE_OPEN;
						gui->hist_action = HIST_NEXT;
					}
					else{
						/* update position */
						gui->drwg_hist_pos ++;
						gui->drwg_hist_wr = gui->drwg_hist_pos + 1; /* next write position */
						gui->hist_new = 0; /* not change history entries */
						gui->action = FILE_OPEN;
					}

				}
			}
			
			/* show position and size of history */
			nk_layout_row_dynamic(gui->ctx, 17, 1);
			nk_style_push_font(gui->ctx, &(gui->alt_font_sizes[FONT_SMALL])); /* change font to tiny*/
			
			snprintf(text, 63, _l("%d of %d"), gui->drwg_hist_wr, gui->drwg_hist_size);
			nk_label(gui->ctx, text, NK_TEXT_CENTERED);
			nk_style_pop_font(gui->ctx); /* return to the default font*/
			
			nk_group_end(gui->ctx);
		}
		
		nk_layout_row_end(gui->ctx);
		
		nk_layout_row_begin(gui->ctx, NK_STATIC, 17, 10);
		
		nk_style_push_font(gui->ctx, &(gui->alt_font_sizes[FONT_SMALL])); /* change font to tiny */
		
		nk_layout_row_push(gui->ctx, 580);
		nk_label(gui->ctx, gui->log_msg, NK_TEXT_LEFT);
		
		text[0] = 0;
		gui->sel_count = list_len(gui->sel_list);
		if (gui->sel_count > 0) snprintf(text, 63, ":%d", gui->sel_count);
		nk_layout_row_push(gui->ctx, 280);
		nk_label(gui->ctx, text, NK_TEXT_LEFT);
		
		nk_style_pop_font(gui->ctx); /* return to the default font*/
		
		/*-------------------------------*/
		nk_layout_row_end(gui->ctx);
	}
	nk_end(gui->ctx);
	
	return 1;
}