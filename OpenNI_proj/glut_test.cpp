/*
 * OpenGLSamples (openglsamples.sf.net) Examples
 * VC++ users should create a Win32 Console project and link 
 * the program with glut32.lib, glu32.lib, opengl32.lib
 *
 * GLUT can be downloaded from http://www.xmission.com/~nate/glut.html
 * OpenGL is by default installed on your system.
 * For an installation of glut on windows for MS Visual Studio 2010 see: http://nafsadh.wordpress.com/2010/08/20/glut-in-ms-visual-studio-2010-msvs10/
 *
 */

#include <stdio.h>
#include <windows.h>	   // Standard header for MS Windows applications
#include <GL/gl.h>		   // Open Graphics Library (OpenGL) header
#include <GL/glut.h>	   // The GL Utility Toolkit (GLUT) Header
#include <vector>
#include "linmath.h"
#define KEY_ESCAPE 27
#define SMALL_SIDE 0.05f
#define DEG_TO_RAD 3.14159 / 180
//float sxx = 0, syy = 0, szz = 0;
//float anglex=0, angley=0;

typedef struct {
    int width;
	int height;
	char* title;

	float field_of_view_angle;
	float z_near;
	float z_far;
} glutWindow;

typedef struct {
	float x1, y1, z1, x2, y2, z2;
} lineSegment;

//std::vector<lineSegment> lines;
//bool drawing;
//lineSegment currentLine;

glutWindow win;
void display() 
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		     // Clear Screen and Depth Buffer
	glLoadIdentity();
	glTranslatef(0.0f,0.0f,-3.0f);
	/*
	mat4x4 temp, temp2;
	mat4x4_identity(temp);
	mat4x4_rotate_X(temp2, temp, anglex);
	mat4x4_rotate_Y(temp, temp2, angley);
	mat4x4_mul(temp2, temp, accum);
	mat4x4 id;
	mat4x4_identity(id);
	mat4x4_mul(accum, id, temp2);
	const GLfloat* multmat = copyMatrix(accum);
	glMultMatrixf(multmat);
	anglex = 0;
	angley = 0;
	*/
	//glRotatef(-anglex, 0, 1, 0);
	//glRotatef(-angley, 1, 0, 0);
	//glTranslatef(0.0f,0.0f,-3.0f);			
	/*
	 * Triangle code starts here
	 * 3 verteces, 3 colors.
	 */
	/*
	glBegin(GL_TRIANGLES);					
		glColor3f(0.0f,0.0f,1.0f);			
		glVertex3f( 0.0f, 1.0f, 0.0f);		
		glColor3f(0.0f,1.0f,0.0f);			
		glVertex3f(-1.0f,-1.0f, 0.0f);		
		glColor3f(1.0f,0.0f,0.0f);			
		glVertex3f( 1.0f,-1.0f, 0.0f);		
	glEnd();				
	*/
	/*
	GLfloat lightpos[] = {0.0f, 0.0f, 0.0f, 1.0f};
	GLfloat cyan[] = {0.f, .8f, .8f, 1.f};
	GLfloat red[] = {1.f, 0.f, 0.f, 1.f};
	GLfloat green[] = {0.f, 0.8f, 0.f, 1.f};
	glLightfv(GL_LIGHT0, GL_POSITION, lightpos);
	//glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, red);
	//drawFloor();
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, green);
	drawCube(0.0f, 0.0f, 0.0f, 0.1f);
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, red);
	drawCube(sxx, syy, szz, 0.1);
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, cyan);
	for(std::vector<lineSegment>::iterator it = lines.begin(); it != lines.end(); it++)
	{
		drawLine(it->x1, it->y1, it->z1, it->x2, it->y2, it->z2);
	}

	//oldx = oldx + frame.deltaPos.x * SCALE_FACTOR;
	//oldy = oldy + frame.deltaPos.y * SCALE_FACTOR;
	//oldz = oldz + frame.deltaPos.z * SCALE_FACTOR;
	glutSwapBuffers();
	*/
}


void initialize () 
{
    glMatrixMode(GL_PROJECTION);												// select projection matrix
    glViewport(0, 0, win.width, win.height);									// set the viewport
    glMatrixMode(GL_PROJECTION);												// set matrix mode
    glLoadIdentity();															// reset projection matrix
    GLfloat aspect = (GLfloat) win.width / win.height;
	gluPerspective(win.field_of_view_angle, aspect, win.z_near, win.z_far);		// set up a perspective projection matrix
    glMatrixMode(GL_MODELVIEW);													// specify which matrix is the current matrix
    glShadeModel( GL_SMOOTH );
    glClearDepth( 1.0f );														// specify the clear value for the depth buffer
    glEnable( GL_DEPTH_TEST );
    glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glDepthFunc( GL_LEQUAL );
    glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );						// specify implementation-specific hints
	glClearColor(0.0, 0.0, 0.0, 1.0);											// specify clear values for the color buffers
	//mat4x4_identity(accum);
}


void keyboard ( unsigned char key, int mousePositionX, int mousePositionY )		
{ 
  //switch ( key ) 
  //{
	 //   case KEY_ESCAPE:        
  //        exit ( 0 );   
  //        break;      
		//case 'z':
		//	sxx -= 0.1*cos(anglex*DEG_TO_RAD);
		//	szz += 0.1*sin(anglex*DEG_TO_RAD);
		//	break;
		//case 'x':
		//	syy -= 0.1;
		//	break;
		//case 'c':
		//	sxx += 0.1*cos(anglex*DEG_TO_RAD);
		//	szz -= 0.1*sin(anglex*DEG_TO_RAD);
		//	break;
		//case 's':
		//	syy += 0.1;
		//	break;
		//case 'f':
		//	sxx += 0.1*sin(anglex*DEG_TO_RAD);
		//	szz += 0.1*cos(anglex*DEG_TO_RAD);
		//	break;
		//case 'v':
		//	sxx -= 0.1*sin(anglex*DEG_TO_RAD);
		//	szz -= 0.1*cos(anglex*DEG_TO_RAD);
		//	break;
		//case 'j':
		//	anglex += -5;
		//	break;
		//case 'k':
		//	anglex += 5;
		//	break;
		//case 'i':
		//	angley += -5;
		//	break;
		//case 'm':
		//	angley += 5;
		//	break;
		//case VK_RETURN:
		//	if(!drawing)
		//	{
		//		currentLine.x1 = sxx;
		//		currentLine.y1 = syy;
		//		currentLine.z1 = szz;
		//	}
		//	else
		//	{
		//		currentLine.x2 = sxx;
		//		currentLine.y2 = syy;
		//		currentLine.z2 = szz;
		//		lines.push_back(currentLine);
		//	}
		//	drawing = !drawing;
		//	break;
  //  default:      
  //    break;
  //}
}

//int main(int argc, char **argv) 
int notmain(int argc, char **argv)
{
	// set window values
	win.width = 640;
	win.height = 480;
	win.title = "OpenGL/GLUT Example. Visit http://openglsamples.sf.net ";
	win.field_of_view_angle = 45;
	win.z_near = 1.0f;
	win.z_far = 500.0f;

	// initialize and run program
	glutInit(&argc, argv);                                      // GLUT initialization
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH );  // Display Mode
	glutInitWindowSize(win.width,win.height);					// set window size
	glutCreateWindow(win.title);								// create Window
	glutDisplayFunc(display);									// register Display Function
	glutIdleFunc( display );									// register Idle Function
    glutKeyboardFunc( keyboard );								// register Keyboard Handler
	initialize();
	glutMainLoop();												// run GLUT mainloop
	return 0;
}
