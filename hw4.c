// William Lyon
// hw4.c
// CSCI 441
// HW 4
// Loads temple.obj blender object file
// uses kixor.net object loader library

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <GLUT/glut.h>
#include "geom.h"
#include "objLoader.h"

struct Mesh {
	int mesh_created;
	Real* verts;
	Real* normals;
	Real* tcoords;
	GLuint* tris;
	GLsizei num_tris;
	GLsizei num_verts;
  GLsizei num_tcoords;
	GLuint texture_id;
	// vbo's
	int buf_created;
	GLuint vert_buf;
	GLuint normal_buf;
	GLuint tcoord_buf;
	GLuint index_buf;
};
typedef struct Mesh Mesh;

Mesh myMesh;

static void set_projection()
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
}
void mesh_draw (float* verts, GLuint* tris, int num_tris)
{
	Real normalVec[] = {0, 0, 0};

    glBegin (GL_TRIANGLES);
	int i;
	for (i=0; i<num_tris; i++)
	{
		//int tri = tri+i*3;

		float* v0 = verts + tris[i*3+0]*3;
		float* v1 = verts + tris[i*3+1]*3;
		float* v2 = verts + tris[i*3+2]*3;

		geom_calc_normal(v0, v1, v2, normalVec);

        glNormal3f(normalVec[0], normalVec[1], normalVec[2]);

        glVertex3f(v0[0], v0[1], v0[2]);
		glVertex3f(v1[0], v1[1], v1[2]);
		glVertex3f(v2[0], v2[1], v2[2]);
    }

    glEnd();
    glFlush();
}


/*
	descrip: Converts tris in param obj to param mesh object.

	params:
		obj : source obj model data (can be created with objloader_load())

		mesh_ret : mesh struct that will receive the geometry and texture from conversion

		err_txt_ret : can be NULL, if not, it should be at least 1024 in size and will receive
			error description if an error occurs

	returns: 1 if success, else 0

	notes:
		* currently, the obj model must ONLY have faces with 3 verts

		* Also, each vertex position should be associated with a unique texture and normal.
			If the same vertex position is associated with multiple textures/normals in
			face points, only the most recent vertex/texture/normal triplet will be used.
			The 'singulate_obj_verts.lua script can be used to de-multiplex verts in an obj file.

		* currently, material data in obj->material_list is not used, except for
		  obj->material_list[0]->texture_filename, which if not NULL will be used to
			create a single texture for the mesh
*/
int model_obj2mesh(const struct obj_scene_data* obj, struct Mesh* mesh_ret, char* err_txt_ret) {
	struct Mesh* m = mesh_ret;
    memset(mesh_ret, 0, sizeof(struct Mesh));

  // check whether tcoords are defined for all triangles
  // count the number of faces that are triangles
	int i;
	int tri_face_count = 0;
	for(i = 0; i < obj->face_count; ++i) {
		obj_face* face = obj->face_list[i];
		// just skip non-tri faces for now
		if(face->vertex_count != 3) {
			continue;
		}
		++tri_face_count;
	}
	m->num_tris = tri_face_count;

  // allocate vertex, tcoord, normal and tri arrays
	size_t n;
	n = sizeof(Real) * 3 * obj->vertex_count;
	m->verts = (Real*) malloc(n);
	memset(m->verts, 0, n);
	m->num_verts = obj->vertex_count;

	n = sizeof(GLuint) * 3 * tri_face_count;
	m->tris = (GLuint*) malloc(n);
	memset(m->tris, 0, n);

  // iterate all faces, get mesh data for tri faces
	for(i = 0; i < obj->face_count; ++i) {
		obj_face* face = obj->face_list[i];
		if(face->vertex_count != 3)
			continue;

		int j;
		int fv[3];
		for(j = 0; j < 3; ++j) {
			fv[j] = face->vertex_index[j];
		}

		int b;

    // verts
		double* ov0 = obj->vertex_list[fv[0]]->e;
		double* ov1 = obj->vertex_list[fv[1]]->e;
		double* ov2 = obj->vertex_list[fv[2]]->e;

		b = fv[0]*3;
		m->verts[b    ] = ov0[0];
		m->verts[b + 1] = ov0[1];
		m->verts[b + 2] = ov0[2];
		Real* v0 = m->verts + b;

		b = fv[1]*3;
		m->verts[b    ] = ov1[0];
		m->verts[b + 1] = ov1[1];
		m->verts[b + 2] = ov1[2];
		Real* v1 = m->verts + b;

		b = fv[2]*3;
		m->verts[b    ] = ov2[0];
		m->verts[b + 1] = ov2[1];
		m->verts[b + 2] = ov2[2];
		Real* v2 = m->verts + b;

		// copy indices
		m->tris[i*3  ] = fv[0];
		m->tris[i*3+1] = fv[1];
		m->tris[i*3+2] = fv[2];
	}
	m->mesh_created = 1;

	return 1;
}
static void check_gl_errors() {
    GLuint err;
    while((err=glGetError())!=GL_NO_ERROR) {
        printf("GL ERROR: %s\n", gluErrorString(err));
    }
}

static void set_more_lighting() {
    glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_NORMALIZE);

    float pos[4] = {10,10, -10, 1};
    glLightfv(GL_LIGHT0, GL_POSITION, pos);

}

static void display()
{
    set_more_lighting();
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // draw mesh here
    static float temple_color[3] = {0.0, 0.5, 0.7};
    glColor3fv(temple_color);

    mesh_draw(myMesh.verts, myMesh.tris, myMesh.num_tris);

        glutSwapBuffers();
        check_gl_errors();
}

void ChangeSize(GLsizei w, GLsizei h)       // update scene keeping same aspect ratio (1)
{
    GLfloat aspectRatio;
    if(h==0)
        h=1;

    int windowHeight;
    int windowWidth;

    windowHeight=h;
    windowWidth=w;

    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    aspectRatio = (GLfloat)w / (GLfloat)h;
    if(w<=h)
        glOrtho(-30.0, 30.0, -30.0 / aspectRatio, 30.0 / aspectRatio, 20.0, -20.0);
    else
        glOrtho(-30.0*aspectRatio, 30.0*aspectRatio, -30.0, 30.0, 20.0, -20.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void TimerFunction(int value)
{
    glRotatef(1, 0.0f, 1.0f, 0.0f);
    glutPostRedisplay();
    glutTimerFunc(33, TimerFunction, 1);
}
int main(int argc, char **argv)
{
	obj_scene_data data;
    if (!parse_obj_scene(&data, "temple.obj"))
        return 0;

    printf("Number of vertices: %i\n", data.vertex_count);


    char* errorText;

    model_obj2mesh(&data, &myMesh, errorText);

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA|GLUT_DEPTH);
    glutInitWindowSize(500, 500);
    glutCreateWindow("The Temple Of DOOM");
    glutDisplayFunc(display);

    set_projection();

    glRotatef(-10, 1.0f, 0.0f, 0.0f);
    glutReshapeFunc(ChangeSize);
    glutTimerFunc(33, TimerFunction, 1);
    glutMainLoop();
    printf("%s \n", errorText);
    delete_obj_data(&data);

    return 0;

}

