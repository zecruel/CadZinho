/* ATTENTION: this source code file is not fully portable.
It use POSIX libraries that are not part of the C standard.
Therefore, not all compilers and platafforms can make this file.
Please, check the compatibility first. */


#include "gui_file.h"
#include <time.h>
/* POSIX libs*/
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
/*-----------*/


const char *filter_types[] = {
	[FILE_ALL] = "*",
	[FILE_DXF] = "DXF",
	[FILE_TXT] = "TXT",
	[FILE_PAT] = "PAT",
	[FILE_LIN] = "LIN",
	[FILE_SHP] = "SHP",
	[FILE_SHX] = "SHX",
	[FILE_TTF] = "TTF",
	[FILE_OTF] = "OTF",
};
const char *filter_descr[] = {
	[FILE_ALL] = "All files (*)",
	[FILE_DXF] = "Drawing files (.dxf)",
	[FILE_TXT] = "Text files (.txt)",
	[FILE_PAT] = "Patterns files (.pat)",
	[FILE_LIN] = "Line style files (.lin)",
	[FILE_SHP] = "Shapes files (.shp)",
	[FILE_SHX] = "Binary shapes file (.shx)",
	[FILE_TTF] = "True type font file (.ttf)",
	[FILE_OTF] = "Open font file (.otf)",
};

struct file_info{
	char name[DXF_MAX_CHARS];
	time_t date;
	off_t size;
};

/* auxiliary functions for sorting files (qsort) */
/* compare by file/dir name*/
int cmp_file_name(const void * a, const void * b) {
	struct file_info *info1 = ((struct sort_by_idx *)a)->data;
	struct file_info *info2 = ((struct sort_by_idx *)b)->data;
	
	return (strcmp(info1->name, info2->name));
}

/* compare by file/dir modification time*/
int cmp_file_date(const void * a, const void * b) {
	struct file_info *info1 = ((struct sort_by_idx *)a)->data;
	struct file_info *info2 = ((struct sort_by_idx *)b)->data;
	
	return (int)round(difftime(info1->date, info2->date));
}

/* compare by file/dir byte size*/
int cmp_file_size(const void * a, const void * b) {
	struct file_info *info1 = ((struct sort_by_idx *)a)->data;
	struct file_info *info2 = ((struct sort_by_idx *)b)->data;
	
	return (int)(info1->size - info2->size);
}

int file_win (gui_obj *gui, const char *ext_type[], const char *ext_descr[], int num_ext, char *init_dir){
	if ((ext_type == NULL) || (ext_descr == NULL) || num_ext == 0) return 0;
	
	static char full_path[MAX_PATH_LEN];
	static char sel_file[MAX_PATH_LEN];
	
	if (init_dir) chdir(init_dir);
	
	struct dirent *entry;
	static DIR *work = NULL;
	DIR *subdir;
	char ext[4], *suffix, *end;
	char str_tmp[20];
	if (!work){
		work = opendir(".");
		full_path[0] = 0;
		sel_file[0] = 0;
	}
	if (!work) return 0;
	
	int show_browser = 1;
	int i = 0;
	int idx = 0;
	
	struct stat filestat;
	struct tm *info;
	
	static struct sort_by_idx sort_dirs[10000];
	static struct sort_by_idx sort_files[10000];
	static struct file_info dirs[10000];
	static struct file_info files[10000];
	int num_files = 0, num_dirs = 0;
	
	//*path = full_path;
	
	//gui->next_win_x += gui->next_win_w + 250;
	//gui->next_win_y += gui->next_win_h + 3;
	//gui->next_win_w = 600;
	//gui->next_win_h = 510;
	
	struct nk_style_button b_dir, b_file;
	b_dir = gui->ctx->style.button;
	b_file = gui->ctx->style.button;
	
	b_file.text_alignment = NK_TEXT_LEFT;
	b_dir.text_alignment = NK_TEXT_LEFT;
	b_dir.text_normal = nk_rgb(255,255,0);
	b_dir.text_hover = nk_rgb(255,255,0);
	b_dir.text_active = nk_rgb(255,255,0);
	
	//char *ext_type[] = {"DXF", "*"};
	//char *ext_descr[] = {"Drawing files (.dxf) ", "All files (*)"};
	//int num_ext = 2;
	static int ext_idx = 0;
	
	if (ext_idx >= num_ext) ext_idx = 0;
	
	
	if (nk_begin(gui->ctx, "File explorer", nk_rect(450, 100, 600, 510),
	NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
	NK_WINDOW_CLOSABLE|NK_WINDOW_TITLE)){
		
		rewinddir(work);
		
		while((entry = readdir(work))){
			subdir = opendir(entry->d_name);
			if (subdir != NULL){
				if (!(strcmp(entry->d_name, ".") == 0) && !(strcmp(entry->d_name, "..") == 0)){
					if (num_dirs < 10000){
						sort_dirs[num_dirs].idx = num_dirs;
						strncpy (dirs[num_dirs].name, entry->d_name, DXF_MAX_CHARS);
						stat(entry->d_name, &filestat);
						dirs[num_dirs].date = filestat.st_mtime;
						dirs[num_dirs].size = filestat.st_size;
						sort_dirs[num_dirs].data = &(dirs[num_dirs]);
						
						num_dirs++;
					}
				}
				
				closedir(subdir);
			}
			else{
				suffix = get_ext(entry->d_name);
				strncpy(ext, suffix, 4);
				ext[3] = 0; /*terminate string */
				str_upp(ext); /* upper case extension*/
				if ((strcmp(ext_type[ext_idx], "*") == 0) || (strcmp(ext, ext_type[ext_idx]) == 0)) {
					if (num_files < 10000){
						sort_files[num_files].idx = num_files;
						strncpy (files[num_files].name, entry->d_name, DXF_MAX_CHARS);
						stat(entry->d_name, &filestat);
						files[num_files].date = filestat.st_mtime;
						files[num_files].size = filestat.st_size;
						sort_files[num_files].data = &(files[num_files]);
						
						num_files++;
					}
				}
				
			}
		}
		
		static int sorted = 0;
		enum sort {
			UNSORTED,
			BY_NAME,
			BY_DATE,
			BY_SIZE
		};
		char curr_path[MAX_PATH_LEN];
		getcwd(curr_path, MAX_PATH_LEN);
		
		nk_layout_row_dynamic(gui->ctx, 20, 1);
		nk_label_colored(gui->ctx, "Current directory:", NK_TEXT_LEFT, nk_rgb(255,255,0));
		
		nk_layout_row_template_begin(gui->ctx, 22);
		nk_layout_row_template_push_dynamic(gui->ctx);
		nk_layout_row_template_push_static(gui->ctx, 30);
		nk_layout_row_template_end(gui->ctx);
		
		nk_label(gui->ctx, curr_path, NK_TEXT_LEFT);
		if (nk_button_label(gui->ctx,  "Up")){
			closedir(work);
			chdir("..");
			work = opendir(".");
			if (!work) return 0;
		}
		
		nk_layout_row_dynamic(gui->ctx, 32, 1);
		if (nk_group_begin(gui->ctx, "file_head", NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {
		
			nk_layout_row_template_begin(gui->ctx, 22);
			nk_layout_row_template_push_dynamic(gui->ctx);
			nk_layout_row_template_push_static(gui->ctx, 80);
			nk_layout_row_template_push_static(gui->ctx, 145);
			nk_layout_row_template_push_static(gui->ctx, 8);
			nk_layout_row_template_end(gui->ctx);
			
			/* sort by dir/file name */
			if (sorted == BY_NAME){
				if (nk_button_symbol_label(gui->ctx, NK_SYMBOL_TRIANGLE_DOWN, "Name", NK_TEXT_CENTERED)){
					sorted = UNSORTED;
				}
			}
			else {
				if (nk_button_label(gui->ctx,  "Name")){
					sorted = BY_NAME;
				}
			}
			
			/* sort by file/dir byte size */
			if (sorted == BY_SIZE){
				if (nk_button_symbol_label(gui->ctx, NK_SYMBOL_TRIANGLE_DOWN, "Size", NK_TEXT_CENTERED)){
					sorted = UNSORTED;
				}
			}
			else {
				if (nk_button_label(gui->ctx,  "Size")){
					sorted = BY_SIZE;
				}
			}
			
			/* sort by file/dir modification time */
			if (sorted == BY_DATE){
				if (nk_button_symbol_label(gui->ctx, NK_SYMBOL_TRIANGLE_DOWN, "Date", NK_TEXT_CENTERED)){
					sorted = UNSORTED;
				}
			}
			else {
				if (nk_button_label(gui->ctx,  "Date")){
					sorted = BY_DATE;
				}
			}
			
			nk_group_end(gui->ctx);
		}
		nk_layout_row_dynamic(gui->ctx, 300, 1);
		if (nk_group_begin(gui->ctx, "File_view", NK_WINDOW_BORDER)) {
			//nk_layout_row_dynamic(gui->ctx, 20, 2);
			nk_layout_row_template_begin(gui->ctx, 20);
			nk_layout_row_template_push_dynamic(gui->ctx);
			nk_layout_row_template_push_static(gui->ctx, 80);
			nk_layout_row_template_push_static(gui->ctx, 145);
			nk_layout_row_template_end(gui->ctx);
			
			
			if (sorted == BY_NAME){
				qsort(sort_dirs, num_dirs, sizeof(struct sort_by_idx), cmp_file_name);
				qsort(sort_files, num_files, sizeof(struct sort_by_idx), cmp_file_name);
			}
			else if (sorted == BY_DATE){
				qsort(sort_dirs, num_dirs, sizeof(struct sort_by_idx), cmp_file_date);
				qsort(sort_files, num_files, sizeof(struct sort_by_idx), cmp_file_date);
			}
			else if (sorted == BY_SIZE){
				qsort(sort_dirs, num_dirs, sizeof(struct sort_by_idx), cmp_file_name);
				qsort(sort_files, num_files, sizeof(struct sort_by_idx), cmp_file_size);
			}
			
			for (i = 0; i < num_dirs; i++){
				idx = sort_dirs[i].idx;
				if (nk_button_label_styled(gui->ctx, &b_dir,  dirs[idx].name)){
					closedir(work);
					chdir(dirs[idx].name);
					work = opendir(".");
					full_path[0] = 0;
					sel_file[0] = 0;
					if (!work) return 0;
				}
				
				/*show byte size */
				//snprintf(str_tmp, 20, "%d", dirs[idx].size);
				//nk_label_colored(gui->ctx, str_tmp, NK_TEXT_RIGHT, nk_rgb(255,255,0));
				nk_label_colored(gui->ctx, "-", NK_TEXT_CENTERED, nk_rgb(255,255,0));
				
				/* show modification date/time */
				info = localtime(&(dirs[idx].date));
				snprintf(str_tmp, 20, "%02d/%02d/%04d-%02d:%02d:%02d", info->tm_mday, info->tm_mon+1, info->tm_year+1900, info->tm_hour, info->tm_min, info->tm_sec);
				nk_label_colored(gui->ctx, str_tmp, NK_TEXT_RIGHT, nk_rgb(255,255,0));
				
			}
			for (i = 0; i < num_files; i++){
				idx = sort_files[i].idx;
				if (nk_button_label_styled(gui->ctx, &b_file,  files[idx].name)){
					strncpy(sel_file, files[idx].name, MAX_PATH_LEN);
				}
				
				/*show byte size */
				if (files[idx].size > 1048576) snprintf(str_tmp, 20, "%0.2fM", (float) files[idx].size / 1048576);
				else if (files[idx].size > 1024) snprintf(str_tmp, 20, "%0.2fK", (float) files[idx].size / 1024);
				else snprintf(str_tmp, 20, "%d", files[idx].size);
				nk_label(gui->ctx, str_tmp, NK_TEXT_RIGHT);
				
				/* show modification date/time */
				info = localtime(&(files[idx].date));
				snprintf(str_tmp, 20, "%02d/%02d/%04d-%02d:%02d:%02d", info->tm_mday, info->tm_mon+1, info->tm_year+1900, info->tm_hour, info->tm_min, info->tm_sec);
				nk_label(gui->ctx, str_tmp, NK_TEXT_RIGHT);
				
			}
			
			nk_group_end(gui->ctx);
			
			
			nk_layout_row_dynamic(gui->ctx, 20, 2);
			nk_label_colored(gui->ctx, "Selected:", NK_TEXT_LEFT, nk_rgb(0,0,255));
			
			if (nk_combo_begin_label(gui->ctx, ext_descr[ext_idx], nk_vec2(200,300))){
				nk_layout_row_dynamic(gui->ctx, 17, 1);
				for (i = 0; i < num_ext; i++){
					if (nk_button_label(gui->ctx, ext_descr[i])){
						ext_idx = i;
						nk_combo_close(gui->ctx);
					}
				}
				
				nk_combo_end(gui->ctx);
			}
			nk_layout_row_dynamic(gui->ctx, 20, 1);
			
			/* show the selected file*/
			nk_flags res; /* the user will can edit the file name */
			res = nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_FIELD|NK_EDIT_SIG_ENTER, sel_file, MAX_PATH_LEN, nk_filter_default);
			if (res & NK_EDIT_COMMITED){ /* user hit enter */
				nk_edit_unfocus(gui->ctx);
				/* the user also will can enter a new path */
				subdir = opendir(sel_file); /* try to open  the path entered by user */
				if (subdir != NULL){
					closedir(work);
					chdir(sel_file); /* change the current path */
					work = subdir;
					full_path[0] = 0;
					sel_file[0] = 0;
				}
			}
			
			nk_layout_row_dynamic(gui->ctx, 20, 2);
			if (nk_button_label(gui->ctx,  "OK")){
				if (strlen(sel_file) > 0){
					snprintf(full_path, MAX_PATH_LEN, "%s%c%s", curr_path, DIR_SEPARATOR, sel_file);
				}
				
				closedir(work);
				work = NULL;
				
				strncpy(gui->curr_path, full_path, MAX_PATH_LEN);
				
				full_path[0] = 0;
				sel_file[0] = 0;
				
				show_browser = 0;
			}
			if (nk_button_label(gui->ctx,  "Cancel")){
				
				closedir(work);
				work = NULL;
				//full_path[0] = 0;
				sel_file[0] = 0;
				
				show_browser = 0;
			}
			
		}
		
	} else {
		closedir(work);
		work = NULL;
		//full_path[0] = 0;
		sel_file[0] = 0;
		
		show_browser = 0;
	}
	nk_end(gui->ctx);
	
	return show_browser;
}

int file_pop (gui_obj *gui, enum files_types filters[], int num_filters, char *init_dir){
	//static char file_path[DXF_MAX_CHARS];
	
	int show_app_file = 1;
	/* about popup */
	static struct nk_rect s = {20, 200, 400, 150};
	if (nk_popup_begin(gui->ctx, NK_POPUP_STATIC, "File", NK_WINDOW_CLOSABLE, s)){
		nk_layout_row_dynamic(gui->ctx, 20, 1);
		nk_label(gui->ctx, "File to Open:", NK_TEXT_CENTERED);
		
		nk_edit_focus(gui->ctx, NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT);
		nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_SIMPLE | NK_EDIT_CLIPBOARD, gui->curr_path, MAX_PATH_LEN, nk_filter_default);
		
		nk_layout_row_dynamic(gui->ctx, 20, 2);
		if ((nk_button_label(gui->ctx, "OK")) && (!gui->show_file_br)) {
			nk_popup_close(gui->ctx);
			show_app_file = 2;
		}
		if (nk_button_label(gui->ctx, "Explore")) {
			int i;
			
			for (i = 0; i < num_filters; i++){
				gui->file_filter_types[i] = filter_types[filters[i]];
				gui->file_filter_descr[i] = filter_descr[filters[i]];
			}
			
			gui->file_filter_count = num_filters;
			
			gui->show_file_br = 1;
		}
		
		nk_popup_end(gui->ctx);
	} else {
		show_app_file = nk_false;
	}
	return show_app_file;
}