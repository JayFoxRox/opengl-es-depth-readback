
/* Copyright (c) Mark J. Kilgard, 1997. */
/* Copyright (c) Jannik Vogel, 2017. */

/* This program is freely distributable without licensing fees 
   and is provided without guarantee or warrantee expressed or 
   implied. This program is -not- in the public domain. */

#include <GL/glut.h>

GLfloat light_diffuse[] = {1.0, 0.0, 0.0, 1.0};  /* Red diffuse light. */
GLfloat light_position[] = {1.0, 1.0, 1.0, 0.0};  /* Infinite light location. */
GLfloat n[6][3] = {  /* Normals for the 6 faces of a cube. */
  {-1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {1.0, 0.0, 0.0},
  {0.0, -1.0, 0.0}, {0.0, 0.0, 1.0}, {0.0, 0.0, -1.0} };
GLint faces[6][4] = {  /* Vertex indices for the 6 faces of a cube. */
  {0, 1, 2, 3}, {3, 2, 6, 7}, {7, 6, 5, 4},
  {4, 5, 1, 0}, {5, 6, 2, 1}, {7, 4, 0, 3} };
GLfloat v[8][3];  /* Will be filled in with X,Y,Z vertexes. */

void
drawBox(void)
{
  int i;

  for (i = 0; i < 6; i++) {
    glBegin(GL_QUADS);
    glNormal3fv(&n[i][0]);
    glVertex3fv(&v[faces[i][0]][0]);
    glVertex3fv(&v[faces[i][1]][0]);
    glVertex3fv(&v[faces[i][2]][0]);
    glVertex3fv(&v[faces[i][3]][0]);
    glEnd();
  }
}

int mx;
int my;

void
display(void)
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glPushMatrix();


  static float t = 0.0f;

  /* Adjust cube position to be asthetic angle. */
  glTranslatef(0.0, 0.0, -1.0);
  glRotatef(60 + t, 1.0, 0.0, 0.0);
  glRotatef(-20 + t * 0.5f, 0.0, 0.0, 1.0);

  //t += 1.0f;

  drawBox();


  // Get the reference depth. This is not possible in GLES
  unsigned short depth;
  glReadPixels(mx, my, 1, 1, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, &depth);
  printf("%d, %d: 0x%04X\n", mx, my, depth);


  // Binary search the depth of mx, my
  // In a real usage scenario you'd probably only do this without anti-aliasing.
  // You'd also overwrite a single pixel or create a backup of the color buffer.
  {
    unsigned char color[4];
    glReadPixels(mx, my, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &color);
    printf("%d, %d: %02X %02X %02X %02X\n", mx, my, color[0], color[1], color[2], color[3]);

    // Try to figure out the depth
    unsigned char set[3];
    memcpy(set, color, 3);

    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    //glOrtho(-1.0f, -1.0f, +1.0f, +1.0f, -1.0f, +1.0f);
    glLoadIdentity();
    glDepthMask(GL_FALSE);
    glDisable(GL_LIGHTING);
    unsigned short barrier = 0x8000;
    unsigned short addend = 0x4000;
    unsigned int tests = 16;

    while(--tests) {
      float z = (float)barrier / (float)0xFFFF;
      z = -1.0f + z * 2.0f;

      printf("Testing %f\n", z);
      glBegin(GL_QUADS);
      set[0] ^= 0xFF;
      set[1] ^= 0xFF;
      set[2] ^= 0xFF;
      glColor3ubv(set);
      glVertex4f(+1.0, +1.0, z, 1.0f);
      glVertex4f(+1.0, -1.0, z, 1.0f);
      glVertex4f(-1.0, -1.0, z, 1.0f);
      glVertex4f(-1.0, +1.0, z, 1.0f);
      glEnd();

      uint8_t readback[4];
      glReadPixels(mx, my, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &readback);
      int visible = !memcmp(set, readback, 3);
      printf("was modified: %d\n", visible);
      memcpy(set, readback, 3);

      if (visible) {
        barrier += addend;
      } else {
        barrier -= addend;
      }
      addend >>= 1;
      printf("barrier: 0x%04X, addend is %d\n", barrier, addend);
    }
    printf("Barrier is 0x%04X == 0x%04X (real depth)?\n", barrier, depth);
    glDepthMask(GL_TRUE);
    glEnable(GL_LIGHTING);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
  }

  glPopMatrix();

  glutSwapBuffers();

  glutPostRedisplay();
}

void
init(void)
{
  /* Setup cube vertex data. */
  v[0][0] = v[1][0] = v[2][0] = v[3][0] = -1;
  v[4][0] = v[5][0] = v[6][0] = v[7][0] = 1;
  v[0][1] = v[1][1] = v[4][1] = v[5][1] = -1;
  v[2][1] = v[3][1] = v[6][1] = v[7][1] = 1;
  v[0][2] = v[3][2] = v[4][2] = v[7][2] = 1;
  v[1][2] = v[2][2] = v[5][2] = v[6][2] = -1;

  /* Enable a single OpenGL light. */
  glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
  glLightfv(GL_LIGHT0, GL_POSITION, light_position);
  glEnable(GL_LIGHT0);
  glEnable(GL_LIGHTING);

  /* Use depth buffering for hidden surface elimination. */
  glEnable(GL_DEPTH_TEST);

  /* Setup the view of the cube. */
  glMatrixMode(GL_PROJECTION);
  gluPerspective( /* field of view in degree */ 40.0,
    /* aspect ratio */ 1.0,
    /* Z near */ 1.0, /* Z far */ 10.0);
  glMatrixMode(GL_MODELVIEW);
  gluLookAt(0.0, 0.0, 5.0,  /* eye is at (0,0,5) */
    0.0, 0.0, 0.0,      /* center is at (0,0,0) */
    0.0, 1.0, 0.);      /* up is in positive Y direction */
}

void mouseMove(int x, int y) {
  int height = glutGet(GLUT_WINDOW_HEIGHT);
  mx = x;
  my = height - y;
}

void mouse(int button, int state, int x, int y) {
  mouseMove(x, y);
}

int
main(int argc, char **argv)
{
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutCreateWindow("red 3D lighted cube");
  glutDisplayFunc(display);
  glutMouseFunc(mouse);
  glutMotionFunc(mouseMove);
  glutPassiveMotionFunc(mouseMove);
  init();
  glutMainLoop();
  return 0;             /* ANSI C requires main to return int. */
}
