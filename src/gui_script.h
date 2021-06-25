#ifndef _CZ_GUI_SCRIPT_LIB
#define _CZ_GUI_SCRIPT_LIB

#include "gui.h"
#include "gui_file.h"
#include "script.h"

int script_win (gui_obj *gui);
int gui_script_interactive(gui_obj *gui);

int gui_script_init (gui_obj *gui, struct script_obj *script, char *fname, char *alt_chunk) ;

#endif