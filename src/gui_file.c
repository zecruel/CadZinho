#include "gui.h"
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#define MAX_PATH_LEN 512

int cmp_file_name(const void * a, const void * b) {
	struct dirent *entry1, *entry2;
	
	DIR *work1 = ((struct sort_by_idx *)a)->data;
	DIR *work2 = ((struct sort_by_idx *)b)->data;
	
	seekdir(work1, ((struct sort_by_idx *)a)->idx);
	entry1 = readdir(work1);
	
	seekdir(work2, ((struct sort_by_idx *)b)->idx);
	entry2 = readdir(work2);
	
	
	return (strcmp(entry1->d_name, entry2->d_name));
}


int file_win (gui_obj *gui){
	
	struct dirent *entry;
	static DIR *work = NULL;
	DIR *subdir;
	char ext[4], buffer[MAX_PATH_LEN], *suffix, *end, dir[] = ".";
	char date[20];
	if (!work) work = opendir(dir);
	if (!work) return 0;
	
	
	int show_browser = 1;
	long int i = 0;
	
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
		rewinddir(work);
		
		static int sorted = 0;
		enum sort {
			UNSORTED,
			BY_NAME,
			BY_DATE
		};
		
		//nk_layout_row_dynamic(gui->ctx, 20, 2);
		nk_layout_row_template_begin(gui->ctx, 20);
                nk_layout_row_template_push_dynamic(gui->ctx);
                nk_layout_row_template_push_static(gui->ctx, 130);
                nk_layout_row_template_end(gui->ctx);
		
		i = 1;
		strncpy(buffer, dir, MAX_PATH_LEN);
		int n = strlen(buffer);
		if (n > 0 && (buffer[n-1] != '/')) buffer[n++] = '/';
		
		static struct sort_by_idx sort_dirs[10000];
		static struct sort_by_idx sort_files[10000];
		int num_files = 0, num_dirs = 0;
		
		#if(0)
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
		#endif
		
		i = telldir(work);
		while((entry = readdir(work))){
			strncpy(buffer + n, entry->d_name, MAX_PATH_LEN-n);
			subdir = opendir(buffer);
			if (subdir != NULL){
				if (!(strcmp(entry->d_name, ".") == 0) && !(strcmp(entry->d_name, "..") == 0)){
					if (num_dirs < 10000){
						sort_dirs[num_dirs].idx = i;
						sort_dirs[num_dirs].data = work;
						
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
				if (strcmp(ext, "DXF") == 0) {
					if (num_files < 10000){
						sort_files[num_files].idx = i;
						sort_files[num_files].data = work;
						
						num_files++;
					}
				}
				
			}
			i = telldir(work);
		}
		
		sorted = BY_NAME;
		
		if (sorted == BY_NAME){
			qsort(sort_dirs, num_dirs, sizeof(struct sort_by_idx), cmp_file_name);
			qsort(sort_files, num_files, sizeof(struct sort_by_idx), cmp_file_name);
		}
		
		for (i = 0; i < num_dirs; i++){
			seekdir(work, sort_dirs[i].idx);
			entry = readdir(work);
			nk_label_colored(gui->ctx, entry->d_name, NK_TEXT_LEFT, nk_rgb(255,255,0));
			stat(entry->d_name, &filestat);
			
			rawtime = filestat.st_mtime;
			info = localtime(&rawtime);
			snprintf(date, 20, "%02d/%02d/%04d %02d:%02d:%02d", info->tm_mday, info->tm_mon+1, info->tm_year+1900, info->tm_hour, info->tm_min, info->tm_sec);
			nk_label_colored(gui->ctx, date, NK_TEXT_LEFT, nk_rgb(255,255,0));
		}
		for (i = 0; i < num_files; i++){
			seekdir(work, sort_files[i].idx);
			entry = readdir(work);
			nk_label(gui->ctx, entry->d_name, NK_TEXT_LEFT);
			stat(entry->d_name, &filestat);
			rawtime = filestat.st_mtime;
			info = localtime(&rawtime);
			snprintf(date, 20, "%02d/%02d/%04d %02d:%02d:%02d", info->tm_mday, info->tm_mon+1, info->tm_year+1900, info->tm_hour, info->tm_min, info->tm_sec);
			nk_label(gui->ctx, date, NK_TEXT_LEFT);
		}
		
	} else {
		closedir(work);
		work = NULL;
		show_browser = 0;
	}
	nk_end(gui->ctx);
	
	return show_browser;
}