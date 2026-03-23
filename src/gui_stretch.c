#include "gui_use.h"

int gui_stretch_interactive(gui_obj *gui){

	if (gui->modal != STRETCH) return 0;
	
	static double x0 = 0.0, y0 = 0.0, x1 = 0.0, y1 = 0.0, ref_x, ref_y;
	
	if (gui->step == 0){
		gui->free_sel = 0;
		if (gui->ev & EV_ENTER){
			/* starts rectangle - first corner*/
			gui->draw_phanton = 1;
      
      x0 = gui->step_x[gui->step];
			y0 = gui->step_y[gui->step];
      x1 = x0; y1 = y0;
			
			gui->step_x[gui->step + 1] = gui->step_x[gui->step];
			gui->step_y[gui->step + 1] = gui->step_y[gui->step]; 
			
			gui->step = 1;
			gui->en_distance = 1;
			gui_next_step(gui);
		}
		else if (gui->ev & EV_CANCEL){
			gui->draw_phanton = 0;
			gui_default_modal(gui);
		}
	}
	else if (gui->step == 1){
		
		x1 = gui->step_x[gui->step];
    y1 = gui->step_y[gui->step];
		
		/* draw the rectangle */
		gui->phanton = list_new(NULL, FRAME_LIFE);
		graph_obj *graph = graph_new(FRAME_LIFE);
		if (graph){
			line_add(graph, x0, y0, 0, x1, y0, 0);
			line_add(graph, x1, y0, 0, x1, y1, 0);
			line_add(graph, x1, y1, 0, x0, y1, 0);
			line_add(graph, x0, y1, 0, x0, y0, 0);
			
			list_node * new_node = list_new(graph, FRAME_LIFE);
			list_push(gui->phanton, new_node);
		}
		
		if (gui->ev & EV_ENTER){
      gui->step_x[gui->step + 1] = gui->step_x[gui->step];
			gui->step_y[gui->step + 1] = gui->step_y[gui->step]; 
      
			gui->step = 2;
			gui->en_distance = 1;
			gui_next_step(gui);
		}
		else if (gui->ev & EV_CANCEL){
			gui->draw_phanton = 0;
			gui_first_step(gui);
		}
	}
	else if (gui->step == 2){
		
		/* draw the rectangle */
		gui->phanton = list_new(NULL, FRAME_LIFE);
		graph_obj *graph = graph_new(FRAME_LIFE);
		if (graph){
			line_add(graph, x0, y0, 0, x1, y0, 0);
			line_add(graph, x1, y0, 0, x1, y1, 0);
			line_add(graph, x1, y1, 0, x0, y1, 0);
			line_add(graph, x0, y1, 0, x0, y0, 0);
			
			list_node * new_node = list_new(graph, FRAME_LIFE);
			list_push(gui->phanton, new_node);
		}
		
		if (gui->ev & EV_ENTER){
      gui->step_x[gui->step + 1] = gui->step_x[gui->step];
			gui->step_y[gui->step + 1] = gui->step_y[gui->step]; 
			ref_x = gui->step_x[gui->step];
      ref_y = gui->step_y[gui->step];
      gui->step = 3;
			gui->en_distance = 1;
			gui_next_step(gui);
		
		}
		else if (gui->ev & EV_CANCEL){
			gui->draw_phanton = 0;
			gui_first_step(gui);
		}
	}
	else if (gui->step == 3){
		double ox, oy;
		ox = gui->step_x[gui->step] - ref_x;
    oy = gui->step_y[gui->step] - ref_y;
		
		/* draw the rectangles */
		/* first position */
		gui->phanton = list_new(NULL, FRAME_LIFE);
		graph_obj *graph = graph_new(FRAME_LIFE);
		if (graph){
      /* dashed rectangle */
      graph->patt_size = 2;
      graph->pattern[0] = 10 / gui->zoom;
      graph->pattern[1] = -10 / gui->zoom;
      
			line_add(graph, x0, y0, 0, x1, y0, 0);
			line_add(graph, x1, y0, 0, x1, y1, 0);
			line_add(graph, x1, y1, 0, x0, y1, 0);
			line_add(graph, x0, y1, 0, x0, y0, 0);
			
			list_node * new_node = list_new(graph, FRAME_LIFE);
			list_push(gui->phanton, new_node);
		}
		/* final position */
		graph_obj *graph2 = graph_new(FRAME_LIFE);
		if (graph){
			line_add(graph2, x0 + ox, y0 + oy, 0, x1 + ox, y0 + oy, 0);
			line_add(graph2, x1 + ox, y0 + oy, 0, x1 + ox, y1 + oy, 0);
			line_add(graph2, x1 + ox, y1 + oy, 0, x0 + ox, y1 + oy, 0);
			line_add(graph2, x0 + ox, y1 + oy, 0, x0 + ox, y0 + oy, 0);
			
			list_node * new_node = list_new(graph2, FRAME_LIFE);
			list_push(gui->phanton, new_node);
		}
		
		if (gui->ev & EV_ENTER){
			double rect_pt1[2], rect_pt2[2], u[3], v[3];
			
			/* sort rectangle corners */
			rect_pt1[0] = (x0 < x1) ? x0 : x1;
			rect_pt1[1] = (y0 < y1) ? y0 : y1;
			rect_pt2[0] = (x0 > x1) ? x0 : x1;
			rect_pt2[1] = (y0 > y1) ? y0 : y1;
      
      /* list of objects to inspect */
      list_node *list = list_new(NULL, FRAME_LIFE);
      list_clear(list);
      int count = 0;
      
      /* get inside objects and also all intersecting to rectangle */
      count = dxf_ents_isect2(list, gui->drawing, rect_pt1, rect_pt2);
      
      /* sweep elements */
      list_node *list_el = NULL;
      if (count > 0) {
        int vert_count = 0, init_do = 0;
        list_el = list->next;
        while (list_el){
          dxf_node * ent = (dxf_node *)list_el->data;
          dxf_node * test_ent = dxf_ent_copy(ent, FRAME_LIFE);
          
          dxf_node * vert_x, * vert_y, * vert_z, * bulge, * next = NULL;
          
          double x = 0.0, y = 0.0, z = 0.0;
          
          /* Sweep vertices */
          while (dxf_get_vert_nxt(test_ent, &next, &vert_x, &vert_y, &vert_z, &bulge)){
            if (vert_x) x = vert_x->value.d_data;
            if (vert_y) y = vert_y->value.d_data;
            if (vert_z) z = vert_z->value.d_data;
           
            /* verify if vertex is inside rectangle */
            if (x > rect_pt1[0] && x < rect_pt2[0] &&
              y > rect_pt1[1] && y < rect_pt2[1]){
              /* modify vertex */
              vert_x->value.d_data += ox;
              vert_y->value.d_data += oy;
              vert_count++;
            }
            
            if (next == NULL) break;
            x = 0.0; y = 0.0; z = 0.0;
          }
          
          if (vert_count){ /* if has modifications */
            /* Write new entity */
            if (!init_do){
              do_add_entry(&gui->list_do, _l("STRETCH"));
              init_do = 1;
            }
            dxf_node * new_ent = dxf_ent_copy(test_ent, DWG_LIFE);
            new_ent->obj.graphics = dxf_graph_parse(gui->drawing, new_ent, 0, DWG_LIFE);
            dxf_obj_subst(ent, new_ent);
            do_add_item(gui->list_do.current, ent, new_ent);
          }
          
          vert_count = 0;
          list_el = list_el->next;
        }
      }
			/* restart the proccess */
			gui->draw_phanton = 0;
			gui_first_step(gui);
		}
		else if (gui->ev & EV_CANCEL){
			gui->step = 2;
		}
	}
	
	return 1;
}

int gui_stretch_info (gui_obj *gui){
	if (gui->modal == STRETCH) {
		nk_layout_row_dynamic(gui->ctx, 20, 1);
		nk_label(gui->ctx, _l("Stretch inside"), NK_TEXT_LEFT);
		if (gui->step == 0){
			nk_label(gui->ctx, _l("First corner"), NK_TEXT_LEFT);
		}
		else if (gui->step == 1){
			nk_label(gui->ctx, _l("Last corner"), NK_TEXT_LEFT);
		}
		else if (gui->step == 2){
			nk_label(gui->ctx, _l("Enter base point"), NK_TEXT_LEFT);
		} else {
			nk_label(gui->ctx, _l("Enter destination point"), NK_TEXT_LEFT);
		}
	}
	return 1;
}