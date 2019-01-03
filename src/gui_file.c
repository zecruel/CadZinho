#include "gui.h"
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#define MAX_PATH_LEN 512

int file_win (gui_obj *gui){
	
	struct dirent *entry;
	static DIR *work = NULL;
	DIR *subdir;
	char ext[4], buffer[MAX_PATH_LEN], *suffix, *end, dir[] = ".";
	char date[20];
	if (!work) work = opendir(dir);
	if (!work) return 0;
	rewinddir(work);
	
	int show_browser = 1;
	int i = 0;
	
	struct stat filestat;
	time_t rawtime;
	struct tm *info;
	
	
	gui->next_win_x += gui->next_win_w + 3;
	//gui->next_win_y += gui->next_win_h + 3;
	gui->next_win_w = 400;
	gui->next_win_h = 300;
	
	if (nk_begin(gui->ctx, "File", nk_rect(gui->next_win_x, gui->next_win_y, gui->next_win_w, gui->next_win_h),
	NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
	NK_WINDOW_CLOSABLE|NK_WINDOW_TITLE)){
		//nk_layout_row_dynamic(gui->ctx, 20, 2);
		nk_layout_row_template_begin(gui->ctx, 20);
                nk_layout_row_template_push_dynamic(gui->ctx);
                nk_layout_row_template_push_static(gui->ctx, 130);
                nk_layout_row_template_end(gui->ctx);
		
		i = 1;
		strncpy(buffer, dir, MAX_PATH_LEN);
		int n = strlen(buffer);
		if (n > 0 && (buffer[n-1] != '/')) buffer[n++] = '/';
		
		while((entry = readdir(work))){
			strncpy(buffer + n, entry->d_name, MAX_PATH_LEN-n);
			subdir = opendir(buffer);
			if (subdir != NULL){
				nk_label_colored(gui->ctx, entry->d_name, NK_TEXT_LEFT, nk_rgb(255,255,0));
				stat(entry->d_name, &filestat);
				
				rawtime = filestat.st_mtime;
				info = localtime(&rawtime);
				snprintf(date, 20, "%02d/%02d/%04d %02d:%02d:%02d", info->tm_mday, info->tm_mon+1, info->tm_year+1900, info->tm_hour, info->tm_min, info->tm_sec);
				nk_label_colored(gui->ctx, date, NK_TEXT_LEFT, nk_rgb(255,255,0));
				
				closedir(subdir);
			}
			else{
				suffix = get_ext(entry->d_name);
				strncpy(ext, suffix, 4);
				ext[3] = 0; /*terminate string */
				str_upp(ext); /* upper case extension*/
				if (strcmp(ext, "DXF") == 0) {
					nk_label(gui->ctx, entry->d_name, NK_TEXT_LEFT);
					stat(entry->d_name, &filestat);
					rawtime = filestat.st_mtime;
					info = localtime(&rawtime);
					snprintf(date, 20, "%02d/%02d/%04d %02d:%02d:%02d", info->tm_mday, info->tm_mon+1, info->tm_year+1900, info->tm_hour, info->tm_min, info->tm_sec);
					nk_label(gui->ctx, date, NK_TEXT_LEFT);
				}
				
			}
		}
		
	} else {
		closedir(work);
		work = NULL;
		show_browser = 0;
	}
	nk_end(gui->ctx);
	
	return show_browser;
}