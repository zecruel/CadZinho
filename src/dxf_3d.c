#include "dxf_3d.h"

#define MEMP_VEC2 4
#define MEMP_SIMPLE_POLY 5
#define MEMP_POLY 6

extern struct script_obj dxf_3d_engine;

void matrix3_mul(double *mat_a, double *mat_b, double *mat_r) {
    for (unsigned int i = 0; i < 9; i += 3)
        for (unsigned int j = 0; j < 3; ++j)
            mat_r[i + j] = (mat_b[i + 0] * mat_a[j +  0])
                          + (mat_b[i + 1] * mat_a[j + 3])
                          + (mat_b[i + 2] * mat_a[j + 6]);
}

void matrix3_sum(double *mat_a, double *mat_b, double *mat_r) {
    for (unsigned int i = 0; i < 9; i++)
        mat_r[i] = mat_a[i] + mat_b[i];
}

void matrix3_sub(double *mat_a, double *mat_b, double *mat_r) {
    for (unsigned int i = 0; i < 9; i++)
        mat_r[i] = mat_a[i] - mat_b[i];
}

void matrix3_k(double *mat_a, double k, double *mat_r) {
    for (unsigned int i = 0; i < 9; i++)
        mat_r[i] = mat_a[i] * k;
}

double scalar_prod (double v_a[3], double v_b[3]){
  return v_a[0] * v_b[0] + v_a[1] * v_b[1] + v_a[2] * v_b[2];
}

void vec_product (double v_a[3], double v_b[3], double v_r[3]){
  v_r[0] = v_a[1] * v_b[2] - v_a[2] * v_b[1];
  v_r[1] = v_a[2] * v_b[0] - v_a[0] * v_b[2];
  v_r[2] = v_a[0] * v_b[1] - v_a[1] * v_b[0];
}

void vec_sum (double v_a[3], double v_b[3], double v_r[3]){
  v_r[0] = v_a[0] + v_b[0];
  v_r[1] = v_a[1] + v_b[1];
  v_r[2] = v_a[2] + v_b[2];
}

void vec_sub (double v_a[3], double v_b[3], double v_r[3]){
  v_r[0] = v_a[0] - v_b[0];
  v_r[1] = v_a[1] - v_b[1];
  v_r[2] = v_a[2] - v_b[2];
}

void vec_k (double v_a[3], double k, double v_r[3]){
  v_r[0] = v_a[0] * k;
  v_r[1] = v_a[1] * k;
  v_r[2] = v_a[2] * k;
}

double vec_abs (double v_a[3]){
  return sqrt(v_a[0] * v_a[0] + v_a[1] * v_a[1] + v_a[2] * v_a[2]);
}

int vec_unit (double v_a[3], double v_r[3]){
  double v_abs = vec_abs(v_a);
  if (v_abs < 1e-9) return 0;
  v_r[0] = v_a[0] / v_abs;
  v_r[1] = v_a[1] / v_abs;
  v_r[2] = v_a[2] / v_abs;
  return 1;
}

int rot_matrix_vec (double v_a[3], double dir[3], double mat_r[9]){
  double a[3], b[3];
  if(!vec_unit(v_a, a)) return 0;
  if(!vec_unit(dir, b)) return 0;
  double c = scalar_prod(a, b);
  if (fabs(c + 1.0) < 1e-9) return 0;
  double v[3], I[9] = {1,0,0, 0,1,0, 0,0,1};
  
  vec_product (a, b, v);
  double V[9] = {0,-v[2],v[1], v[2],0,-v[0], -v[1],v[0],0}, V2[9];
  
  matrix3_mul(V, V, V2);
  
  matrix3_k(V2, 1/(1+c), V2);
  
  matrix3_sum(I, V, mat_r);
  matrix3_sum(mat_r, V2, mat_r);
  return 1;
}

struct manifold_obj {
	ManifoldManifold *obj;
  ManifoldManifold *prev;
};

struct path_slice {
  double x, y, z; /* current contour center offset */
  double rot_mtx[9];
};

struct path_ctx {
  int n;
  double step;
  struct path_slice *slices;
};

ManifoldVec3 path_warp (double x, double y, double z, void *ctx) {
  ManifoldVec3 v = {x, y, z};
  struct path_ctx *context = (struct path_ctx *) ctx;
  int n = context->n;
  double step = context->step;
  struct path_slice *slices = context->slices;
  
  if (step == 0.0) return v;
  int idx = fabs(round(z/step));
  if (idx > n) return v;
  double *mtx = slices[idx].rot_mtx;
  
  //v.x = x*mtx[0] + y*mtx[1] + z*mtx[2] + slices[idx].x;
  v.x = x*mtx[0] + y*mtx[1] + slices[idx].x;
  //v.y = x*mtx[3] + y*mtx[4] + z*mtx[5] + slices[idx].y;
  v.y = x*mtx[3] + y*mtx[4] + slices[idx].y;
  //v.z = x*mtx[6] + y*mtx[7] + z*mtx[8] + slices[idx].z;
  v.z = x*mtx[6] + y*mtx[7] + slices[idx].z;
  return v;
};

void *manifold_buffer() { return malloc(manifold_manifold_size()); }
void *meshgl64_buffer() {
  struct Mem_buffer *mem = manage_buffer(manifold_meshgl64_size(),
    BUF_GET, MEMP_POLY);
  if (mem) return mem->buffer;
  return NULL;
}
void *simple_polyg_buffer() {
  struct Mem_buffer *mem = manage_buffer(manifold_simple_polygon_size(), 
    BUF_GET, MEMP_SIMPLE_POLY);
  if (mem) return mem->buffer;
  return NULL;
}
void *polygons_buffer() {
  struct Mem_buffer *mem = manage_buffer(manifold_polygons_size(),
    BUF_GET, MEMP_POLY);
  if (mem) return mem->buffer;
  return NULL;
}


/* create a sphere manifold */
/* given parameters:
	- radius, as number (optional, dflt = 1)
	Returns: Manifold object, as userdata
*/
int dxf_3d_sphere (lua_State *L) {
  double r = 1.0;
  int c_seg = 32;
	
	/* verify passed arguments */
	
	if (lua_isnumber(L, 1)) r = lua_tonumber(L, 1);
  
  lua_getglobal(L, "circular_segments");
  if (lua_isnumber(L, -1)) c_seg = lua_tointeger(L, -1);
  lua_pop (L, 1);
  
  if (r <= 0.0) r = 1.0;
  if (c_seg <= 0) c_seg = 32;
	
	/* create a userdata object */
	struct manifold_obj *sphere;
	
	sphere = (struct manifold_obj *) lua_newuserdatauv(L, sizeof(struct manifold_obj), 0); 
	luaL_getmetatable(L, "Manifold");
	lua_setmetatable(L, -2);
  
  sphere->obj = NULL;
  sphere->prev = NULL;
	
  sphere->obj = manifold_sphere(manifold_buffer(), r, c_seg);
  sphere->prev = manifold_empty(manifold_buffer());
	
	return 1;
}

/* create a slab manifold */
/* given parameters:
	- edge lengths w,h,p, as numbers (dflt = 1,1,1)
	Returns: Manifold object, as userdata
*/
int dxf_3d_slab (lua_State *L) {
  double w = 1.0, h = 1.0, p = 1.0;
	
	/* verify passed arguments */
	
	if (lua_isnumber(L, 1)) w = lua_tonumber(L, 1);
  if (lua_isnumber(L, 2)) h = lua_tonumber(L, 2);
  if (lua_isnumber(L, 3)) p = lua_tonumber(L, 3);
  
  if (w <= 0.0) w = 1.0;
  if (h <= 0.0) h = 1.0;
  if (p <= 0.0) p = 1.0;
	
	/* create a userdata object */
	struct manifold_obj *slab;
	
	slab = (struct manifold_obj *) lua_newuserdatauv(L, sizeof(struct manifold_obj), 0); 
	luaL_getmetatable(L, "Manifold");
	lua_setmetatable(L, -2);
  
  slab->obj = NULL;
  slab->prev = NULL;
	
  
  //ManifoldVec2 pts[] = {{0, 0}, {w, 0}, {w, h}, {0, h}};
  ManifoldVec2 pts[] = {{-w/2.0, -h/2.0}, {w/2.0, -h/2.0}, {w/2.0, h/2.0}, {-w/2.0, h/2.0}};
  ManifoldSimplePolygon *sq[] = {
      manifold_simple_polygon(simple_polyg_buffer(), &pts[0], 4)};
  ManifoldPolygons *polys = manifold_polygons(polygons_buffer(), sq, 1);

  slab->obj = manifold_extrude(manifold_buffer(), polys, p, 0, 0, 1.0, 1.0);
  slab->prev = manifold_empty(manifold_buffer());
  
  manifold_destruct_simple_polygon(sq[0]);
  manifold_destruct_polygons(polys);
  
  manage_buffer(0, BUF_RELEASE, MEMP_SIMPLE_POLY);
  manage_buffer(0, BUF_RELEASE, MEMP_POLY);
	
	return 1;
}

/* create a cylinder manifold */
/* given parameters:
  - heigth, as number (optional, dflt = 1)
  - radius, as number (optional, dflt = 1)
	Returns: Manifold object, as userdata
*/
int dxf_3d_cylinder (lua_State *L) {
  double h = 1.0, r = 1.0;
  int c_seg = 32;
	
	/* verify passed arguments */
	
  if (lua_isnumber(L, 1)) h = lua_tonumber(L, 1);
  if (lua_isnumber(L, 2)) r = lua_tonumber(L, 2);
  
  lua_getglobal(L, "circular_segments");
  if (lua_isnumber(L, -1)) c_seg = lua_tointeger(L, -1);
  lua_pop (L, 1);
  
  if (r <= 0.0) r = 1.0;
  if (h <= 0.0) h = 1.0;
  if (c_seg <= 0) c_seg = 32;
	
	/* create a userdata object */
	struct manifold_obj *cylinder;
	
	cylinder = (struct manifold_obj *) lua_newuserdatauv(L, sizeof(struct manifold_obj), 0); 
	luaL_getmetatable(L, "Manifold");
	lua_setmetatable(L, -2);
  
  cylinder->obj = NULL;
  cylinder->prev = NULL;
	
  cylinder->obj = manifold_cylinder(manifold_buffer(), h, r, r, c_seg, 0);
  cylinder->prev = manifold_empty(manifold_buffer());
	
	return 1;
}

/* create a cone manifold */
/* given parameters:
  - heigth, as number (optional, dflt = 1)
  - bottom radius, as number (optional, dflt = 1)
	- top radius, as number (optional, dflt = 0 - sharp cone)
	Returns: Manifold object, as userdata
*/
int dxf_3d_cone (lua_State *L) {
  double h = 1.0, r1 = 1.0, r2 = 0.0;
  int c_seg = 32;
	
	/* verify passed arguments */
	
  if (lua_isnumber(L, 1)) h = lua_tonumber(L, 1);
  if (lua_isnumber(L, 2)) r1 = lua_tonumber(L, 2);
  if (lua_isnumber(L, 3)) r2 = lua_tonumber(L, 3);
  
  lua_getglobal(L, "circular_segments");
  if (lua_isnumber(L, -1)) c_seg = lua_tointeger(L, -1);
  lua_pop (L, 1);
  
  if (r1 <= 0.0) r1 = 1.0;
  if (h <= 0.0) h = 1.0;
  if (r2 <= 0.0) r2 = 0.0;
  if (c_seg <= 0) c_seg = 32;
	
	/* create a userdata object */
	struct manifold_obj *cone;
	
	cone = (struct manifold_obj *) lua_newuserdatauv(L, sizeof(struct manifold_obj), 0); 
	luaL_getmetatable(L, "Manifold");
	lua_setmetatable(L, -2);
  
  cone->obj = NULL;
  cone->prev = NULL;
	
  cone->obj = manifold_cylinder(manifold_buffer(), h, r1, r2, c_seg, 0);
  cone->prev = manifold_empty(manifold_buffer());
	
	return 1;
}

/* create a pyramid manifold */
/* given parameters:
	- base edge lengths w,h, as numbers (dflt = 1,1)
  - pyramid total heigth p, as number (dflt = 1)
  - top scale factor relative to base, as number (dflt = 0 - complete pyramid)
	Returns: Manifold object, as userdata
*/
int dxf_3d_pyramid (lua_State *L) {
  double w = 1.0, h = 1.0, p = 1.0, s = 0.0;
	
	/* verify passed arguments */
	
	if (lua_isnumber(L, 1)) w = lua_tonumber(L, 1);
  if (lua_isnumber(L, 2)) h = lua_tonumber(L, 2);
  if (lua_isnumber(L, 3)) p = lua_tonumber(L, 3);
  if (lua_isnumber(L, 4)) s = lua_tonumber(L, 4);
  
  if (w <= 0.0) w = 1.0;
  if (h <= 0.0) h = 1.0;
  if (p <= 0.0) p = 1.0;
  if (s < 0.0) s = 0.0;
	
	/* create a userdata object */
	struct manifold_obj *pyr;
	
	pyr = (struct manifold_obj *) lua_newuserdatauv(L, sizeof(struct manifold_obj), 0); 
	luaL_getmetatable(L, "Manifold");
	lua_setmetatable(L, -2);
  
  pyr->obj = NULL;
  pyr->prev = NULL;
	
  
  //ManifoldVec2 pts[] = {{0, 0}, {w, 0}, {w, h}, {0, h}};
  ManifoldVec2 pts[] = {{-w/2.0, -h/2.0}, {w/2.0, -h/2.0}, {w/2.0, h/2.0}, {-w/2.0, h/2.0}};
  ManifoldSimplePolygon *sq[] = {
      manifold_simple_polygon(simple_polyg_buffer(), &pts[0], 4)};
  ManifoldPolygons *polys = manifold_polygons(polygons_buffer(), sq, 1);

  pyr->obj = manifold_extrude(manifold_buffer(), polys, p, 0, 0, s, s);
  pyr->prev = manifold_empty(manifold_buffer());
  
  manifold_destruct_simple_polygon(sq[0]);
  manifold_destruct_polygons(polys);
  
  manage_buffer(0, BUF_RELEASE, MEMP_SIMPLE_POLY);
  manage_buffer(0, BUF_RELEASE, MEMP_POLY);

	
	return 1;
}

/* create a wedge manifold */
/* given parameters:
	- base edge lengths w,h, as numbers (dflt = 1,1)
  - pyramid total heigth p, as number (dflt = 1)
  - top scale xy factors relative to base, as numbers (dflt = 1,0 - sharp wedge)
	Returns: Manifold object, as userdata
*/
int dxf_3d_wedge (lua_State *L) {
  double w = 1.0, h = 1.0, p = 1.0, sx = 1.0, sy = 0.0;
  
	/* verify passed arguments */
	
	if (lua_isnumber(L, 1)) w = lua_tonumber(L, 1);
  if (lua_isnumber(L, 2)) h = lua_tonumber(L, 2);
  if (lua_isnumber(L, 3)) p = lua_tonumber(L, 3);
  if (lua_isnumber(L, 4)) sx = lua_tonumber(L, 4);
  if (lua_isnumber(L, 5)) sy = lua_tonumber(L, 5);
  
  if (w <= 0.0) w = 1.0;
  if (h <= 0.0) h = 1.0;
  if (p <= 0.0) p = 1.0;
  if (sx < 0.0) sx = 0.0;
  if (sy < 0.0) sy = 0.0;
  
	/* create a userdata object */
	struct manifold_obj *wedge;
	
	wedge = (struct manifold_obj *) lua_newuserdatauv(L, sizeof(struct manifold_obj), 0); 
	luaL_getmetatable(L, "Manifold");
	lua_setmetatable(L, -2);
  
  wedge->obj = NULL;
  wedge->prev = NULL;
	
  
  //ManifoldVec2 pts[] = {{0, 0}, {w, 0}, {w, h}, {0, h}};
  ManifoldVec2 pts[] = {{-w/2.0, -h/2.0}, {w/2.0, -h/2.0}, {w/2.0, h/2.0}, {-w/2.0, h/2.0}};
  ManifoldSimplePolygon *sq[] = {
      manifold_simple_polygon(simple_polyg_buffer(), &pts[0], 4)};
  ManifoldPolygons *polys = manifold_polygons(polygons_buffer(), sq, 1);

  wedge->obj = manifold_extrude(manifold_buffer(), polys, p, 0, 0, sx, sy);
  wedge->prev = manifold_empty(manifold_buffer());
  
  manifold_destruct_simple_polygon(sq[0]);
  manifold_destruct_polygons(polys);
  
  manage_buffer(0, BUF_RELEASE, MEMP_SIMPLE_POLY);
  manage_buffer(0, BUF_RELEASE, MEMP_POLY);

	
	return 1;
}

/* create a torus manifold */
/* given parameters:
	- torus radius, as number (optional, dflt = 1)
	- profile radius, as number (optional, dflt = 0.5)
  - revolve angle, as number (dflt = 360 - full torus)
  - start angle of profile, as number (dflt = -180 - full circle)
  - end angle of profile, as number (dflt = 180 - full circle)
	Returns: Manifold object, as userdata
*/
int dxf_3d_torus (lua_State *L) {
  double tr = 1.0, pr = 0.5, ang = 360.0;
  double ang_start = 0.0, ang_end = 360.0;
	
  int c_seg = 32, i, steps;
  
  lua_getglobal(L, "circular_segments");
  if (lua_isnumber(L, -1)) c_seg = lua_tointeger(L, -1);
  lua_pop (L, 1);
  
	/* verify passed arguments */
	
	if (lua_isnumber(L, 1)) tr = lua_tonumber(L, 1);
  if (lua_isnumber(L, 2)) pr = lua_tonumber(L, 2);
  if (lua_isnumber(L, 3)) ang = lua_tonumber(L, 3);
  if (lua_isnumber(L, 4)) ang_start = lua_tonumber(L, 4);
  if (lua_isnumber(L, 5)) ang_end = lua_tonumber(L, 5);
  
  if (tr <= 0.0) tr = 1.0;
  if (pr <= 0.0) pr = 0.5;
  if (ang <= 0.0) ang = 360.0;
  else if (ang > 360.0) ang = 360.0;
  if (ang_start < 0.0) ang_start = 0.0;
  else if (ang_start > 360.0) ang_start = 360.0;
  if (ang_end < 0.0) ang_end = 0.0;
  else if (ang_end > 360.0) ang_end = 360.0;
  
  if (fabs(ang_end - ang_start) < 1e-9){
    ang_end = 0.0;
    ang_start = 360.0;
  }
  if (c_seg <= 0) c_seg = 32;

  ManifoldVec2 pts[1002];
  
  /* get step increment in loop */
  double step = 360.0 /(double) c_seg;
  
  steps = fabs(ang_end - ang_start)/step;
  
  if (steps > 1000){
    steps = 1000;
    step = fabs(ang_end - ang_start)/1000.0;
  }
  
  ang_start *= M_PI/180;
  ang_end *= M_PI/180;
  step *= M_PI/180;
  
  /* first point */
  pts[0].x = tr + pr * cos(ang_start);
  pts[0].y = pr * sin(ang_start);
  
  /* starts loop at second point */
  for (i = 1; i < steps; i++){
    pts[i].x = tr + pr * cos(step * i + ang_start);
    pts[i].y = pr * sin(step * i + ang_start);
  }
  /* last point, from end angle */
  if (fabs(ang_end - ang_start)+1e-6 < 2 * M_PI){
    pts[steps].x = tr + pr * cos(ang_end);
    pts[steps].y = pr * sin(ang_end);
    steps++;
    pts[steps].x = tr;
    pts[steps].y = 0.0;
    steps++;
  }
	
	/* create a userdata object */
	struct manifold_obj *torus;
	
	torus = (struct manifold_obj *) lua_newuserdatauv(L, sizeof(struct manifold_obj), 0); 
	luaL_getmetatable(L, "Manifold");
	lua_setmetatable(L, -2);
  
  torus->obj = NULL;
  torus->prev = NULL;
	
  ManifoldSimplePolygon *prof[] = {
      manifold_simple_polygon(simple_polyg_buffer(), &pts[0], steps)};
  ManifoldPolygons *polys = manifold_polygons(polygons_buffer(), prof, 1);

  torus->obj = manifold_revolve(manifold_buffer(), polys, c_seg, ang);
  torus->prev = manifold_empty(manifold_buffer());
  
  manifold_destruct_simple_polygon(prof[0]);
  manifold_destruct_polygons(polys);
  
  manage_buffer(0, BUF_RELEASE, MEMP_SIMPLE_POLY);
  manage_buffer(0, BUF_RELEASE, MEMP_POLY);

	
	return 1;
}

/* create a extruded manifold */
/* given parameters:
	- contours polygons points, as table of tables(3x)
  - total heigth h, as number (dflt = 1)
  - top scale factors (x and y) relative to base, as numbers (dflt = 1,1 - parallel)
  - slices, as integer (dflt = 0)
  - twist degrees angle, as number (dflt = 0)
	Returns: Manifold object, as userdata
*/
int dxf_3d_extrude (lua_State *L) {
  double h = 1.0, sx = 1.0, sy = 1.0, twist = 0.0;
	int i = 0, j = 0, k = 0 , slices = 0, current = 0;
  ManifoldSimplePolygon *contour[100];
  int poly_sz = manifold_simple_polygon_size();
  struct Mem_buffer *mem_poly = manage_buffer(poly_sz * 100, 
    BUF_GET, MEMP_SIMPLE_POLY);
  if (!mem_poly){
    lua_pushnil(L); /* return fail */
    return 1;
  }
  
	/* verify passed arguments */
	if (!lua_istable(L, 1)) {
    manage_buffer(0, BUF_RELEASE, MEMP_SIMPLE_POLY);
    lua_pushnil(L); /* return fail */
    return 1;
  }
  
  /* get size of contours table */
  int n_loops = lua_rawlen(L, 1);
	if (n_loops < 1){
    manage_buffer(0, BUF_RELEASE, MEMP_SIMPLE_POLY);
    lua_pushnil(L); /* return fail */
    return 1;
  }
  
  /* organize the contours to form correct polygons (with holes) */
  lua_getglobal(L, "check_poly_loops");
  lua_pushvalue(L, 1);
  lua_call(L, 1, 1);
  
  /* get passed arguments*/
  if (lua_isnumber(L, 2)) h = lua_tonumber(L, 2);
  if (lua_isnumber(L, 3)) sx = lua_tonumber(L, 3);
  if (lua_isnumber(L, 4)) sy = lua_tonumber(L, 4);
  if (lua_isnumber(L, 5)) slices = lua_tointeger(L, 5);
  if (lua_isnumber(L, 6)) twist = lua_tonumber(L, 6);
  
  /* iterate over contours table */
  for (k = 0; k < n_loops; k++) {
    lua_rawgeti(L, 1, k + 1);
    if (lua_istable(L, -1)) {
      /* get points from current contour */
      int n_pts = lua_rawlen(L, -1);
      if (n_pts > 2){
       struct Mem_buffer *mem_pts = manage_buffer(n_pts * sizeof(ManifoldVec2),
          BUF_GET, MEMP_VEC2);
        if (!mem_pts){
          manage_buffer(0, BUF_RELEASE, MEMP_SIMPLE_POLY);
          lua_pushnil(L); /* return fail */
          return 1;
        }
        ManifoldVec2 *pts = (ManifoldVec2 *) mem_pts->buffer;
        /* iterate over points table */
        for (i = 0; i < n_pts; i++) {
          lua_rawgeti(L, -1, i + 1);
          if (lua_istable(L, -1)) {
            int n = lua_rawlen(L, -1);
            if (n > 1) {
              for (j = 1; j <=2; j++){
                /* get each coordinate value */
                lua_rawgeti(L, -1, j);
                if (lua_isnumber(L, -1)) {
                  if (j == 1) pts[i].x = lua_tonumber(L, -1);
                  else pts[i].y = lua_tonumber(L, -1);
                } else {
                  if (j == 1) pts[i].x = 0.0;
                  else pts[i].y = 0.0;
                }
                lua_pop (L, 1);
              }
            }
          }
          lua_pop (L, 1);
        }
        /* generate current simple polygon and add to list */
        contour[current] = manifold_simple_polygon(
          mem_poly->buffer+(poly_sz * current), pts, n_pts);
        if (current < 99) current++;
      }
      manage_buffer(0, BUF_RELEASE, MEMP_VEC2);
    }
    lua_pop (L, 1);
  }
  
  /* verify passed arguments - minimal and default parameters */
  if (h <= 0.0) h = 1.0;
  if (sx < 0.0) sx = 0.0;
  if (sy < 0.0) sy = 0.0;
  if (slices < 0) slices = 0;
	
	/* create a userdata object - Manifold*/
	struct manifold_obj *extr;
	extr = (struct manifold_obj *) lua_newuserdatauv(L, sizeof(struct manifold_obj), 0); 
	luaL_getmetatable(L, "Manifold");
	lua_setmetatable(L, -2);
  extr->obj = NULL;
  extr->prev = NULL;
	
  /* convert list of simple polygons to consolidated profile polygon to extrude */
  ManifoldPolygons *polys = manifold_polygons(polygons_buffer(), contour, current);
  
  /* generate final Manifold */
  extr->obj = manifold_extrude(manifold_buffer(), polys, h, slices, twist, sx, sy);
  extr->prev = manifold_empty(manifold_buffer());
  
  /* release allocated resources */
  for (i = 0; i < current; i++){
    manifold_destruct_simple_polygon(contour[i]);
  }
  manifold_destruct_polygons(polys);
  manage_buffer(0, BUF_RELEASE, MEMP_SIMPLE_POLY);
  manage_buffer(0, BUF_RELEASE, MEMP_POLY);
  manage_buffer(0, BUF_RELEASE, MEMP_VEC2);
  
	return 1;
}

/* create a extruded manifold along the path */
/* given parameters:
	- contours polygons points, as table of tables(3x)
  - path points, as table of tables
  - top scale factors (x and y) relative to base, as numbers (dflt = 1,1 - parallel)
  - twist degrees angle, as number (dflt = 0)
	Returns: Manifold object, as userdata
*/
int dxf_3d_extrude_path (lua_State *L) {
  double sx = 1.0, sy = 1.0, twist = 0.0;
	int i = 0, j = 0, k = 0 , slices = 0, current = 0;
  ManifoldSimplePolygon *contour[100];
  int poly_sz = manifold_simple_polygon_size();
  struct Mem_buffer *mem_poly = manage_buffer(poly_sz * 100, 
    BUF_GET, MEMP_SIMPLE_POLY);
  if (!mem_poly){
    lua_pushnil(L); /* return fail */
    return 1;
  }
  
	/* verify passed arguments */
	if (!lua_istable(L, 1)) {
    manage_buffer(0, BUF_RELEASE, MEMP_SIMPLE_POLY);
    lua_pushnil(L); /* return fail */
    return 1;
  }
  
  /* get size of contours table */
  int n_loops = lua_rawlen(L, 1);
	if (n_loops < 1){
    manage_buffer(0, BUF_RELEASE, MEMP_SIMPLE_POLY);
    lua_pushnil(L); /* return fail */
    return 1;
  }
  
  if (!lua_istable(L, 2)) {
    manage_buffer(0, BUF_RELEASE, MEMP_SIMPLE_POLY);
    lua_pushnil(L); /* return fail */
    return 1;
  }
  /* get size of path points table */
  int n_path = lua_rawlen(L, 2);
	if (n_path < 2){
    manage_buffer(0, BUF_RELEASE, MEMP_SIMPLE_POLY);
    lua_pushnil(L); /* return fail */
    return 1;
  }
  
  /* organize the contours to form correct polygons (with holes) */
  lua_getglobal(L, "check_poly_loops");
  lua_pushvalue(L, 1);
  lua_call(L, 1, 1);
  
  slices = n_path - 2;
  
  /* get passed arguments*/
  if (lua_isnumber(L, 3)) sx = lua_tonumber(L, 3);
  if (lua_isnumber(L, 4)) sy = lua_tonumber(L, 4);
  if (lua_isnumber(L, 5)) twist = lua_tonumber(L, 5);
  int t1, t2, t3;
  
  /* iterate over contours table */
  for (k = 0; k < n_loops; k++) {
    lua_rawgeti(L, 1, k + 1);
    if (lua_istable(L, -1)) {
      /* get points from current contour */
      int n_pts = lua_rawlen(L, -1);
      if (n_pts > 2){
       struct Mem_buffer *mem_pts = manage_buffer(n_pts * sizeof(ManifoldVec2),
          BUF_GET, MEMP_VEC2);
        if (!mem_pts){
          manage_buffer(0, BUF_RELEASE, MEMP_SIMPLE_POLY);
          lua_pushnil(L); /* return fail */
          return 1;
        }
        ManifoldVec2 *pts = (ManifoldVec2 *) mem_pts->buffer;
        /* iterate over points table */
        for (i = 0; i < n_pts; i++) {
          lua_rawgeti(L, -1, i + 1);
          if (lua_istable(L, -1)) {
            int n = lua_rawlen(L, -1);
            if (n > 1) {
              for (j = 1; j <=2; j++){
                /* get each coordinate value */
                lua_rawgeti(L, -1, j);
                if (lua_isnumber(L, -1)) {
                  if (j == 1) pts[i].x = lua_tonumber(L, -1);
                  else pts[i].y = lua_tonumber(L, -1);
                } else {
                  if (j == 1) pts[i].x = 0.0;
                  else pts[i].y = 0.0;
                }
                lua_pop (L, 1);
              }
            }
          }
          lua_pop (L, 1);
        }
        /* generate current simple polygon and add to list */
        contour[current] = manifold_simple_polygon(
          mem_poly->buffer+(poly_sz * current), pts, n_pts);
        if (current < 99) current++;
      }
      manage_buffer(0, BUF_RELEASE, MEMP_VEC2);
    }
    lua_pop (L, 1);
  }
  
  /* verify passed arguments - minimal and default parameters */
  if (sx < 0.0) sx = 0.0;
  if (sy < 0.0) sy = 0.0;
  if (slices < 0) slices = 0;
	
	/* create a userdata object - Manifold*/
	struct manifold_obj *extr;
	extr = (struct manifold_obj *) lua_newuserdatauv(L, sizeof(struct manifold_obj), 0); 
	luaL_getmetatable(L, "Manifold");
	lua_setmetatable(L, -2);
  extr->obj = NULL;
  extr->prev = NULL;
	
  /* convert list of simple polygons to consolidated profile polygon to extrude */
  ManifoldPolygons *polys = manifold_polygons(polygons_buffer(), contour, current);
  
  /* generate Manifold to be warped */
  extr->prev = manifold_extrude(manifold_buffer(), polys, 100.0, slices, twist, sx, sy);
  
  /* release allocated resources */
  for (i = 0; i < current; i++){
    manifold_destruct_simple_polygon(contour[i]);
  }
  manifold_destruct_polygons(polys);
  manage_buffer(0, BUF_RELEASE, MEMP_SIMPLE_POLY);
  manage_buffer(0, BUF_RELEASE, MEMP_POLY);
  manage_buffer(0, BUF_RELEASE, MEMP_VEC2);
  
  /* now, the extruded manifold will be deformed to fit along the path */
  
  /* alloc resources to path */
  struct Mem_buffer *mem_sl = manage_buffer(sizeof(struct path_slice) * n_path,
    BUF_GET, MEMP_VEC2);
  double x0 = 0.0, y0 = 0.0, z0 = 0.0;
  double normal[3] = {0.0,0.0,1.0};
  double dir[3] = {0.0,0.0,1.0};
  if (!mem_sl) {
    manage_buffer(0, BUF_RELEASE, MEMP_VEC2);
    lua_pushnil(L); /* return fail */
    return 1;
  }
  
  /* informations to pass to warp function - locate points along z axis */
  struct path_ctx ctx;
  ctx.n = n_path-1;
  ctx.step = 100.0/(n_path-1);
  ctx.slices = (struct path_slice*) mem_sl->buffer;
  
  /* iterate over path table to get points */
  for (i = 0; i < n_path; i++) {
    /* current point */
    lua_rawgeti(L, 2, i + 1);
    if (!lua_istable(L, -1)) {
      manage_buffer(0, BUF_RELEASE, MEMP_VEC2);
      lua_pushnil(L); /* return fail */
      return 1;
    }
    int n = lua_rawlen(L, -1);
    if (n < 3) { /* fail if point coordinates are < 3 */
      manage_buffer(0, BUF_RELEASE, MEMP_VEC2);
      lua_pushnil(L); /* return fail */
      return 1;
    }
    /* get point coordinates - relative to start point*/
    for (j = 1; j <= 3; j++){
      lua_rawgeti(L, -1, j);
      if (!lua_isnumber(L, -1)) {
        manage_buffer(0, BUF_RELEASE, MEMP_VEC2);
        lua_pushnil(L); /* return fail */
        return 1;
      }
      /* start point */
      if (i == 0){
        if (j == 1) {
          ctx.slices[i].z = 0.0;
          z0 = lua_tonumber(L, -1);
        }
        else if (j == 2) {
          ctx.slices[i].y = 0.0;
          y0 = lua_tonumber(L, -1);
        }
        else {
          ctx.slices[i].x = 0.0;
          x0 = lua_tonumber(L, -1);
        }
      }
      else {
        if (j == 1) ctx.slices[i].z = lua_tonumber(L, -1) - z0;
        else if (j == 2) ctx.slices[i].y = lua_tonumber(L, -1) - y0;
        else ctx.slices[i].x = lua_tonumber(L, -1) - x0;
      }
      lua_pop (L, 1);
    }
    /* rotate profile along the path */
    if (i == 1){ /* first point - rotation follows the segment */
      dir[0] = ctx.slices[i].x - ctx.slices[i-1].x;
      dir[1] = ctx.slices[i].y - ctx.slices[i-1].y;
      dir[2] = ctx.slices[i].z - ctx.slices[i-1].z;
      vec_unit (dir, dir);
      rot_matrix_vec (normal, dir, ctx.slices[i-1].rot_mtx);
    } else if (i > 1){ /* inner points - "average" rotation between segments */
      double d1[3], d2[3];
      
      d1[0] = ctx.slices[i].x - ctx.slices[i-1].x;
      d1[1] = ctx.slices[i].y - ctx.slices[i-1].y;
      d1[2] = ctx.slices[i].z - ctx.slices[i-1].z;
      
      d2[0] = ctx.slices[i-1].x - ctx.slices[i-2].x;
      d2[1] = ctx.slices[i-1].y - ctx.slices[i-2].y;
      d2[2] = ctx.slices[i-1].z - ctx.slices[i-2].z;
      
      vec_unit (d1, d1);
      vec_unit (d2, d2);
      
      vec_sum (d1, d2, dir);
      vec_unit (dir, dir);
      rot_matrix_vec (normal, dir, ctx.slices[i-1].rot_mtx);
    }
    if (i == n_path - 1){ /* last point - rotation follows the segment */
      dir[0] = ctx.slices[i].x - ctx.slices[i-1].x;
      dir[1] = ctx.slices[i].y - ctx.slices[i-1].y;
      dir[2] = ctx.slices[i].z - ctx.slices[i-1].z;
      vec_unit (dir, dir);
      rot_matrix_vec (normal, dir, ctx.slices[i].rot_mtx);
    }
    lua_pop (L, 1);
  }
  
  /* finally, warp the extruded manifold */
  extr->obj = manifold_warp(manifold_buffer(), extr->prev, path_warp, (void *) &ctx);
  
  /* release allocated resources */
  manage_buffer(0, BUF_RELEASE, MEMP_VEC2);
	
	return 1;
}

/* Manifold union operation */
/* given parameters:
	- Manifold object "a", as userdata
  - Manifold object "b", as userdata
returns:
	- Manifold object "a+b", as userdata
*/
int dxf_3d_union (lua_State *L) {
	
	struct manifold_obj *a, *b;
	
	/* verify passed arguments */
	int n = lua_gettop(L);    /* number of arguments */
	if (n < 2){
		lua_pushnil(L); /* return error */
    return 1;
	}
	if (!( a = udata_check(L, 1, "Manifold") )) { /* the Manifold object is a Lua userdata type*/
		lua_pushnil(L); /* return error */
    return 1;
	}
  if (!( b = udata_check(L, 2, "Manifold") )) { /* the Manifold object is a Lua userdata type*/
		lua_pushnil(L); /* return error */
    return 1;
	}
  
  if (a->obj == NULL){
		lua_pushnil(L); /* return error */
    return 1;
	}
  
  if (b->obj == NULL){
		lua_pushnil(L); /* return error */
    return 1;
	}
	
	/* create a userdata object */
	struct manifold_obj *uni;
	
	uni = (struct manifold_obj *) lua_newuserdatauv(L, sizeof(struct manifold_obj), 0); 
	luaL_getmetatable(L, "Manifold");
	lua_setmetatable(L, -2);
  
  uni->obj = NULL;
  uni->prev = NULL;
	
  uni->obj = manifold_union(manifold_buffer(), a->obj, b->obj);
  uni->prev = manifold_empty(manifold_buffer());
  
	
	return 1;
}

/* Manifold difference operation */
/* given parameters:
	- Manifold object "a", as userdata
  - Manifold object "b", as userdata
returns:
	- Manifold object "a-b", as userdata
*/
int dxf_3d_difference (lua_State *L) {
	
	struct manifold_obj *a, *b;
	
	/* verify passed arguments */
	int n = lua_gettop(L);    /* number of arguments */
	if (n < 2){
		lua_pushnil(L); /* return error */
    return 1;
	}
	if (!( a = udata_check(L, 1, "Manifold") )) { /* the Manifold object is a Lua userdata type*/
		lua_pushnil(L); /* return error */
    return 1;
	}
  if (!( b = udata_check(L, 2, "Manifold") )) { /* the Manifold object is a Lua userdata type*/
		lua_pushnil(L); /* return error */
    return 1;
	}
  
  if (a->obj == NULL){
		lua_pushnil(L); /* return error */
    return 1;
	}
  
  if (b->obj == NULL){
		lua_pushnil(L); /* return error */
    return 1;
	}
	
	/* create a userdata object */
	struct manifold_obj *dif;
	
	dif = (struct manifold_obj *) lua_newuserdatauv(L, sizeof(struct manifold_obj), 0); 
	luaL_getmetatable(L, "Manifold");
	lua_setmetatable(L, -2);
  
  dif->obj = NULL;
  dif->prev = NULL;
	
  dif->obj = manifold_difference(manifold_buffer(), a->obj, b->obj);
  dif->prev = manifold_empty(manifold_buffer());
  
	
	return 1;
}

/* Manifold difference operation */
/* given parameters:
	- Manifold object "a", as userdata
  - Manifold object "b", as userdata
returns:
	- Manifold object "a-b", as userdata
*/
int dxf_3d_intersection (lua_State *L) {
	
	struct manifold_obj *a, *b;
	
	/* verify passed arguments */
	int n = lua_gettop(L);    /* number of arguments */
	if (n < 2){
		lua_pushnil(L); /* return error */
    return 1;
	}
	if (!( a = udata_check(L, 1, "Manifold") )) { /* the Manifold object is a Lua userdata type*/
		lua_pushnil(L); /* return error */
    return 1;
	}
  if (!( b = udata_check(L, 2, "Manifold") )) { /* the Manifold object is a Lua userdata type*/
		lua_pushnil(L); /* return error */
    return 1;
	}
  
  if (a->obj == NULL){
		lua_pushnil(L); /* return error */
    return 1;
	}
  
  if (b->obj == NULL){
		lua_pushnil(L); /* return error */
    return 1;
	}
	
	/* create a userdata object */
	struct manifold_obj *inter;
	
	inter = (struct manifold_obj *) lua_newuserdatauv(L, sizeof(struct manifold_obj), 0); 
	luaL_getmetatable(L, "Manifold");
	lua_setmetatable(L, -2);
  
  inter->obj = NULL;
  inter->prev = NULL;
	
  inter->obj = manifold_intersection(manifold_buffer(), a->obj, b->obj);
  inter->prev = manifold_empty(manifold_buffer());
	
	return 1;
}

/* modify a manifold object - translate */
/* given parameters:
	- a Manifold object (this function is called as method inside object)
  - x,y,z values to move, as numbers (dflt = 0,0,0)
returns:
	- a boolean indicating success or fail
*/
int dxf_3d_translate (lua_State *L) {
	
	struct manifold_obj * manifold;
  double x = 0.0, y = 0.0, z = 0.0;
	
	/* verify passed arguments */
	int n = lua_gettop(L);    /* number of arguments */
	if (n < 1){
		lua_pushboolean(L, 0); /* return fail */
    return 1;
	}
	if (!( manifold = udata_check(L, 1, "Manifold") )) { /* the Manifold object is a Lua userdata type*/
		lua_pushboolean(L, 0); /* return fail */
    return 1;
	}
  
  if (lua_isnumber(L, 2)) x = lua_tonumber(L, 2);
  if (lua_isnumber(L, 3)) y = lua_tonumber(L, 3);
  if (lua_isnumber(L, 4)) z = lua_tonumber(L, 4);
	
	/* check if it is not destroyed */
	if (manifold->obj == NULL){
    lua_pushboolean(L, 0); /* return fail */
    return 1;
  }
  /* check if it is not destroyed */
	if (manifold->prev == NULL){
    lua_pushboolean(L, 0); /* return fail */
    return 1;
  }
	
  ManifoldManifold *tmp = manifold->obj;
  manifold->prev = manifold_translate(manifold->prev, manifold->obj, x, y, z);
  manifold->obj = manifold->prev;
  manifold->prev = tmp;
  
	lua_pushboolean(L, 1); /* return success */
	return 1;
}

/* modify a manifold object - rotate */
/* given parameters:
	- a Manifold object (this function is called as method inside object)
  - x,y,z angle values in degrees, as numbers (dflt = 0,0,0)
  - pivot point x,y,z values, as numbers (dflt = 0,0,0)
returns:
	- a boolean indicating success or fail
*/
int dxf_3d_rotate (lua_State *L) {
	
	struct manifold_obj * manifold;
  double ax = 0.0, ay = 0.0, az = 0.0;
  double px = 0.0, py = 0.0, pz = 0.0;
	
	/* verify passed arguments */
	int n = lua_gettop(L);    /* number of arguments */
	if (n < 1){
		lua_pushboolean(L, 0); /* return fail */
    return 1;
	}
	if (!( manifold = udata_check(L, 1, "Manifold") )) { /* the Manifold object is a Lua userdata type*/
		lua_pushboolean(L, 0); /* return fail */
    return 1;
	}
  
  if (lua_isnumber(L, 2)) ax = lua_tonumber(L, 2);
  if (lua_isnumber(L, 3)) ay = lua_tonumber(L, 3);
  if (lua_isnumber(L, 4)) az = lua_tonumber(L, 4);
  if (lua_isnumber(L, 5)) px = lua_tonumber(L, 5);
  if (lua_isnumber(L, 6)) py = lua_tonumber(L, 6);
  if (lua_isnumber(L, 7)) pz = lua_tonumber(L, 7);
	
	/* check if it is not destroyed */
	if (manifold->obj == NULL){
    lua_pushboolean(L, 0); /* return fail */
    return 1;
  }
  /* check if it is not destroyed */
	if (manifold->prev == NULL){
    lua_pushboolean(L, 0); /* return fail */
    return 1;
  }
	
  manifold->prev = manifold_translate(manifold->prev, manifold->obj, -px, -py, -pz);
  manifold->obj = manifold_rotate(manifold->obj, manifold->prev, ax, ay, az);
  manifold->prev = manifold_translate(manifold->prev, manifold->obj, px, py, pz);
  
  ManifoldManifold *tmp = manifold->obj;
  manifold->obj = manifold->prev;
  manifold->prev = tmp;
  
	lua_pushboolean(L, 1); /* return success */
	return 1;
}

/* modify a manifold object - scale */
/* given parameters:
	- a Manifold object (this function is called as method inside object)
  - x,y,z values factors, as numbers (dflt = 1,1,1)
returns:
	- a boolean indicating success or fail
*/
int dxf_3d_scale (lua_State *L) {
	
	struct manifold_obj * manifold;
  double sx = 1.0, sy = 1.0, sz = 1.0;
	
	/* verify passed arguments */
	int n = lua_gettop(L);    /* number of arguments */
	if (n < 1){
		lua_pushboolean(L, 0); /* return fail */
    return 1;
	}
	if (!( manifold = udata_check(L, 1, "Manifold") )) { /* the Manifold object is a Lua userdata type*/
		lua_pushboolean(L, 0); /* return fail */
    return 1;
	}
  
  if (lua_isnumber(L, 2)) sx = lua_tonumber(L, 2);
  if (lua_isnumber(L, 3)) sy = lua_tonumber(L, 3);
  if (lua_isnumber(L, 4)) sz = lua_tonumber(L, 4);
  
  if (sx <= 0.0) sx = 1.0;
  if (sy <= 0.0) sy = 1.0;
  if (sz <= 0.0) sz = 1.0;
	
	/* check if it is not destroyed */
	if (manifold->obj == NULL){
    lua_pushboolean(L, 0); /* return fail */
    return 1;
  }
  /* check if it is not destroyed */
	if (manifold->prev == NULL){
    lua_pushboolean(L, 0); /* return fail */
    return 1;
  }
	
  ManifoldManifold *tmp = manifold->obj;
  manifold->prev = manifold_scale(manifold->prev, manifold->obj, sx, sy, sz);
  manifold->obj = manifold->prev;
  manifold->prev = tmp;
  
	lua_pushboolean(L, 1); /* return success */
	return 1;
}

/* modify a manifold object - mirror */
/* given parameters:
	- a Manifold object (this function is called as method inside object)
  - normal vector x,y,z values, as numbers (dflt = 0,0,1)
returns:
	- a boolean indicating success or fail
*/
int dxf_3d_mirror (lua_State *L) {
	
	struct manifold_obj * manifold;
  double norm[3] = {0.0, 0.0, 1.0};
	
	/* verify passed arguments */
	int n = lua_gettop(L);    /* number of arguments */
	if (n < 1){
		lua_pushboolean(L, 0); /* return fail */
    return 1;
	}
	if (!( manifold = udata_check(L, 1, "Manifold") )) { /* the Manifold object is a Lua userdata type*/
		lua_pushboolean(L, 0); /* return fail */
    return 1;
	}
  
  if (lua_isnumber(L, 2)) norm[0] = lua_tonumber(L, 2);
  if (lua_isnumber(L, 3)) norm[1] = lua_tonumber(L, 3);
  if (lua_isnumber(L, 4)) norm[2] = lua_tonumber(L, 4);
  
  if (!vec_unit(norm, norm)){
    norm[0] = 0.0; norm[1] = 0.0; norm[2] = 1.0;
  }
	
	/* check if it is not destroyed */
	if (manifold->obj == NULL){
    lua_pushboolean(L, 0); /* return fail */
    return 1;
  }
  /* check if it is not destroyed */
	if (manifold->prev == NULL){
    lua_pushboolean(L, 0); /* return fail */
    return 1;
  }
	
  ManifoldManifold *tmp = manifold->obj;
  manifold->prev = manifold_mirror(manifold->prev, manifold->obj, 
        norm[0], norm[1], norm[2]);
  manifold->obj = manifold->prev;
  manifold->prev = tmp;
  
	lua_pushboolean(L, 1); /* return success */
	return 1;
}

/* modify a manifold object - transform */
/* given parameters:
	- a Manifold object (this function is called as method inside object)
	- transformation matrix 4x3, as table (dflt = unitary matrix)
returns:
	- modified Manifold object or nil if fail
*/
int dxf_3d_transform (lua_State *L) {
	
	struct manifold_obj * manifold;
	double matrix[4][3] = {{1,0,0},{0,1,0},{0,0,1},{0,0,0}};
	
	/* verify passed arguments */
	int n = lua_gettop(L); /* number of arguments */
	if (n < 1){
		lua_pushnil(L); /* return fail */
    return 1;
	}
	if (!( manifold = udata_check(L, 1, "Manifold") )) { /* the Manifold object is a Lua userdata type*/
		lua_pushnil(L); /* return fail */
    return 1;
	}
  
	/* Sweep the tables to get matrix values */
	if (lua_istable(L, 2)) {
		int lines = lua_rawlen(L, 2);
		int i,  j;
		for (i = 0; i < lines; i++) {
			lua_rawgeti(L, 2, i + 1);
			if (lua_istable(L, -1) && i < 4) {
				int cols = lua_rawlen(L, -1);
				for (j = 0; j < cols;  j++) {
					lua_rawgeti(L, -1, j + 1);
					if (lua_isnumber(L, -1) && j < 3) {
						matrix[i][j] = lua_tonumber(L, -1);
					}
					lua_pop (L, 1);
				}
			}
			lua_pop (L, 1);
		}
	}
	
	/* check if it is not destroyed */
	if (manifold->obj == NULL){
    lua_pushnil(L); /* return fail */
    return 1;
  }
  /* check if it is not destroyed */
	if (manifold->prev == NULL){
    lua_pushnil(L); /* return fail */
    return 1;
  }
	
  ManifoldManifold *tmp = manifold->obj;
  manifold->prev = manifold_transform(manifold->prev, manifold->obj,
	matrix[0][0], matrix[0][1], matrix[0][2],
	matrix[1][0], matrix[1][1], matrix[1][2],
	matrix[2][0], matrix[2][1], matrix[2][2],
	matrix[3][0], matrix[3][1], matrix[3][2]);
  manifold->obj = manifold->prev;
  manifold->prev = tmp;
  
	lua_pushvalue(L, 1); /* return success */
	return 1;
}

/* destroy a previouly created manifold object */
/* given parameters:
	- a Manifold object (this function is called as method inside object)
returns:
	- a boolean indicating success or fail
*/
int manifold_destroy (lua_State *L) {
	
	struct manifold_obj * manifold;
	
	/* verify passed arguments */
	int n = lua_gettop(L);    /* number of arguments */
	if (n < 1){
		lua_pushboolean(L, 0); /* return fail */
    return 1;
	}
	if (!( manifold = udata_check(L, 1, "Manifold") )) { /* the Manifold object is a Lua userdata type*/
		lua_pushboolean(L, 0); /* return fail */
    return 1;
	}
	
	/* check if it is not destroyed */
	if (manifold->obj != NULL){
    /* destroy the object, freeing any resources it was using */
    manifold_destruct_manifold(manifold->obj);
    free(manifold->obj);
    manifold->obj = NULL;
  }
  /* check if it is not destroyed */
	if (manifold->prev != NULL){
    /* destroy the prevect, freeing any resources it was using */
    manifold_destruct_manifold(manifold->prev);
    free(manifold->prev);
    manifold->prev = NULL;
  }
	
	lua_pushboolean(L, 1); /* return success */
	return 1;
}

/* create a mesh entity */
/* given parameters:
	- manifold commands, as string
	- drawing parameters (layer, color, etc), as table (optional)
returns:
	- DXF entity, as userdata
Notes:
	- The returned data is for one shot use in Lua script, because
	the alocated memory is valid in single iteration of main loop.
	It is assumed that soon afterwards it will be appended or drawn.
*/
dxf_node * dxf_new_mesh  (char *chunk, int color, char *layer, int pool){
	lua_State *L = dxf_3d_engine.T;
  dxf_3d_engine.time = clock();
  
  lua_newtable(L);
	lua_setglobal(L, "manifold");
  
  lua_pushinteger(L, 32);
	lua_setglobal(L, "circular_segments");
  
  
	dxf_3d_engine.status = luaL_loadbuffer(L, chunk, strlen(chunk), "engine3d");
	if (dxf_3d_engine.status != LUA_OK){
		//dxf_3d_engine.active = 0; /* error */
    printf("3D engine error  - chunk\n" );
		return NULL;
	}
  int n_res = 0; /* for Lua 5.4*/
  dxf_3d_engine.status = lua_resume(L, NULL, 0, &n_res); /* start thread */
	if (dxf_3d_engine.status != LUA_OK){
		//dxf_3d_engine.active = 0; /* error */
    int n = lua_gettop(L);    /* number of arguments */
    printf("3D engine error  - resume, %d , %d\n", n_res, n );
		return NULL;
	}
	
	struct manifold_obj * manifold;
	lua_getglobal(L, "manifold");
	/* verify passed arguments */
	if(!lua_istable(L, -1)){
    printf("3D engine error  - global\n" );
    return NULL;
  }
  
  int last = lua_rawlen(L, -1);
  
  lua_rawgeti (L, -1, last);
  
	if (!( manifold = udata_check(L, -1, "Manifold") )) { /* the Manifold object is a Lua userdata type*/
    printf("3D engine error  - metatable\n" );
		return NULL;
	}
	lua_pop(L, 2);
	
	ManifoldMeshGL64 *mesh = manifold_get_meshgl64(meshgl64_buffer(), manifold->obj);
	
	
	
	/* new mesh entity */
	dxf_node * new_el = (dxf_node *) dxf_new_face_mesh (mesh, chunk, 
		color, layer, pool); 

	
	
	manifold_destruct_meshgl64(mesh);
	manage_buffer(0, BUF_RELEASE, MEMP_POLY);
  
	
	if (!new_el) {
		return NULL;
	}
	/* return success */
	return new_el;
}

int dxf_3d_init (){
	const char * chunk = "manifold = {}";
	/* close previous Lua state */
	if(dxf_3d_engine.L) lua_close(dxf_3d_engine.L);
  
	/* initialize script object */
	if(!(dxf_3d_engine.L = luaL_newstate())) return 0; /* opens Lua */
	dxf_3d_engine.T = NULL;
	dxf_3d_engine.status = LUA_OK;
	dxf_3d_engine.active = 0;
	dxf_3d_engine.dynamic = 0;
	dxf_3d_engine.do_init = 0;
	//dxf_3d_engine.wait_gui_resume = 0;
	dxf_3d_engine.groups = 0;
	dxf_3d_engine.path[0] = 0;
	
	dxf_3d_engine.timeout = 10.0; /* default timeout value */
	
	luaL_openlibs(dxf_3d_engine.L); /* opens the standard libraries */
	
	/* create a new lua thread, allowing yield */
	lua_State *T = lua_newthread(dxf_3d_engine.L);
	if(!T) {
		lua_close(dxf_3d_engine.L);
		return 0;
	}
	dxf_3d_engine.T = T;
	
	/* put the engine script structure in lua global registry */
	lua_pushstring(T, "cz_script");
	lua_pushlightuserdata(T, (void *) &dxf_3d_engine);
	lua_settable(T, LUA_REGISTRYINDEX);
	
	static const struct luaL_Reg manifold_meths[] = {
    {"translate", dxf_3d_translate},
    {"rotate", dxf_3d_rotate},
    {"scale", dxf_3d_scale},
    {"mirror", dxf_3d_mirror},
    {"transform", dxf_3d_transform},
		{"__gc", manifold_destroy},
		{NULL, NULL}
	};
	
	/* create a new type of lua userdata to represent a Manifold object */
	/* create metatable */
	luaL_newmetatable(T, "Manifold");
	/* metatable.__index = metatable */
	lua_pushvalue(T, -1);
	lua_setfield(T, -2, "__index");
	/* register methods */
	luaL_setfuncs(T, manifold_meths, 0);
	lua_pop( T, 1);
  
  lua_pushinteger(T, 32);
	lua_setglobal(T, "circular_segments");
	
	lua_pushcfunction(T, dxf_3d_sphere);
	lua_setglobal(T, "sphere");
  lua_pushcfunction(T, dxf_3d_slab);
	lua_setglobal(T, "slab");
  lua_pushcfunction(T, dxf_3d_cylinder);
	lua_setglobal(T, "cylinder");
  lua_pushcfunction(T, dxf_3d_cone);
	lua_setglobal(T, "cone");
  lua_pushcfunction(T, dxf_3d_pyramid);
	lua_setglobal(T, "pyramid");
  lua_pushcfunction(T, dxf_3d_wedge);
	lua_setglobal(T, "wedge");
  lua_pushcfunction(T, dxf_3d_torus);
	lua_setglobal(T, "torus");
  lua_pushcfunction(T, dxf_3d_extrude);
	lua_setglobal(T, "extrude");
  lua_pushcfunction(T, dxf_3d_extrude_path);
	lua_setglobal(T, "extrude_path");
  
  lua_pushcfunction(T, dxf_3d_union);
	lua_setglobal(T, "union");
  lua_pushcfunction(T, dxf_3d_difference);
	lua_setglobal(T, "difference");
  lua_pushcfunction(T, dxf_3d_intersection);
	lua_setglobal(T, "intersection");
	
	lua_sethook(T, script_check, LUA_MASKCOUNT, 10000);
	
  const char *f = 
    "function check_polygon_wound (contour)\n"
    "  -- verify contour wound\n"
    "  local sum = 0\n"
    "  for i = 1, #contour do\n"
    "    local pt1 = contour[i]\n"
    "    local pt2 = pt1\n"
    "    if (i < #contour) then\n"
    "      pt2 = contour[i+1]\n"
    "    else\n"
    "      pt2 = contour[1]\n"
    "    end\n"
    "    sum = sum + (pt2[1] - pt1[1])*(pt2[2] + pt1[2])\n"
    "  end\n"
    "  if sum * contour.wound < 0 then -- if not correct wound, reserse the contour table\n"
    "    for i = 1, #contour//2, 1 do\n"
    "      contour[i], contour[#contour-i+1] = contour[#contour-i+1], contour[i]\n"
    "    end\n"
    "  end\n"
    "end\n"
    "function inside_poly(pt, poly)\n"
    "  -- Check if a point lies inside polygon\n"
    "  local inside = false\n"
    "  local prev = #poly -- prev point index\n"
    "  -- horizontal scanline method\n"
    "  for i = 1, #poly do\n"
    "    if ((poly[i][2] > pt[2]) ~= (poly[prev][2] > pt[2])) and \n"
    "    (pt[1] < (poly[prev][1] - poly[i][1]) * (pt[2] - poly[i][2]) /\n"
    "    (poly[prev][2] - poly[i][2]) + poly[i][1]) then\n"
    "      inside = not inside\n"
    "    end\n"
    "    prev = i\n"
    "  end\n"
    "  return inside\n"
    "end\n"
    "function find_inside_poly(contours)\n"
    "  -- search for polygons inside other polygon\n"
    "  for i = 1, #contours do\n"
    "    -- init with default wound\n"
    "    contours[i].wound = -1\n"
    "  end\n"
    "  -- sweep polygons, comparing by pairs\n"
    "  for i = 1, #contours do\n"
    "    for j = 1, #contours do\n"
    "      if i ~= j then\n"
    "        local contour = contours[i]\n"
    "        for k = 1, #contour do\n"
    "          -- check if any point lies inside polygon\n"
    "          if inside_poly(contour[k], contours[j]) then\n"
    "            -- flag with reverse wound\n"
    "            contours[i].wound = -1 * contours[i].wound\n"
    "            break\n"
    "          end\n"
    "        end\n"
    "      end\n"
    "    end\n"
    "  end\n"
    "end\n"
    "function check_poly_loops (contours)\n"
    "  find_inside_poly(contours)\n"
    "  for i = 1, #contours do\n"
    "    check_polygon_wound(contours[i])\n"
    "  end\n"
    "end\n";
    luaL_dostring(T, f);
}