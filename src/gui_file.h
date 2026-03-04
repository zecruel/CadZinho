#include "gui.h"

#ifndef _CZ_GUI_FILE
#define _CZ_GUI_FILE
enum files_types {
  /* Drawing - main filetype in CadZinho */
	FILE_DXF,
  /* Config, macros and scripts */
  FILE_LUA,
  /* Print output */
	FILE_PDF,
  FILE_SVG,
  FILE_PS,
  FILE_JPG,
  FILE_PNG,
  FILE_BMP,
  /* export output */
  FILE_PLT,
  FILE_NC,
  /* patterns (hatch) and line styles */
	FILE_PAT,
	FILE_LIN,
  /* fonts */
	FILE_SHP,
	FILE_SHX,
	FILE_TTF,
	FILE_OTF,
	/* filetypes with minimal support */
  FILE_XLSX,
  FILE_DB,
  FILE_SQL,
  FILE_ZIP,
  /* other know types */
	FILE_TXT,
  FILE_BIN,
  FILE_EXE,
  FILE_ELF,
  FILE_SO,
  FILE_DLL,
  
  FILE_ALL,
  FILE_UNKNOW,
  
  DIR_SUBDIR,
  DIR_CURR,
  DIR_UP
};

int file_win (gui_obj *gui, const char *ext_type[], const char *ext_descr[], int num_ext, char *init_dir);

int file_pop (gui_obj *gui, enum files_types filters[], int num_filters, char *init_dir);

int gui_file_open (gui_obj *gui, char *init_dir);

int gui_file_save (gui_obj *gui, char *init_dir);

int gui_hist_add (gui_obj *gui);

#endif