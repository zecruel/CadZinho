#include "gui.h"

#ifndef _CZ_GUI_FILE
#define _CZ_GUI_FILE
enum files_types {
	FILE_ALL,
	FILE_DXF,
	FILE_TXT,
	FILE_PAT,
	FILE_LIN,
	FILE_SHP,
	FILE_SHX,
	FILE_TTF,
	FILE_OTF,
	FILE_PDF,
	
	FILE_T_MAX
};

int file_win (gui_obj *gui, const char *ext_type[], const char *ext_descr[], int num_ext, char *init_dir);

int file_pop (gui_obj *gui, enum files_types filters[], int num_filters, char *init_dir);

int gui_file_open (gui_obj *gui, char *init_dir);

int gui_file_save (gui_obj *gui, char *init_dir);

#endif