#ifndef _CZ_GUI_USE_LIB
#define _CZ_GUI_USE_LIB

#include "gui.h"

int gui_tools_win (gui_obj *gui);

int gui_main_win (gui_obj *gui);

int gui_bottom_win (gui_obj *gui);


int gui_blk_mng (gui_obj *gui);


int gui_select_interactive(gui_obj *gui);

int gui_select_info (gui_obj *gui);

int gui_line_interactive(gui_obj *gui);

int gui_line_info (gui_obj *gui);

int gui_pline_interactive(gui_obj *gui);

int gui_pline_info (gui_obj *gui);

int gui_circle_interactive(gui_obj *gui);

int gui_circle_info (gui_obj *gui);

int gui_rect_interactive(gui_obj *gui);

int gui_rect_info (gui_obj *gui);

int gui_text_interactive(gui_obj *gui);

int gui_text_info (gui_obj *gui);

int gui_move_interactive(gui_obj *gui);

int gui_move_info (gui_obj *gui);

int gui_dupli_interactive(gui_obj *gui);

int gui_dupli_info (gui_obj *gui);

int gui_scale_interactive(gui_obj *gui);

int gui_scale_info (gui_obj *gui);

int gui_rotate_interactive(gui_obj *gui);

int gui_rotate_info (gui_obj *gui);

int gui_mirror_interactive(gui_obj *gui);

int gui_mirror_info (gui_obj *gui);


int gui_insert_interactive(gui_obj *gui);

int gui_insert_info (gui_obj *gui);

int gui_block_interactive(gui_obj *gui);

int gui_block_info (gui_obj *gui);

int gui_hatch_interactive(gui_obj *gui);

int gui_hatch_info (gui_obj *gui);

int gui_mtext_interactive(gui_obj *gui);

int gui_mtext_info (gui_obj *gui);

int gui_paste_interactive(gui_obj *gui);

int gui_paste_info (gui_obj *gui);

int gui_ed_text_interactive(gui_obj *gui);

int gui_ed_text_info (gui_obj *gui);


int gui_spline_interactive(gui_obj *gui);

int gui_spline_info (gui_obj *gui);

int gui_arc_interactive(gui_obj *gui);

int gui_arc_info (gui_obj *gui);

int gui_ellip_interactive(gui_obj *gui);

int gui_ellip_info (gui_obj *gui);

int gui_image_interactive(gui_obj *gui);

int gui_image_info (gui_obj *gui);

int gui_ed_attr_interactive(gui_obj *gui);

int gui_ed_attr_info (gui_obj *gui);

int gui_attrib_interactive(gui_obj *gui);

int gui_attrib_info (gui_obj *gui);

int gui_expl_interactive(gui_obj *gui);

int gui_expl_info (gui_obj *gui);

int gui_measure_interactive(gui_obj *gui);

int gui_measure_info (gui_obj *gui);

int gui_find_interactive(gui_obj *gui);

int gui_find_info (gui_obj *gui);

int gui_prop_interactive(gui_obj *gui);

int gui_prop_info (gui_obj *gui);

int gui_txt_prop_interactive(gui_obj *gui);

int gui_txt_prop_info (gui_obj *gui);

int gui_vertex_interactive(gui_obj *gui);

int gui_vertex_info (gui_obj *gui);

int gui_dim_interactive(gui_obj *gui);

int gui_dim_linear_info (gui_obj *gui);

int gui_dim_angular_info (gui_obj *gui);

int gui_dim_mng (gui_obj *gui);

#endif