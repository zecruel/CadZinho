#include "draw_gl.h"
 #include "gui.h"

#define TOLERANCE 1e-6

/* mantain one unique element in a sorted array - array of integer values */
static int unique (int n, int * a) {
	int dst = 0, i;
	for (i = 1; i < n; ++i) {
	if (a[dst] != a[i])
		a[++dst] = a[i];
	}
	return dst + 1;
}

/* comparation function for qsort - array of integer values */
static int cmp_int (const void * a, const void * b) {
	return (*(int*)a - *(int*)b);
}

/* comparation function for qsort  - array of nodes */
static int cmp_node (const void * a, const void * b) {
	struct p_node *aa, *bb;
	aa = (struct p_node *) a; bb = (struct p_node *) b;
	
	int r = aa->up - bb->up;
	if (r > 0) return 1;
	else if (r < 0) return -1;
	/* if node.up values are same, compare node.low values */
	else return (aa->low - bb->low);
}

int draw_gl_init (void *data, int clear){ /* init (or de-init) OpenGL */
	gui_obj *gui = data;
	
	static GLuint vao, vbo, ebo, vertexShader, fragmentShader, shaderProgram;
	static int init = 0;
	
	if (!init && !clear){ /* init OpenGL */
		//printf("%s\n", glGetString(GL_VERSION) );
		
		/* buffer setup */
		GLsizei vs = sizeof(struct Vertex);
		size_t vp = offsetof(struct Vertex, pos);
		size_t vt = offsetof(struct Vertex, uv);
		size_t vc = offsetof(struct Vertex, col);
		
    #ifndef GLES2
		/* Init GLEW */
		glewExperimental = GL_TRUE;
		if (glewInit() != GLEW_OK) {
			fprintf(stderr, "Failed to initialize GLEW\n");
			return -1;
		}

		/* Create Vertex Array Object */
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
    #endif

		/* Create a Vertex Buffer Object and copy the vertex data to it */
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, MAX_TRIANG * 3 * vs, NULL, GL_STREAM_DRAW); //GL_STATIC_DRAW);
		
		/* Create a Element Buffer Object */
		glGenBuffers(1, &ebo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_TRIANG * 3 * sizeof(GLuint), NULL,  GL_STREAM_DRAW);//GL_STATIC_DRAW); //GL_STREAM_DRAW
		
		/* Create and compile the vertex shader */
    #ifdef GLES2
    const char* vertexSource = "#version 100\n"
      "attribute vec3 position;\n"
      "attribute vec2 uv;\n"
      "attribute vec4 color;\n"
      "varying vec4 vertexColor;\n"
      "varying vec2 texcoord;\n"
      "uniform mat4 transform;\n"
      "void main() {\n"
      "	vertexColor = color;\n"
      "	texcoord = uv;\n"
      "	gl_Position = transform * vec4(position, 1.0);\n"
      "}\n"; /* =========== vertex shader */
      
    const char* fragmentSource = "#version 100\n"
      "precision mediump float;\n"
      "varying vec4 vertexColor;\n"
      "varying vec2 texcoord;\n"
      "uniform sampler2D tex;\n\n"
      "void main() {\n"
      "	gl_FragColor = texture2D(tex, texcoord) * vertexColor;\n"
      "}\n"; /* ========== fragment shader */
  #else
    const char* vertexSource = "#version 150 core\n"
      "in vec3 position;\n"
      "in vec2 uv;\n"
      "in vec4 color;\n"
      "out vec4 vertexColor;\n"
      "out vec2 texcoord;\n"
      "uniform mat4 transform;\n"
      "void main() {\n"
      "	gl_Position = transform * vec4(position, 1.0);\n"
      "	vertexColor = color;\n"
      "	texcoord = uv;\n"
      "}\n"; /* =========== vertex shader */
      
    const char* fragmentSource = "#version 150 core\n"
      "in vec4 vertexColor;\n"
      "in vec2 texcoord;\n"
      "out vec4 outColor;\n"
      "uniform sampler2D tex;\n\n"
      "void main() {\n"
      "	outColor = texture(tex, texcoord) * vertexColor;\n"
      "}\n"; /* ========== fragment shader */
   #endif

		vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, &vertexSource, NULL);
		glCompileShader(vertexShader);

		fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
		glCompileShader(fragmentShader);

		/* Link the vertex and fragment shader into a shader program */
		shaderProgram = glCreateProgram();
		glAttachShader(shaderProgram, vertexShader);
		glAttachShader(shaderProgram, fragmentShader);
  #ifndef GLES2
		glBindFragDataLocation(shaderProgram, 0, "outColor");
  #endif
		glLinkProgram(shaderProgram);
		glUseProgram(shaderProgram);
			
		/*texture */
		GLuint textures[2];
		glGenTextures(2, textures);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textures[0]);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		
		/* blank texture (default) */
		GLubyte blank[] = {255, 255, 255, 255};
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, blank);
		
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, textures[1]);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, gui->gl_ctx.tex_w, gui->gl_ctx.tex_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		
		gui->gl_ctx.tex = textures[1];
		gui->gl_ctx.tex_uni = glGetUniformLocation(shaderProgram, "tex");
		
		glUniform1i(gui->gl_ctx.tex_uni, 0);
		
		gui->gl_ctx.transf[0][0] = 1.0;
		gui->gl_ctx.transf[0][1] = 0.0;
		gui->gl_ctx.transf[0][2] = 0.0;
		gui->gl_ctx.transf[0][3] = 0.0;
		gui->gl_ctx.transf[1][0] = 0.0;
		gui->gl_ctx.transf[1][1] = 1.0;
		gui->gl_ctx.transf[1][2] = 0.0;
		gui->gl_ctx.transf[1][3] = 0.0;
		gui->gl_ctx.transf[2][0] = 0.0;
		gui->gl_ctx.transf[2][1] = 0.0;
		gui->gl_ctx.transf[2][2] = 1.0;
		gui->gl_ctx.transf[2][3] = 0.0;
		gui->gl_ctx.transf[3][0] = 0.0;
		gui->gl_ctx.transf[3][1] = 0.0;
		gui->gl_ctx.transf[3][2] = 0.0;
		gui->gl_ctx.transf[3][3] = 1.0;
		
		gui->gl_ctx.transf_uni = glGetUniformLocation(shaderProgram, "transform");
		glUniformMatrix4fv(gui->gl_ctx.transf_uni, 1,  GL_FALSE, &gui->gl_ctx.transf[0][0]);

		/* Specify the layout of the vertex data */
		GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
		GLint uvAttrib = glGetAttribLocation(shaderProgram, "uv");
		GLint colorAttrib = glGetAttribLocation(shaderProgram, "color");
		
		glEnableVertexAttribArray(posAttrib);
		glEnableVertexAttribArray(uvAttrib);
		glEnableVertexAttribArray(colorAttrib);
		
		glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, vs, (void*)vp);
		glVertexAttribPointer(uvAttrib, 2, GL_FLOAT, GL_FALSE, vs, (void*)vt);
		glVertexAttribPointer(colorAttrib, 4, GL_UNSIGNED_BYTE, GL_TRUE, vs, (void*)vc);
		
		
		glEnable(GL_BLEND); 
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		
		// Enable depth test
		glEnable(GL_DEPTH_TEST);
		// Accept fragment if it closer to the camera than the former one
		glDepthFunc(GL_LESS);
    
  #ifdef GLES2
    /* load vertices/elements directly into vertex/element buffer */
    gui->gl_ctx.verts = malloc(MAX_TRIANG*3*vs);
    gui->gl_ctx.elems = malloc(MAX_TRIANG*3*sizeof(GLuint));
  #else
		/* load vertices/elements directly into vertex/element buffer */
    if (gui->gl_ctx.elems == NULL){
      gui->gl_ctx.verts = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
      gui->gl_ctx.elems = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
    }
  #endif
		//glScissor(200,200,100,100);
		//glEnable(GL_SCISSOR_TEST);
		//glDisable(GL_SCISSOR_TEST);
		init = 1;
		return 1;
	}
	else if (init && clear){ /* clear OpenGL - Delete allocated resources */
		glDeleteProgram(shaderProgram);
		glDeleteShader(fragmentShader);
		glDeleteShader(vertexShader);
		glDeleteBuffers(1, &ebo);
		glDeleteBuffers(1, &vbo);
  #ifndef GLES2
		glDeleteVertexArrays(1, &vao);
  #endif
		return 1;
	}
}

int draw_gl_line (struct ogl *gl_ctx, int p0[3], int p1[3], int thick){
	/* emulate drawing a single line with thickness, using triangles in openGL */
	
	/* verify struct and buffers */
	if (!gl_ctx) return 0;
	if (!gl_ctx->verts) return 0;
	if (!gl_ctx->elems) return 0;
	
	/* check if elements buffer is full,
	and draw waiting elements to clear buffer if necessary */
	draw_gl (gl_ctx, 0);
  #ifndef GLES2
	if (gl_ctx->elems == NULL){
		gl_ctx->verts = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		gl_ctx->elems = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
	}
  #endif
	/* */
	
	if (thick <= 0) thick = 1; /* one pixel as minimal thickness */
	
	/* get polar parameters of line */
	float dx = p1[0] - p0[0];
	float dy = p1[1] - p0[1];
	float modulus = sqrt(pow(dx, 2) + pow(dy, 2));
	float cosine = 1.0;
	float sine = 0.0;
	if (modulus > TOLERANCE){
		cosine = dx/modulus;
		sine = dy/modulus;
	}
	else {
		/* a dot*/
		//p1[0] += thick;
		return 0;
	}
	
	/* convert input coordinates, in pixles (int), to openGL units */
	float tx = (float) thick / 2.0;
	float ty = tx;
	float x0 = (float) p0[0];
	float y0 = (float) p0[1];
	float z0 = (float) p0[2];
	float x1 = (float) p1[0];
	float y1 = (float) p1[1];
	float z1 = (float) p1[2];
	
	/* orientation in drawing area */
	float flip_y = (gl_ctx->flip_y) ? -1.0 : 1.0;
	
	/* store vertices - 4 vertices */
	/* 0 */
	int j = gl_ctx->vert_count;
	gl_ctx->verts[j].pos[0] = x0 - sine * tx;
	gl_ctx->verts[j].pos[1] = y0 + cosine * ty;
	gl_ctx->verts[j].pos[2] = z0;
	gl_ctx->verts[j].col[0] = gl_ctx->fg[0];
	gl_ctx->verts[j].col[1] = gl_ctx->fg[1];
	gl_ctx->verts[j].col[2] = gl_ctx->fg[2];
	gl_ctx->verts[j].col[3] = gl_ctx->fg[3];
	gl_ctx->verts[j].uv[0] = 0.0;
	gl_ctx->verts[j].uv[1] = (float)(gl_ctx->flip_y);
	gl_ctx->vert_count ++;
	/* 1 */
	j = gl_ctx->vert_count;
	gl_ctx->verts[j].pos[0] = x0 + sine * tx;
	gl_ctx->verts[j].pos[1] = y0 - cosine * ty;
	gl_ctx->verts[j].pos[2] = z0;
	gl_ctx->verts[j].col[0] = gl_ctx->fg[0];
	gl_ctx->verts[j].col[1] = gl_ctx->fg[1];
	gl_ctx->verts[j].col[2] = gl_ctx->fg[2];
	gl_ctx->verts[j].col[3] = gl_ctx->fg[3];
	gl_ctx->verts[j].uv[0] = 0.0;
	gl_ctx->verts[j].uv[1] = (float)(1 - gl_ctx->flip_y);
	gl_ctx->vert_count ++;
	/* 2 */
	j = gl_ctx->vert_count;
	gl_ctx->verts[j].pos[0] = x1 - sine * tx;
	gl_ctx->verts[j].pos[1] = y1 + cosine * ty;
	gl_ctx->verts[j].pos[2] = z1;
	gl_ctx->verts[j].col[0] = gl_ctx->fg[0];
	gl_ctx->verts[j].col[1] = gl_ctx->fg[1];
	gl_ctx->verts[j].col[2] = gl_ctx->fg[2];
	gl_ctx->verts[j].col[3] = gl_ctx->fg[3];
	gl_ctx->verts[j].uv[0] = 1.0;
	gl_ctx->verts[j].uv[1] = (float)(gl_ctx->flip_y);
	gl_ctx->vert_count ++;
	/* 3 */
	j = gl_ctx->vert_count;
	gl_ctx->verts[j].pos[0] = x1 + sine * tx;
	gl_ctx->verts[j].pos[1] = y1 - cosine * ty;
	gl_ctx->verts[j].pos[2] = z1;
	gl_ctx->verts[j].col[0] = gl_ctx->fg[0];
	gl_ctx->verts[j].col[1] = gl_ctx->fg[1];
	gl_ctx->verts[j].col[2] = gl_ctx->fg[2];
	gl_ctx->verts[j].col[3] = gl_ctx->fg[3];
	gl_ctx->verts[j].uv[0] = 1.0;
	gl_ctx->verts[j].uv[1] = (float)(1 - gl_ctx->flip_y);
	gl_ctx->vert_count ++;
	
	/* store vertex indexes in elements buffer - 2 triangles that share vertices  */
	/* 0 */
	j = gl_ctx->elem_count * 3;
	gl_ctx->elems[j] = gl_ctx->vert_count - 4;
	gl_ctx->elems[j+1] = gl_ctx->vert_count - 3;
	gl_ctx->elems[j+2] = gl_ctx->vert_count - 2;
	/* 1 */
	gl_ctx->elems[j+3] = gl_ctx->vert_count - 3;
	gl_ctx->elems[j+4] = gl_ctx->vert_count - 2;
	gl_ctx->elems[j+5] = gl_ctx->vert_count - 1;
	gl_ctx->elem_count+= 2;
	
	return 1;
}

int draw_gl_quad (struct ogl *gl_ctx, int tl[3], int bl[3], int tr[3], int br[3]){
	/* emulate drawing a filled and convex quadrilateral, using triangles in openGL */
	
	/* verify struct and buffers */
	if (!gl_ctx) return 0;
	if (!gl_ctx->verts) return 0;
	if (!gl_ctx->elems) return 0;
	
	/* check if elements buffer is full,
	and draw waiting elements to clear buffer if necessary */
	draw_gl (gl_ctx, 0);
  #ifndef GLES2
	if (gl_ctx->elems == NULL){
		gl_ctx->verts = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		gl_ctx->elems = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
	}
  #endif
	/* */
	
	/* orientation in drawing area */
	float flip_y = (gl_ctx->flip_y) ? -1.0 : 1.0;
	
	/* convert input coordinates, in pixles (int), to openGL units and store vertices - 4 vertices */
	/* 0 */
	int j = gl_ctx->vert_count;
	gl_ctx->verts[j].pos[0] = (float) bl[0];
	gl_ctx->verts[j].pos[1] = (float) bl[1];
	gl_ctx->verts[j].pos[2] = (float) bl[2];
	gl_ctx->verts[j].col[0] = gl_ctx->fg[0];
	gl_ctx->verts[j].col[1] = gl_ctx->fg[1];
	gl_ctx->verts[j].col[2] = gl_ctx->fg[2];
	gl_ctx->verts[j].col[3] = gl_ctx->fg[3];
	gl_ctx->verts[j].uv[0] = 0.0;
	gl_ctx->verts[j].uv[1] = (float)(gl_ctx->flip_y);
	gl_ctx->vert_count ++;
	/* 1 */
	j = gl_ctx->vert_count;
	gl_ctx->verts[j].pos[0] = (float) tl[0];
	gl_ctx->verts[j].pos[1] = (float) tl[1];
	gl_ctx->verts[j].pos[2] = (float) tl[2];
	gl_ctx->verts[j].col[0] = gl_ctx->fg[0];
	gl_ctx->verts[j].col[1] = gl_ctx->fg[1];
	gl_ctx->verts[j].col[2] = gl_ctx->fg[2];
	gl_ctx->verts[j].col[3] = gl_ctx->fg[3];
	gl_ctx->verts[j].uv[0] = 0.0;
	gl_ctx->verts[j].uv[1] = (float)(1 - gl_ctx->flip_y);
	gl_ctx->vert_count ++;
	/* 2 */
	j = gl_ctx->vert_count;
	gl_ctx->verts[j].pos[0] = (float) br[0];
	gl_ctx->verts[j].pos[1] = (float) br[1];
	gl_ctx->verts[j].pos[2] = (float) br[2];
	gl_ctx->verts[j].col[0] = gl_ctx->fg[0];
	gl_ctx->verts[j].col[1] = gl_ctx->fg[1];
	gl_ctx->verts[j].col[2] = gl_ctx->fg[2];
	gl_ctx->verts[j].col[3] = gl_ctx->fg[3];
	gl_ctx->verts[j].uv[0] = 1.0;
	gl_ctx->verts[j].uv[1] = (float)(gl_ctx->flip_y);
	gl_ctx->vert_count ++;
	/* 3 */
	j = gl_ctx->vert_count;
	gl_ctx->verts[j].pos[0] = (float) tr[0];
	gl_ctx->verts[j].pos[1] = (float) tr[1];
	gl_ctx->verts[j].pos[2] = (float) tr[2];
	gl_ctx->verts[j].col[0] = gl_ctx->fg[0];
	gl_ctx->verts[j].col[1] = gl_ctx->fg[1];
	gl_ctx->verts[j].col[2] = gl_ctx->fg[2];
	gl_ctx->verts[j].col[3] = gl_ctx->fg[3];
	gl_ctx->verts[j].uv[0] = 1.0;
	gl_ctx->verts[j].uv[1] = (float)(1 - gl_ctx->flip_y);
	gl_ctx->vert_count ++;
	/* store vertex indexes in elements buffer - 2 triangles that share vertices  */
	/* 0 */
	j = gl_ctx->elem_count * 3;
	gl_ctx->elems[j] = gl_ctx->vert_count - 4;
	gl_ctx->elems[j+1] = gl_ctx->vert_count - 3;
	gl_ctx->elems[j+2] = gl_ctx->vert_count - 2;
	/* 1 */
	gl_ctx->elems[j+3] = gl_ctx->vert_count - 3;
	gl_ctx->elems[j+4] = gl_ctx->vert_count - 2;
	gl_ctx->elems[j+5] = gl_ctx->vert_count - 1;
	gl_ctx->elem_count+= 2;
	
	return 1;
}

int draw_gl_rect (struct ogl *gl_ctx, int x, int y, int z, int w, int h){
	/* emulate drawing a filled and convex quadrilateral, using triangles in openGL */
	
	/* verify struct and buffers */
	if (!gl_ctx) return 0;
	if (!gl_ctx->verts) return 0;
	if (!gl_ctx->elems) return 0;
	
	/* check if elements buffer is full,
	and draw waiting elements to clear buffer if necessary */
	draw_gl (gl_ctx, 0);
  #ifndef GLES2
	if (gl_ctx->elems == NULL){
		gl_ctx->verts = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		gl_ctx->elems = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
	}
  #endif
	/* */
	
	/* orientation in drawing area */
	float flip_y = (gl_ctx->flip_y) ? -1.0 : 1.0;
	float scale_u = 1.0;
	float scale_v = 1.0;
	
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gl_ctx->tex);
	
	if (w > gl_ctx->tex_w || h > gl_ctx->tex_h){
		
		scale_u = 1.0;
		scale_v = 1.0;
		gl_ctx->tex_w = w;
		gl_ctx->tex_h = h;
	}
	else {
		
		scale_u = (float) w / (float) gl_ctx->tex_w;
		scale_v = (float) h / (float) gl_ctx->tex_h;
	}
	
	/* convert input coordinates, in pixles (int), to openGL units and store vertices - 4 vertices */
	/* 0 */
	int j = gl_ctx->vert_count;
	gl_ctx->verts[j].pos[0] = (float) x;
	gl_ctx->verts[j].pos[1] = (float) y;
	gl_ctx->verts[j].pos[2] = (float) z;
	gl_ctx->verts[j].col[0] = gl_ctx->fg[0];
	gl_ctx->verts[j].col[1] = gl_ctx->fg[1];
	gl_ctx->verts[j].col[2] = gl_ctx->fg[2];
	gl_ctx->verts[j].col[3] = gl_ctx->fg[3];
	gl_ctx->verts[j].uv[0] = 0.0;
	gl_ctx->verts[j].uv[1] = (float)(1 - gl_ctx->flip_y) * scale_v;
	gl_ctx->vert_count ++;
	/* 1 */
	j = gl_ctx->vert_count;
	gl_ctx->verts[j].pos[0] = (float) x;
	gl_ctx->verts[j].pos[1] = (float) (y + h);
	gl_ctx->verts[j].pos[2] = (float) z;
	gl_ctx->verts[j].col[0] = gl_ctx->fg[0];
	gl_ctx->verts[j].col[1] = gl_ctx->fg[1];
	gl_ctx->verts[j].col[2] = gl_ctx->fg[2];
	gl_ctx->verts[j].col[3] = gl_ctx->fg[3];
	gl_ctx->verts[j].uv[0] = 0.0;
	gl_ctx->verts[j].uv[1] = (float)(gl_ctx->flip_y) * scale_v;
	gl_ctx->vert_count ++;
	/* 2 */
	j = gl_ctx->vert_count;
	gl_ctx->verts[j].pos[0] = (float) (x + w);
	gl_ctx->verts[j].pos[1] = (float) y;
	gl_ctx->verts[j].pos[2] = (float) z;
	gl_ctx->verts[j].col[0] = gl_ctx->fg[0];
	gl_ctx->verts[j].col[1] = gl_ctx->fg[1];
	gl_ctx->verts[j].col[2] = gl_ctx->fg[2];
	gl_ctx->verts[j].col[3] = gl_ctx->fg[3];
	gl_ctx->verts[j].uv[0] = scale_u;
	gl_ctx->verts[j].uv[1] = (float)(1 - gl_ctx->flip_y) * scale_v;
	gl_ctx->vert_count ++;
	/* 3 */
	j = gl_ctx->vert_count;
	gl_ctx->verts[j].pos[0] = (float) (x + w);
	gl_ctx->verts[j].pos[1] = (float) (y + h);
	gl_ctx->verts[j].pos[2] = (float) z;
	gl_ctx->verts[j].col[0] = gl_ctx->fg[0];
	gl_ctx->verts[j].col[1] = gl_ctx->fg[1];
	gl_ctx->verts[j].col[2] = gl_ctx->fg[2];
	gl_ctx->verts[j].col[3] = gl_ctx->fg[3];
	gl_ctx->verts[j].uv[0] =  scale_u;
	gl_ctx->verts[j].uv[1] = (float)(gl_ctx->flip_y) * scale_v;
	gl_ctx->vert_count ++;
	/* store vertex indexes in elements buffer - 2 triangles that share vertices  */
	/* 0 */
	j = gl_ctx->elem_count * 3;
	gl_ctx->elems[j] = gl_ctx->vert_count - 4;
	gl_ctx->elems[j+1] = gl_ctx->vert_count - 3;
	gl_ctx->elems[j+2] = gl_ctx->vert_count - 2;
	/* 1 */
	gl_ctx->elems[j+3] = gl_ctx->vert_count - 3;
	gl_ctx->elems[j+4] = gl_ctx->vert_count - 2;
	gl_ctx->elems[j+5] = gl_ctx->vert_count - 1;
	gl_ctx->elem_count+= 2;
	
	return 1;
}

int draw_gl_triang (struct ogl *gl_ctx, int p0[3], int p1[3], int p2[3]){
	/* add a triangle to openGL */
	
	/* verify struct and buffers */
	if (!gl_ctx) return 0;
	if (!gl_ctx->verts) return 0;
	if (!gl_ctx->elems) return 0;
	
	/* check if elements buffer is full,
	and draw waiting elements to clear buffer if necessary */
	draw_gl (gl_ctx, 0);
  #ifndef GLES2
	if (gl_ctx->elems == NULL){
		gl_ctx->verts = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		gl_ctx->elems = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
	}
  #endif
	/* */
		
	/* orientation in drawing area */
	float flip_y = (gl_ctx->flip_y) ? -1.0 : 1.0;
	
	/* convert input coordinates, in pixles (int), to openGL units and store vertices - 3 vertices */
	/* 0 */
	int j = gl_ctx->vert_count;
	gl_ctx->verts[j].pos[0] = (float) p0[0];
	gl_ctx->verts[j].pos[1] = (float) p0[1];
	gl_ctx->verts[j].pos[2] = (float) p0[2];
	gl_ctx->verts[j].col[0] = gl_ctx->fg[0];
	gl_ctx->verts[j].col[1] = gl_ctx->fg[1];
	gl_ctx->verts[j].col[2] = gl_ctx->fg[2];
	gl_ctx->verts[j].col[3] = gl_ctx->fg[3];
	gl_ctx->verts[j].uv[0] = 0.0;
	gl_ctx->verts[j].uv[1] = (float)(gl_ctx->flip_y);
	gl_ctx->vert_count ++;
	/* 1 */
	j = gl_ctx->vert_count;
	gl_ctx->verts[j].pos[0] = (float) p1[0];
	gl_ctx->verts[j].pos[1] = (float) p1[1];
	gl_ctx->verts[j].pos[2] = (float) p1[2];
	gl_ctx->verts[j].col[0] = gl_ctx->fg[0];
	gl_ctx->verts[j].col[1] = gl_ctx->fg[1];
	gl_ctx->verts[j].col[2] = gl_ctx->fg[2];
	gl_ctx->verts[j].col[3] = gl_ctx->fg[3];
	gl_ctx->verts[j].uv[0] = 0.0;
	gl_ctx->verts[j].uv[1] = (float)(1 - gl_ctx->flip_y);
	gl_ctx->vert_count ++;
	/* 2 */
	j = gl_ctx->vert_count;
	gl_ctx->verts[j].pos[0] = (float) p2[0];
	gl_ctx->verts[j].pos[1] = (float) p2[1];
	gl_ctx->verts[j].pos[2] = (float) p2[2];
	gl_ctx->verts[j].col[0] = gl_ctx->fg[0];
	gl_ctx->verts[j].col[1] = gl_ctx->fg[1];
	gl_ctx->verts[j].col[2] = gl_ctx->fg[2];
	gl_ctx->verts[j].col[3] = gl_ctx->fg[3];
	gl_ctx->verts[j].uv[0] = 1.0;
	gl_ctx->verts[j].uv[1] = (float)(1 - gl_ctx->flip_y);
	gl_ctx->vert_count ++;
	
	/* store vertex indexes in elements buffer - single element  */
	j = gl_ctx->elem_count * 3;
	gl_ctx->elems[j] = gl_ctx->vert_count - 3;
	gl_ctx->elems[j+1] = gl_ctx->vert_count - 2;
	gl_ctx->elems[j+2] = gl_ctx->vert_count - 1;
	gl_ctx->elem_count++;
	
	return 1;
}

int draw_gl_image_rec (struct ogl *gl_ctx, int x, int y, int z, int w, int h, bmp_img *img){
	/* emulate drawing a filled and convex quadrilateral, using triangles in openGL */
	
	/* verify struct and buffers */
	if (!gl_ctx) return 0;
	if (!gl_ctx->verts) return 0;
	if (!gl_ctx->elems) return 0;
	if (!img) return 0;
	
	/* check if elements buffer is full,
	and draw waiting elements to clear buffer if necessary */
	draw_gl (gl_ctx, 0);
  #ifndef GLES2
	if (gl_ctx->elems == NULL){
		gl_ctx->verts = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		gl_ctx->elems = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
	}
  #endif
	/* */
	
	/* orientation in drawing area */
	float flip_y = (gl_ctx->flip_y) ? -1.0 : 1.0;
	float scale_u = 1.0;
	float scale_v = 1.0;
	
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gl_ctx->tex);
	
	if (img->width > gl_ctx->tex_w || img->height > gl_ctx->tex_h){ /* verify if image is greater then texture */
		gl_ctx->tex_w = (gl_ctx->tex_w > img->width) ? gl_ctx->tex_w : img->width;
		gl_ctx->tex_h = (gl_ctx->tex_h > img->height) ? gl_ctx->tex_h : img->height;
		/* realoc texture */
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, gl_ctx->tex_w, gl_ctx->tex_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	}
	
	/* transfer image pixels to texture */
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, img->width, img->height, GL_RGBA, GL_UNSIGNED_BYTE, img->buf);
	/* image dimmensions in texture */
	scale_u = (float) img->width / (float) gl_ctx->tex_w;
	scale_v = (float) img->height / (float) gl_ctx->tex_h;
	
	
	/* convert input coordinates, in pixles (int), to openGL units and store vertices - 4 vertices */
	/* 0 */
	int j = gl_ctx->vert_count;
	gl_ctx->verts[j].pos[0] = (float) x;
	gl_ctx->verts[j].pos[1] = (float) y;
	gl_ctx->verts[j].pos[2] = (float) z;
	gl_ctx->verts[j].col[0] = gl_ctx->fg[0];
	gl_ctx->verts[j].col[1] = gl_ctx->fg[1];
	gl_ctx->verts[j].col[2] = gl_ctx->fg[2];
	gl_ctx->verts[j].col[3] = gl_ctx->fg[3];
	gl_ctx->verts[j].uv[0] = 0.0;
	gl_ctx->verts[j].uv[1] = (float)(1 - gl_ctx->flip_y) * scale_v;
	gl_ctx->vert_count ++;
	/* 1 */
	j = gl_ctx->vert_count;
	gl_ctx->verts[j].pos[0] = (float) x;
	gl_ctx->verts[j].pos[1] = (float) (y + h);
	gl_ctx->verts[j].pos[2] = (float) z;
	gl_ctx->verts[j].col[0] = gl_ctx->fg[0];
	gl_ctx->verts[j].col[1] = gl_ctx->fg[1];
	gl_ctx->verts[j].col[2] = gl_ctx->fg[2];
	gl_ctx->verts[j].col[3] = gl_ctx->fg[3];
	gl_ctx->verts[j].uv[0] = 0.0;
	gl_ctx->verts[j].uv[1] = (float)(gl_ctx->flip_y) * scale_v;
	gl_ctx->vert_count ++;
	/* 2 */
	j = gl_ctx->vert_count;
	gl_ctx->verts[j].pos[0] = (float) (x + w);
	gl_ctx->verts[j].pos[1] = (float) y;
	gl_ctx->verts[j].pos[2] = (float) z;
	gl_ctx->verts[j].col[0] = gl_ctx->fg[0];
	gl_ctx->verts[j].col[1] = gl_ctx->fg[1];
	gl_ctx->verts[j].col[2] = gl_ctx->fg[2];
	gl_ctx->verts[j].col[3] = gl_ctx->fg[3];
	gl_ctx->verts[j].uv[0] = scale_u;
	gl_ctx->verts[j].uv[1] = (float)(1 - gl_ctx->flip_y) * scale_v;
	gl_ctx->vert_count ++;
	/* 3 */
	j = gl_ctx->vert_count;
	gl_ctx->verts[j].pos[0] = (float) (x + w);
	gl_ctx->verts[j].pos[1] = (float) (y + h);
	gl_ctx->verts[j].pos[2] = (float) z;
	gl_ctx->verts[j].col[0] = gl_ctx->fg[0];
	gl_ctx->verts[j].col[1] = gl_ctx->fg[1];
	gl_ctx->verts[j].col[2] = gl_ctx->fg[2];
	gl_ctx->verts[j].col[3] = gl_ctx->fg[3];
	gl_ctx->verts[j].uv[0] =  scale_u;
	gl_ctx->verts[j].uv[1] = (float)(gl_ctx->flip_y) * scale_v;
	gl_ctx->vert_count ++;
	/* store vertex indexes in elements buffer - 2 triangles that share vertices  */
	/* 0 */
	j = gl_ctx->elem_count * 3;
	gl_ctx->elems[j] = gl_ctx->vert_count - 4;
	gl_ctx->elems[j+1] = gl_ctx->vert_count - 3;
	gl_ctx->elems[j+2] = gl_ctx->vert_count - 2;
	/* 1 */
	gl_ctx->elems[j+3] = gl_ctx->vert_count - 3;
	gl_ctx->elems[j+4] = gl_ctx->vert_count - 2;
	gl_ctx->elems[j+5] = gl_ctx->vert_count - 1;
	gl_ctx->elem_count+= 2;
	
	return 1;
}

int draw_gl_image_quad(struct ogl *gl_ctx, int tl[3], int bl[3], int tr[3], int br[3], bmp_img *img){
	/* emulate drawing a filled and convex quadrilateral, using triangles in openGL */
	
	/* verify struct and buffers */
	if (!gl_ctx) return 0;
	if (!gl_ctx->verts) return 0;
	if (!gl_ctx->elems) return 0;
	
	/* check if elements buffer is full,
	and draw waiting elements to clear buffer if necessary */
	draw_gl (gl_ctx, 0);
  #ifndef GLES2
	if (gl_ctx->elems == NULL){
		gl_ctx->verts = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		gl_ctx->elems = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
	}
  #endif
	/* */
	
	/* orientation in drawing area */
	float flip_y = (gl_ctx->flip_y) ? -1.0 : 1.0;
	float scale_u = 1.0;
	float scale_v = 1.0;
	
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gl_ctx->tex);
	
	if (img->width > gl_ctx->tex_w || img->height > gl_ctx->tex_h){ /* verify if image is greater then texture */
		gl_ctx->tex_w = (gl_ctx->tex_w > img->width) ? gl_ctx->tex_w : img->width;
		gl_ctx->tex_h = (gl_ctx->tex_h > img->height) ? gl_ctx->tex_h : img->height;
		/* realoc texture */
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, gl_ctx->tex_w, gl_ctx->tex_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	}
	
	/* transfer image pixels to texture */
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, img->width, img->height, GL_RGBA, GL_UNSIGNED_BYTE, img->buf);
	/* image dimmensions in texture */
	scale_u = (float) img->width / (float) gl_ctx->tex_w;
	scale_v = (float) img->height / (float) gl_ctx->tex_h;
	
	/* convert input coordinates, in pixles (int), to openGL units and store vertices - 4 vertices */
	/* 0 */
	int j = gl_ctx->vert_count;
	gl_ctx->verts[j].pos[0] = (float) bl[0];
	gl_ctx->verts[j].pos[1] = (float) bl[1];
	gl_ctx->verts[j].pos[2] = (float) bl[2];
	gl_ctx->verts[j].col[0] = gl_ctx->fg[0];
	gl_ctx->verts[j].col[1] = gl_ctx->fg[1];
	gl_ctx->verts[j].col[2] = gl_ctx->fg[2];
	gl_ctx->verts[j].col[3] = gl_ctx->fg[3];
	gl_ctx->verts[j].uv[0] =  0.0;
	gl_ctx->verts[j].uv[1] = (float)(1 - gl_ctx->flip_y) * scale_v;
	gl_ctx->vert_count ++;
	/* 1 */
	j = gl_ctx->vert_count;
	gl_ctx->verts[j].pos[0] = (float) tl[0];
	gl_ctx->verts[j].pos[1] = (float) tl[1];
	gl_ctx->verts[j].pos[2] = (float) tl[2];
	gl_ctx->verts[j].col[0] = gl_ctx->fg[0];
	gl_ctx->verts[j].col[1] = gl_ctx->fg[1];
	gl_ctx->verts[j].col[2] = gl_ctx->fg[2];
	gl_ctx->verts[j].col[3] = gl_ctx->fg[3];
	gl_ctx->verts[j].uv[0] =  0.0;
	gl_ctx->verts[j].uv[1] = (float)(gl_ctx->flip_y) * scale_v;
	gl_ctx->vert_count ++;
	/* 2 */
	j = gl_ctx->vert_count;
	gl_ctx->verts[j].pos[0] = (float) br[0];
	gl_ctx->verts[j].pos[1] = (float) br[1];
	gl_ctx->verts[j].pos[2] = (float) br[2];
	gl_ctx->verts[j].col[0] = gl_ctx->fg[0];
	gl_ctx->verts[j].col[1] = gl_ctx->fg[1];
	gl_ctx->verts[j].col[2] = gl_ctx->fg[2];
	gl_ctx->verts[j].col[3] = gl_ctx->fg[3];
	gl_ctx->verts[j].uv[0] =  scale_u;
	gl_ctx->verts[j].uv[1] = (float)(1 - gl_ctx->flip_y) * scale_v;
	gl_ctx->vert_count ++;
	/* 3 */
	j = gl_ctx->vert_count;
	gl_ctx->verts[j].pos[0] = (float) tr[0];
	gl_ctx->verts[j].pos[1] = (float) tr[1];
	gl_ctx->verts[j].pos[2] = (float) tr[2];
	gl_ctx->verts[j].col[0] = gl_ctx->fg[0];
	gl_ctx->verts[j].col[1] = gl_ctx->fg[1];
	gl_ctx->verts[j].col[2] = gl_ctx->fg[2];
	gl_ctx->verts[j].col[3] = gl_ctx->fg[3];
	gl_ctx->verts[j].uv[0] =  scale_u;
	gl_ctx->verts[j].uv[1] = (float)(gl_ctx->flip_y) * scale_v;
	gl_ctx->vert_count ++;
	/* store vertex indexes in elements buffer - 2 triangles that share vertices  */
	/* 0 */
	j = gl_ctx->elem_count * 3;
	gl_ctx->elems[j] = gl_ctx->vert_count - 4;
	gl_ctx->elems[j+1] = gl_ctx->vert_count - 3;
	gl_ctx->elems[j+2] = gl_ctx->vert_count - 2;
	/* 1 */
	gl_ctx->elems[j+3] = gl_ctx->vert_count - 3;
	gl_ctx->elems[j+4] = gl_ctx->vert_count - 2;
	gl_ctx->elems[j+5] = gl_ctx->vert_count - 1;
	gl_ctx->elem_count+= 2;
	
	return 1;
}

int draw_gl_polygon (struct ogl *gl_ctx, int n, struct edge edges[]){
	/* draw a arbitrary and filled polygon, in openGL - use a scanline like algorithm */
	
	/* verify struct and buffers */
	if (!gl_ctx) return 0;
	if (!gl_ctx->verts) return 0;
	if (!gl_ctx->elems) return 0;
	
	if (2 * n > MAX_SCAN_LINES) return 0;
	
	int scan_lines[MAX_SCAN_LINES]; /* scan line defined by y coordinate */
	struct p_node nodes[MAX_P_NODES];
	
	int i = 0, j = 0, scan_count = 0, nodes_count = 0;
	
	/* define a scanline in each edge vertex */
	for (i = 0; i < n; i++){
		scan_lines[scan_count] = edges[i].y0;
		scan_count++;
		scan_lines[scan_count] = edges[i].y1;
		scan_count++;
	}
	
	/* sort and delete duplicates in scanline array */
	qsort(scan_lines, scan_count, sizeof(int), cmp_int);
	scan_count = unique (scan_count, scan_lines);
	
	/* perform polygon fill, by draw quads in scanline pairs */
	for (i = 1; i < scan_count; i++){ /* sweep scanlines by pairs */
		nodes_count = 0;
		
		for(j = 0; j < n; j++){ /* sweep polygon edges */
			if (((edges[j].y0 <= scan_lines[i - 1] && edges[j].y1 >= scan_lines[i - 1]) || 
			(edges[j].y1 <= scan_lines[i - 1] && edges[j].y0 >= scan_lines[i - 1])) &&
			((edges[j].y0 <= scan_lines[i] && edges[j].y1 >= scan_lines[i]) || 
			(edges[j].y1 <= scan_lines[i ] && edges[j].y0 >= scan_lines[i]))){
				/* find interceptions between edges and  current scanlines pair */
				
				/* up scanline */
				if (edges[j].y0 == scan_lines[i - 1]) { /*interception in one vertex */
					nodes[nodes_count].up = edges[j].x0;
					nodes[nodes_count].z = edges[j].z0;
				}
				else if (edges[j].y1 == scan_lines[i-1]) {  /*interception in other vertex */
					nodes[nodes_count].up = edges[j].x1;
					nodes[nodes_count].z = edges[j].z1;
				}
				else {  /* claculate interception in ege*/
					nodes[nodes_count].up = (int) round(edges[j].x1 + (double) (scan_lines[i - 1] - edges[j].y1)/(edges[j].y0 - edges[j].y1)*(edges[j].x0 - edges[j].x1));
					nodes[nodes_count].z = edges[j].z0;
				}
				
				/* lower scanline */
				if (edges[j].y0 == scan_lines[i]) { /*interception in one vertex */
					nodes[nodes_count].low = edges[j].x0;
					nodes[nodes_count].z = edges[j].z0;
				}
				else if (edges[j].y1 == scan_lines[i]) { /*interception in other vertex */
					nodes[nodes_count].low = edges[j].x1;
					nodes[nodes_count].z = edges[j].z1;
				}
				else { /* claculate interception in ege*/
					nodes[nodes_count].low = (int) round(edges[j].x1 + (double) (scan_lines[i] - edges[j].y1)/(edges[j].y0 - edges[j].y1)*(edges[j].x0 - edges[j].x1));
					nodes[nodes_count].z = edges[j].z0;
				}
				
				nodes_count++;
			}
		}
		
		/* sort interception nodes, by x coordinates */
		qsort(nodes, nodes_count, sizeof(struct p_node), cmp_node);
		
		/* draw quads (or triangles) between nodes pairs*/
		for (j = 1; j < nodes_count; j+= 2){ /* triangle, if up corners are same */
			if (nodes[j - 1].up == nodes[j].up){
				draw_gl_triang (gl_ctx, (int[]){nodes[j - 1].up, scan_lines[i-1], nodes[j - 1].z},
					(int[]){nodes[j - 1].low, scan_lines[i], nodes[j - 1].z},
					(int[]){nodes[j].low, scan_lines[i], nodes[j].z});
			}
			
			else if (nodes[j - 1].low == nodes[j].low){ /* triangle, if lower corners are same */
				draw_gl_triang (gl_ctx, (int[]){nodes[j - 1].up, scan_lines[i-1], nodes[j - 1].z},
					(int[]){nodes[j - 1].low, scan_lines[i], nodes[j - 1].z},
					(int[]){nodes[j].up, scan_lines[i-1], nodes[j].z});
			}
			else { /* general case - quadrilateral polygon */
				draw_gl_quad (gl_ctx, (int[]){nodes[j - 1].up, scan_lines[i-1], nodes[j - 1].z},
					(int[]){nodes[j - 1].low, scan_lines[i], nodes[j - 1].z},
					(int[]){nodes[j].up, scan_lines[i-1], nodes[j].z},
					(int[]){nodes[j].low, scan_lines[i], nodes[j].z});
			}
		}
	}
	
	return 1;
}

int graph_draw_gl(graph_obj * master, struct ogl *gl_ctx, struct draw_param param){
	if ((master == NULL) || (gl_ctx == NULL)) return 0;
	/* check if list is not empty */
	if (master->list == NULL) return 0;
	if (master->list->next == NULL) return 0;
	
	double x0, y0, z0, x1, y1, z1;
	int xd0, yd0, zd0, xd1, yd1, zd1;
	double dx, dy, dz, modulus, sine, cosine, uz;
	line_node *current = master->list->next;
	int i, iter, thick;
	
	if (!current) return 0;
	
	/*extention corners */
	xd0 = 0.5 + (master->ext_min_x - param.ofs_x) * param.scale;
	yd0 = 0.5 + (master->ext_min_y - param.ofs_y) * param.scale;
	xd1 = 0.5 + (master->ext_max_x - param.ofs_x) * param.scale;
	yd1 = 0.5 + (master->ext_max_y - param.ofs_y) * param.scale;
	zd0 = 0.5 + (master->ext_min_z - param.ofs_z) * param.scale;
	zd1 = 0.5 + (master->ext_max_z - param.ofs_z) * param.scale;
	
	
	/* verify if current graph is inside window*/
	/*
	if ( (xd0 < 0 && xd1 < 0) || 
		(xd0 > gl_ctx->win_w && xd1 > gl_ctx->win_w) || 
		(yd0 < 0 && yd1 < 0) || 
		(yd0 > gl_ctx->win_h && yd1 > gl_ctx->win_h) ) return 0;
	*/
	/* define color */
	bmp_color color = validate_color(master->color, param.list, param.subst, param.len_subst);
	gl_ctx->fg[0] = color.r;
	gl_ctx->fg[1] = color.g;
	gl_ctx->fg[2] = color.b;
	gl_ctx->fg[3] = color.a;
	
	/* check if graph is legible (greater then 3 pixels) */
	if (xd1 - xd0 < 3 && yd1 - yd0 < 3 && zd1 - zd0 < 3 && current->next != NULL){
		/* draw a single triangle if not legible */
		draw_gl_triang (gl_ctx, (int[]){xd0, yd0, zd0}, (int[]){xd0, yd1, zd0}, (int[]){xd1, yd0, zd0});
		return 0;
	}
	
	/* if has a bitmap image associated */
	if (master->img){
		/* base colors */
		bmp_color white = {.r = 255, .g = 255, .b =255, .a = 255};
		bmp_color transp = {.r = 255, .g = 255, .b =255, .a = 0};
		
		int tl[3], bl[3], tr[3], br[3];
		/* apply  offset an scale */
		/* first vertice */
		bl[0] = 0.5 + (current->x0 - param.ofs_x) * param.scale;
		bl[1] = 0.5 + (current->y0 - param.ofs_y) * param.scale;
		bl[2] = 0.5 + (current->z0 - param.ofs_z) * param.scale;
		/* second vertice */
		br[0] = 0.5 + (current->x1 - param.ofs_x) * param.scale;
		br[1] = 0.5 + (current->y1 - param.ofs_y) * param.scale;
		br[2] = 0.5 + (current->z1 - param.ofs_z) * param.scale;
		current = current->next;
		if (!current) return 0;
		/* 3# vertice */
		tr[0] = 0.5 + (current->x1 - param.ofs_x) * param.scale;
		tr[1] = 0.5 + (current->y1 - param.ofs_y) * param.scale;
		tr[2] = 0.5 + (current->z1 - param.ofs_z) * param.scale;
		current = current->next;
		if (!current) return 0;
		/* 4# vertice */
		tl[0] = 0.5 + (current->x1 - param.ofs_x) * param.scale;
		tl[1] = 0.5 + (current->y1 - param.ofs_y) * param.scale;
		tl[2] = 0.5 + (current->z1 - param.ofs_z) * param.scale;
		
		/* draw a frame */
		color = validate_color(transp, param.list, param.subst, param.len_subst);
		gl_ctx->fg[0] = color.r;
		gl_ctx->fg[1] = color.g;
		gl_ctx->fg[2] = color.b;
		gl_ctx->fg[3] = color.a;
		draw_gl_line (gl_ctx, bl, br, 1 + param.inc_thick);
		draw_gl_line (gl_ctx, br, tr, 1 + param.inc_thick);
		draw_gl_line (gl_ctx, tr, tl, 1 + param.inc_thick);
		draw_gl_line (gl_ctx, tl, bl, 1 + param.inc_thick);
		
		/* draw bitmap image */
		draw_gl (gl_ctx, 1); /* force draw previous commands and cleanup */
		/* choose blank base color */
		color = validate_color(white, param.list, param.subst, param.len_subst);
		gl_ctx->fg[0] = color.r;
		gl_ctx->fg[1] = color.g;
		gl_ctx->fg[2] = color.b;
		gl_ctx->fg[3] = color.a;
		/* prepare for new opengl commands */
    #ifndef GLES2
    if (gl_ctx->elems == NULL){
      gl_ctx->verts = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
      gl_ctx->elems = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
    }
    #endif
		glUniform1i(gl_ctx->tex_uni, 1); /* choose second texture */
		/* finally draw image */
		draw_gl_image_quad(gl_ctx, tl, bl, tr, br, master->img);
		draw_gl (gl_ctx, 1); /* force draw and cleanup */
		
		return 1;
	}
	
	if (master->flags & FILLED){
		/* filled polyon */
		i = 0;
		struct edge edges[2 * MAX_SCAN_LINES];
		
		while(current){ /*sweep the list content */
			/* apply the scale and offset */
			xd0 = 0.5 + (current->x0 - param.ofs_x) * param.scale;
			yd0 = 0.5 + (current->y0 - param.ofs_y) * param.scale;
			zd0 = 0.5 + (current->z0 - param.ofs_z) * param.scale;
			xd1 = 0.5 + (current->x1 - param.ofs_x) * param.scale;
			yd1 = 0.5 + (current->y1 - param.ofs_y) * param.scale;
			zd1 = 0.5 + (current->z1 - param.ofs_z) * param.scale;
			
			/* build edges array */
			edges[i] = (struct edge){xd0, yd0, zd0, xd1, yd1, zd1};
			
			current = current->next; /* go to next */
			if (i < 2 * MAX_SCAN_LINES - 1) i++;
			else current = NULL;
		}
		
		if (i > 2){
			if (i < 4){
				//int draw_gl_triang (struct ogl *gl_ctx, int p0[3], int p1[3], int p2[3]);
				draw_gl_triang (gl_ctx, (int []){edges[0].x0, edges[0].y0, edges[0].z0},
					(int []){edges[1].x0, edges[1].y0, edges[1].z0},
					(int []){edges[2].x0, edges[2].y0, edges[2].z0});
			}
			else if (i < 5) {
				//int draw_gl_quad (struct ogl *gl_ctx, int tl[3], int bl[3], int tr[3], int br[3]);
				draw_gl_quad (gl_ctx, (int []){edges[0].x0, edges[0].y0, edges[0].z0},
					(int []){edges[1].x0, edges[1].y0, edges[1].z0},
					(int []){edges[3].x0, edges[3].y0, edges[3].z0},
					(int []){edges[2].x0, edges[2].y0, edges[2].z0});
			}
			else {
				draw_gl_polygon (gl_ctx, i, edges);
			}
			
		}
	}
	else {
		/* set the thickness */
		if (master->flags & THICK_CONST) thick = (int) round(master->tick) + param.inc_thick;
		else thick = (int) round(master->tick * param.scale) + param.inc_thick;
		
		double patt_len = 0.0;
		/* get the pattern length */
		for (i = 0; i < master->patt_size && i < 20; i++){
			patt_len += fabs(master->pattern[i]);
		}
		
		if (master->patt_size > 1 &&   /* if graph is dashed lines */
			patt_len * param.scale > 3.0  /* and line pattern is legible (more then 3 pixels) */
		) {
			int patt_i = 0, patt_a_i = 0, patt_p_i = 0, draw;
			double patt_int, patt_part, patt_rem = 0.0, patt_acc, patt_rem_n;
			
			double p1x, p1y, p1z, p2x, p2y, p2z;
			double last;
			
			/* draw the lines */
			while(current){ /*sweep the list content */
				
				x0 = current->x0;
				y0 = current->y0;
				z0 = current->z0;
				x1 = current->x1;
				y1 = current->y1;
				z1 = current->z1;
				
				/* get polar parameters of line */
				dx = x1 - x0;
				dy = y1 - y0;
				dz = z1 - z0;
				modulus = sqrt(pow(dx, 2) + pow(dy, 2) + pow(dz, 2));
				cosine = 1.0;
				sine = 0.0;
				uz = 0.0;
				
				if (modulus > TOLERANCE){
					cosine = dx/modulus;
					sine = dy/modulus;
					uz = dz/modulus;
				}
				
				/* initial point */
				draw = master->pattern[patt_i] >= 0.0;
				p1x = ((x0 - param.ofs_x) * param.scale);
				p1y = ((y0 - param.ofs_y) * param.scale);
				p1z = ((z0 - param.ofs_z) * param.scale);
				
				if (patt_rem <= modulus){ /* current segment needs some iterations over pattern */
				
					/* find how many interations over whole pattern */ 
					patt_part = modf((modulus - patt_rem)/patt_len, &patt_int);
					patt_part *= patt_len; /* remainder for the next step*/
					
					/* find how many interations over partial pattern */
					patt_a_i = 0;
					patt_p_i = patt_i;
					if (patt_rem > 0) patt_p_i++;
					if (patt_p_i >= master->patt_size) patt_p_i = 0;
					patt_acc = fabs(master->pattern[patt_p_i]);
					
					patt_rem_n = patt_part; /* remainder pattern for next segment continues */
					if (patt_part < patt_acc) patt_rem_n = patt_acc - patt_part;
					
					last = modulus - patt_int*patt_len - patt_rem; /* the last stroke (pattern fractional part) of current segment*/
					for (i = 0; i < master->patt_size && i < 20; i++){
						patt_a_i = i;
						if (patt_part < patt_acc) break;
						
						last -= fabs(master->pattern[patt_p_i]);
						
						patt_p_i++;
						if (patt_p_i >= master->patt_size) patt_p_i = 0;
						
						patt_acc += fabs(master->pattern[patt_p_i]);
						
						patt_rem_n = patt_acc - patt_part;
					}
					
					/* first stroke - remainder of past pattern*/
					p2x = patt_rem * param.scale * cosine + p1x;
					p2y = patt_rem * param.scale * sine + p1y;
					p2z = patt_rem * param.scale * uz + p1z;
					
					if (patt_rem > 0) {
						/*------------- complex line type ----------------*/
						if (master->cmplx_pat[patt_i] != NULL)// &&  /* complex element */
							//p2x > 0 && p2x < gl_ctx->win_w && /* inside bound parameters */
							//p2y > 0 && p2y < gl_ctx->win_h )
						{
							list_node *cplx = master->cmplx_pat[patt_i]->next;
							graph_obj *cplx_gr = NULL;
							line_node *cplx_lin = NULL;
							
							/* sweep the main list */
							while (cplx != NULL){
								if (cplx->data){
									cplx_gr = (graph_obj *)cplx->data;
									cplx_lin = cplx_gr->list->next;
									/* draw the lines */
									while(cplx_lin){ /*sweep the list content */
										xd0 = 0.5 + p2x + ((cplx_lin->x0 * cosine -  cplx_lin->y0 * sine) * param.scale);
										yd0 = 0.5 + p2y + ((cplx_lin->x0 * sine +  cplx_lin->y0 * cosine) * param.scale);
										zd0 = 0.5 + p2z;
										xd1 = 0.5 + p2x + ((cplx_lin->x1 * cosine -  cplx_lin->y1 * sine) * param.scale);
										yd1 = 0.5 + p2y + ((cplx_lin->x1 * sine +  cplx_lin->y1 * cosine) * param.scale);
										zd1 = 0.5 + p2z;
										
										//bmp_line(img, xd0, yd0, xd1, yd1);
										draw_gl_line (gl_ctx, (int []){xd0, yd0, zd0}, (int []){ xd1, yd1, zd1}, 0);
										
										cplx_lin = cplx_lin->next; /* go to next */
									}
								}
								cplx = cplx->next;
							}
						}
						/*------------------------------------------------------*/
						
						patt_i++;
						if (patt_i >= master->patt_size) patt_i = 0;
						
						if (draw){
							//bmp_line_norm(img, p1x, p1y, p2x, p2y, -sine, cosine);
							xd0 = p1x + 0.5; yd0 = p1y + 0.5; zd0 = p1z + 0.5;
							xd1 = p2x + 0.5; yd1 = p2y + 0.5; zd1 = p2z + 0.5;
							if (xd0 != xd1 || yd0 != yd1 || zd0 != zd1){
								draw_gl_line (gl_ctx, (int []){xd0, yd0, zd0}, (int []){ xd1, yd1, zd1}, thick);
							} else { /* draw a dot */
								float t = thick, tx, ty;
								if (t < 2.0) t = 2.0;
								tx = t * 0.5 * cosine;
								ty = t * 0.5 * sine;
								draw_gl_quad (gl_ctx, (int []){p1x - tx - ty + 0.5, p1y + tx - ty, zd0}, 
									(int []){p1x - tx + ty, p1y - tx - ty, zd0}, 
									(int []){p1x + tx - ty, p1y + tx + ty, zd0},
									(int []){p1x + tx + ty, p1y - tx + ty, zd0});
							}
						}
					}
					
					patt_rem = patt_rem_n; /* for next segment */
					p1x = p2x;
					p1y = p2y;
					
					/* draw pattern */
					iter = (int) (patt_int * (master->patt_size)) + patt_a_i;
					for (i = 0; i < iter; i++){					
						draw = master->pattern[patt_i] >= 0.0;
						p2x = fabs(master->pattern[patt_i]) * param.scale * cosine + p1x;
						p2y = fabs(master->pattern[patt_i]) * param.scale * sine + p1y;
						p2z = fabs(master->pattern[patt_i]) * param.scale * uz + p1z;
						if (draw){
							//bmp_line_norm(img, p1x, p1y, p2x, p2y, -sine, cosine);
							xd0 = p1x + 0.5; yd0 = p1y + 0.5; zd0 = p1z + 0.5;
							xd1 = p2x + 0.5; yd1 = p2y + 0.5; zd1 = p2z + 0.5;
							if (xd0 != xd1 || yd0 != yd1 || zd0 != zd1){
								draw_gl_line (gl_ctx, (int []){xd0, yd0, zd0}, (int []){ xd1, yd1, zd1}, thick);
							} else { /* draw a dot */
								float t = thick, tx, ty;
								if (t < 2.0) t = 2.0;
								tx = t * 0.5 * cosine;
								ty = t * 0.5 * sine;
								draw_gl_quad (gl_ctx, (int []){p1x - tx - ty, p1y + tx - ty, zd0}, 
									(int []){p1x - tx + ty, p1y - tx - ty, zd0}, 
									(int []){p1x + tx - ty, p1y + tx + ty, zd0},
									(int []){p1x + tx + ty, p1y - tx + ty, zd0});
							}
						}
						p1x = p2x;
						p1y = p2y;
						p1z = p2z;
						
						/*------------- complex line type ----------------*/
						if (master->cmplx_pat[patt_i] != NULL)// &&  /* complex element */
							//p1x > 0 && p1x < gl_ctx->win_w && /* inside bound parameters */
							//p1y > 0 && p1y < gl_ctx->win_h )
						{
							list_node *cplx = master->cmplx_pat[patt_i]->next;
							graph_obj *cplx_gr = NULL;
							line_node *cplx_lin = NULL;
							
							/* sweep the main list */
							while (cplx != NULL){
								if (cplx->data){
									cplx_gr = (graph_obj *)cplx->data;
									cplx_lin = cplx_gr->list->next;
									/* draw the lines */
									while(cplx_lin){ /*sweep the list content */
										
										xd0 = 0.5 + p1x + ((cplx_lin->x0 * cosine -  cplx_lin->y0 * sine) * param.scale);
										yd0 = 0.5 + p1y + ((cplx_lin->x0 * sine +  cplx_lin->y0 * cosine) * param.scale);
										zd0 = 0.5 + p1z;
										xd1 = 0.5 + p1x + ((cplx_lin->x1 * cosine -  cplx_lin->y1 * sine) * param.scale);
										yd1 = 0.5 + p1y + ((cplx_lin->x1 * sine +  cplx_lin->y1 * cosine) * param.scale);
										zd1 = 0.5 + p1z;
										
										//bmp_line(img, xd0, yd0, xd1, yd1);
										draw_gl_line (gl_ctx, (int []){xd0, yd0, zd0}, (int []){ xd1, yd1, zd1}, 0);
										
										cplx_lin = cplx_lin->next; /* go to next */
									}
								}
								cplx = cplx->next;
							}
						}
						/*------------------------------------------------------*/
						
						patt_i++;
						if (patt_i >= master->patt_size) patt_i = 0;
					}
					
					p2x = last * param.scale * cosine + p1x;
					p2y = last * param.scale * sine + p1y;
					p2z = last * param.scale * uz + p1z;
					draw = master->pattern[patt_i] >= 0.0;
					if (draw) {//bmp_line_norm(img, p1x, p1y, p2x, p2y, -sine, cosine);
						xd0 = p1x + 0.5; yd0 = p1y + 0.5; zd0 = p1z + 0.5;
						xd1 = p2x + 0.5; yd1 = p2y + 0.5; zd1 = p2z + 0.5;
						draw_gl_line (gl_ctx, (int []){xd0, yd0, zd0}, (int []){ xd1, yd1, zd1}, thick);
					}
				}
				else{ /* current segment is in same past iteration pattern */
					p2x = modulus * param.scale * cosine + p1x;
					p2y = modulus * param.scale * sine + p1y;
					p2z = modulus * param.scale * uz + p1z;
					
					patt_rem -= modulus;
				
					if (draw){
						//bmp_line_norm(img, p1x, p1y, p2x, p2y, -sine, cosine);
						xd0 = p1x + 0.5; yd0 = p1y + 0.5; zd0 = p1z + 0.5;
						xd1 = p2x + 0.5; yd1 = p2y + 0.5; zd1 = p2z + 0.5;
						draw_gl_line (gl_ctx, (int []){xd0, yd0, zd0}, (int []){ xd1, yd1, zd1}, thick);
					}
					p1x = p2x;
					p1y = p2y;
					p1z = p2z;
				}
			
				current = current->next; /* go to next */
			}
		}
		else{ /* for continuous lines*/
			while(current){ /*sweep the list content */
				/* apply the scale and offset */
				
				xd0 = 0.5 + (current->x0 - param.ofs_x) * param.scale;
				yd0 = 0.5 + (current->y0 - param.ofs_y) * param.scale;
				zd0 = 0.5 + (current->z0 - param.ofs_z) * param.scale;
				xd1 = 0.5 + (current->x1 - param.ofs_x) * param.scale;
				yd1 = 0.5 + (current->y1 - param.ofs_y) * param.scale;
				zd1 = 0.5 + (current->z1 - param.ofs_z) * param.scale;
				
				if (master->pattern[0] >= 0.0)
					//bmp_line_norm(img, x0, y0, x1, y1, -sine, cosine);
					//draw_gl_line (gl_ctx, (int []){xd0, yd0, zd0}, (int []){ xd1, yd1, zd1}, thick);
					if (xd0 != xd1 || yd0 != yd1 || zd0 != zd1){
						draw_gl_line (gl_ctx, (int []){xd0, yd0, zd0}, (int []){ xd1, yd1, zd1}, thick);
					} else { /* draw a dot */
						int t = thick * 0.5 + 0.5;
						if (t < 1) t = 1;
						draw_gl_quad (gl_ctx, (int []){xd0 - t, yd0 + t, zd0}, (int []){xd0 - t, yd0 - t, zd0}, 
							(int []){xd0 + t, yd0 + t, zd0}, (int []){xd0 + t, yd0 - t, zd0});
					}
				
				current = current->next; /* go to next */
			}
		}
	}
	
}

int draw_gl (struct ogl *gl_ctx, int force){
	if (!gl_ctx) return 0;
	if (gl_ctx->elem_count > MAX_TRIANG_2 || force ){
		float win_mtx[4][4];
		float res_mtx[4][4];
		
		win_mtx[0][0] = 2.0 / gl_ctx->win_w;
		win_mtx[0][1] = 0.0;
		win_mtx[0][2] = 0.0;
		win_mtx[0][3] = 0.0;
		win_mtx[1][0] = 0.0;
		win_mtx[1][1] = ( (gl_ctx->flip_y) ? -1.0 : 1.0 )  * 2.0 / gl_ctx->win_h;
		win_mtx[1][2] = 0.0;
		win_mtx[1][3] = 0.0;
		win_mtx[2][0] = 0.0;
		win_mtx[2][1] = 0.0;
		win_mtx[2][2] = -0.000001;
		win_mtx[2][3] = 0.0;
		win_mtx[3][0] = -1.0;
		win_mtx[3][1] = ( (gl_ctx->flip_y) ? -1.0 : 1.0 ) * -1.0;
		win_mtx[3][2] = 0.0;
		win_mtx[3][3] = 1.0;
		
		matrix4_mul(win_mtx[0], gl_ctx->transf[0], res_mtx[0]);
		
		glUniformMatrix4fv(gl_ctx->transf_uni, 1,  GL_FALSE, res_mtx[0]);
  #ifndef GLES2
    if(gl_ctx->elems){
      glUnmapBuffer(GL_ARRAY_BUFFER);
      glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
    }
  #else
    glBufferSubData(GL_ARRAY_BUFFER, 0, gl_ctx->vert_count*sizeof(struct Vertex), gl_ctx->verts);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, gl_ctx->elem_count*3*sizeof(GLuint), gl_ctx->elems);
  #endif
		glDrawElements(GL_TRIANGLES, gl_ctx->elem_count*3, GL_UNSIGNED_INT, 0);
		gl_ctx->vert_count = 0;
		gl_ctx->elem_count = 0;
  #ifndef GLES2
		gl_ctx->elems = NULL;
		gl_ctx->verts = NULL;
  #endif
		glUniform1i(gl_ctx->tex_uni, 0);
		
		return 1;
	}
	return 0;
}

int graph_list_draw_gl(list_node *list, struct ogl *gl_ctx, struct draw_param param){
	list_node *current = NULL;
	graph_obj *curr_graph = NULL;
	int ok = 0;
		
	if (list != NULL){
		current = list->next;
		
		/* sweep the main list */
		while (current != NULL){
			if (current->data){
				curr_graph = (graph_obj *)current->data;
				
				/*
        #ifndef GLES2
        if (gl_ctx->elems == NULL){
          gl_ctx->verts = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
          gl_ctx->elems = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
        }
        #endif
				gl_ctx->vert_count = 0;
				gl_ctx->elem_count = 0;
				glUniform1i(gl_ctx->tex_uni, 0);*/
				
				graph_draw_gl(curr_graph, gl_ctx, param);
				
				/*
        #ifndef GLES2
          glUnmapBuffer(GL_ARRAY_BUFFER);
          glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
        #else
          glBufferSubData(GL_ARRAY_BUFFER, 0, gl_ctx->vert_count*sizeof(struct Vertex), gl_ctx->verts);
          glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, gl_ctx->elem_count*3*sizeof(GLuint), gl_ctx->elems);
        #endif
				glDrawElements(GL_TRIANGLES, gl_ctx->elem_count*3, GL_UNSIGNED_INT, 0);*/
			}
			current = current->next;
		}
		ok = 1;
	}
	return ok;
}

int graph_list_draw_gl2(list_node *list, struct ogl *gl_ctx, struct draw_param param){
	list_node *current = NULL;
	graph_obj *curr_graph = NULL;
	int ok = 0;
		
	if (list == NULL) return 0;
	if (!gl_ctx) return 0;
	
	current = list->next;
	
	/* sweep the main list */
	while (current != NULL){
		if (current->data){
			curr_graph = (graph_obj *)current->data;
			#ifndef GLES2
			if (gl_ctx->elems == NULL){
				gl_ctx->verts = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
				gl_ctx->elems = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
			}
			#endif
			graph_draw_gl(curr_graph, gl_ctx, param);
			
			draw_gl (gl_ctx, 0);
		}
		current = current->next;
	}
	ok = 1;
	return ok;
}

int dxf_list_draw_gl(list_node *list, struct ogl *gl_ctx,  struct draw_param param){
	list_node *current = NULL;
		
	if (list != NULL){
		current = list->next;
		
		/* starts the content sweep  */
		while (current != NULL){
			if (current->data){
				if (((dxf_node *)current->data)->type == DXF_ENT){ /* DXF entity */
					// -------------------------------------------
					/* draw each entity */
					graph_list_draw_gl2(((dxf_node *)current->data)->obj.graphics, gl_ctx, param);
					
					//---------------------------------------
				}
			}
			current = current->next;
		}
	}
}

int dxf_ents_draw_gl(dxf_drawing *drawing, struct ogl *gl_ctx, struct draw_param param){
	dxf_node *current = NULL;
	//int lay_idx = 0;
		
	if ((drawing->ents != NULL) && (drawing->main_struct != NULL)){
		current = drawing->ents->obj.content->next;
		
		/* starts the content sweep  */
		while (current != NULL){
			if (current->type == DXF_ENT){ /* DXF entity */
				/*verify if entity layer is on and thaw */
				if ((!drawing->layers[current->obj.layer].off) && 
					(!drawing->layers[current->obj.layer].frozen)){
					
					/* draw each entity */
					graph_list_draw_gl2(current->obj.graphics, gl_ctx, param);
					
				}
			}
			current = current->next;
		}
	}
}