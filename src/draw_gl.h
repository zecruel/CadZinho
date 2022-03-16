#ifndef _DRAW_GL_LIB
#define _DRAW_GL_LIB

#include <stddef.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#ifdef PLATFORM_Darwin
  #include <OpenGL/glu.h>
  #include <OpenGL/gl.h>
#else
  #include <GL/glu.h>
  #include <GL/gl.h>
#endif
 
 #include "bmp.h"
 #include "graph.h"
 
#define MAX_TRIANG 100000
#define MAX_TRIANG_2 80000
#define MAX_SCAN_LINES 15000
#define MAX_P_NODES 1000

#define GLSL(src) "#version 150 core\n" #src

struct Vertex {	/*use a struct to represent a vertex in openGL */
    GLfloat pos[3];  /*vertex position - x, y, z*/
    GLfloat uv[2];  /*texture coordinates */
    GLubyte col[4]; /* vertex color - RGBA each 0-255d */
};

struct ogl { /* openGL context to pass main parameters */
	SDL_GLContext ctx;
	struct Vertex *verts; /* vertex buffer */
	GLuint *elems; /*elements buffer */
	int vert_count; /* vertex count */
	int elem_count; /* elements count */
	
	int win_w, win_h; /* canvas size - to perform pixel to openGL conversion */
	int flip_y; /* orientation flag - use cartesian's like oritentation, or canvas like (window up corner) */
	GLubyte fg[4], bg[4]; /*foreground and background colors */
	
	GLuint tex;
	int tex_w, tex_h;
	GLint tex_uni;

};

struct p_node { /* node to perform scanline fill algorithm */
	int up, low;
};

struct edge { /* polygon edge */
	int x0, y0, x1, y1;
};

struct Image {
	int w, h;
	GLubyte *data;
};

int draw_gl_init (void *data, int clear);

int draw_gl_line (struct ogl *gl_ctx, int p0[2], int p1[2], int thick);

int draw_gl_quad (struct ogl *gl_ctx, int tl[2], int bl[2], int tr[2], int br[2]);

int draw_gl_rect (struct ogl *gl_ctx, int x, int y, int w, int h);

int draw_gl_triang (struct ogl *gl_ctx, int p0[2], int p1[2], int p2[2]);

int draw_gl_polygon (struct ogl *gl_ctx, int n, struct edge edges[]);

int draw_gl_image (struct ogl *gl_ctx, int x, int y, int w, int h, bmp_img *img);

int draw_gl (struct ogl *gl_ctx, int force);

int graph_list_draw_gl(list_node *list, struct ogl *gl_ctx, struct draw_param param);

int graph_list_draw_gl2(list_node *list, struct ogl *gl_ctx, struct draw_param param);

int dxf_list_draw_gl(list_node *list, struct ogl *gl_ctx,  struct draw_param param);

int dxf_ents_draw_gl(dxf_drawing *drawing, struct ogl *gl_ctx, struct draw_param param);

#endif