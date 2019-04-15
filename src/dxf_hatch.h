#ifndef _DXF_HACTH_LIB
#define _DXF_HACTH_LIB

#include "dxf.h"

struct hatch_line {
	double ang, ox, oy, dx, dy;
	double dash[20];
	int num_dash;
	struct hatch_line *next;
};

struct h_pattern {
	char name[DXF_MAX_CHARS];
	char descr[DXF_MAX_CHARS];
	int num_lines;
	struct hatch_line *lines;
	struct h_pattern *next;
};

struct h_family {
	char name[DXF_MAX_CHARS];
	char descr[DXF_MAX_CHARS];
	struct h_pattern *list;
	struct h_family *next;
};

enum hatch_type{
	HATCH_USER,
	HATCH_PREDEF,
	HATCH_SOLID
};

int dxf_parse_patt(char *buf, struct h_pattern *ret);
int dxf_hatch_free (struct h_pattern *hatch);
struct h_family * dxf_hatch_family(char *name, char* descr, char *buf);
struct h_family * dxf_hatch_family_file(char *name, char *path);
int dxf_h_fam_free (struct h_family *fam);

#endif