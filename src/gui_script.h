#ifndef _CZ_GUI_SCRIPT_LIB
#define _CZ_GUI_SCRIPT_LIB

#include "gui.h"
#include "gui_file.h"
#include "script.h"

int script_win (gui_obj *gui);
int gui_script_interactive(gui_obj *gui);

int gui_script_init (gui_obj *gui, struct script_obj *script, char *fname, char *alt_chunk);

int gui_script_exec_file_slot (gui_obj *gui, char *path);

int gui_script_exec_file (lua_State *L);

int gui_script_dyn(gui_obj *gui);

int gui_script_clear_dyn(gui_obj *gui);

#endif