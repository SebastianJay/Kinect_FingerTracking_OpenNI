/****************************************************************************
*                                                                           *
*  OpenNI 1.x Alpha                                                         *
*  Copyright (C) 2011 PrimeSense Ltd.                                       *
*                                                                           *
*  This file is part of OpenNI.                                             *
*                                                                           *
*  OpenNI is free software: you can redistribute it and/or modify           *
*  it under the terms of the GNU Lesser General Public License as published *
*  by the Free Software Foundation, either version 3 of the License, or     *
*  (at your option) any later version.                                      *
*                                                                           *
*  OpenNI is distributed in the hope that it will be useful,                *
*  but WITHOUT ANY WARRANTY; without even the implied warranty of           *
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the             *
*  GNU Lesser General Public License for more details.                      *
*                                                                           *
*  You should have received a copy of the GNU Lesser General Public License *
*  along with OpenNI. If not, see <http://www.gnu.org/licenses/>.           *
*                                                                           *
****************************************************************************/
//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include <XnOS.h>
#if (XN_PLATFORM == XN_PLATFORM_MACOSX)
	#include <GLUT/glut.h>
#else
	#include <GL/glut.h>
#endif

#include "FingerAnalysisAPI.h"

#include <math.h>
#include <set>
#include <utility>
#include <vector>

#include <XnList.h>
#include <XnCppWrapper.h>
#include <XnFPSCalculator.h>

using namespace xn;

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------
//#define SAMPLE_XML_PATH "../../../Data/SamplesConfig.xml"
//#define SAMPLE_XML_PATH "../OpenNI_proj/SamplesConfig.xml"

#define DRAW_MODE 1

#define GL_WIN_SIZE_X 640
#define GL_WIN_SIZE_Y 480

#define DISPLAY_MODE_OVERLAY	1
#define DISPLAY_MODE_DEPTH		2
#define DISPLAY_MODE_IMAGE		3
#define DEFAULT_DISPLAY_MODE	DISPLAY_MODE_DEPTH

#define MAX_DEPTH 10000

#define DEPTH_THRESHOLD_MAX 1500
#define DEPTH_THRESHOLD_MIN 500

#define SCALE_FACTOR 0.005
#define ANGLE_SCALE_FACTOR 0.5
#define SMALL_SIDE 0.05f
#define DEG_TO_RAD 3.14159 / 180
#define TOO_BIG 50

#define CHECK_RC(rc, what)											\
	if (rc != XN_STATUS_OK)											\
	{																\
		printf("%s failed: %s\n", what, xnGetStatusString(rc));		\
		return rc;													\
	}

//---------------------------------------------------------------------------
// Globals
//---------------------------------------------------------------------------
float g_pDepthHist[MAX_DEPTH];
XnRGB24Pixel* g_pTexMap = NULL;
unsigned int g_nTexMapX = 0;
unsigned int g_nTexMapY = 0;

unsigned int g_nViewState = DEFAULT_DISPLAY_MODE;

Context g_context;
ScriptNode g_scriptNode;
DepthGenerator g_depth;
ImageGenerator g_image;
DepthMetaData g_depthMD;
ImageMetaData g_imageMD;

FingerTracker g_tracker;

XnFPSData g_xnFPS;
unsigned int g_depthFrame[640*480]; 
float sx=0, sy=0, sz=0;
float anglex=0, angley=0;
bool tracking = false;
typedef struct {
	float x1, y1, z1, x2, y2, z2;
} lineSegment;

std::vector<lineSegment> lines;
bool drawing;
lineSegment currentLine;

//const int c_dsize = 180;
//	4, 12, 24, 40, 60, 84, 112, 144, 180

/*
Search radius + order:

          13
	   24 5  14
	23 12 1  6  15
 22 11 4  X  2  7  16
	21 10 3  8  17
	   20 9  18
	      19

*/ 
//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------

void readData(void);
XnRGB24Pixel* getTexPixel(int , int );
void glutIdle (void);
void glutDisplay(void);
void glutKeyboard(unsigned char, int, int);
void drawCube(float, float, float, float);
void drawLine(float, float, float, float, float, float);

void drawCube(float x, float y, float z, float len)
{
glBegin(GL_QUADS);
   glNormal3f(0.0f, 0.0f, -1.0f);
   glVertex3f(x-len/2, y+len/2, z-len/2);
   glVertex3f(x+len/2, y+len/2, z-len/2);
   glVertex3f(x+len/2, y-len/2, z-len/2);
   glVertex3f(x-len/2, y-len/2, z-len/2);

   glNormal3f(0.0f, 0.0f, 1.0f);
   glVertex3f(x-len/2, y+len/2, z+len/2);
   glVertex3f(x+len/2, y+len/2, z+len/2);
   glVertex3f(x+len/2, y-len/2, z+len/2);
   glVertex3f(x-len/2, y-len/2, z+len/2);

   glNormal3f(1.0f, 0.0f, 0.0f);
   glVertex3f(x+len/2, y+len/2, z-len/2);
   glVertex3f(x+len/2, y+len/2, z+len/2);
   glVertex3f(x+len/2, y-len/2, z+len/2);
   glVertex3f(x+len/2, y-len/2, z-len/2);

   glNormal3f(-1.0f, 0.0f, 0.0f);
   glVertex3f(x-len/2, y+len/2, z-len/2);
   glVertex3f(x-len/2, y+len/2, z+len/2);
   glVertex3f(x-len/2, y-len/2, z+len/2);
   glVertex3f(x-len/2, y-len/2, z-len/2);

   glNormal3f(0.0f, 1.0f, 0.0f);
   glVertex3f(x-len/2, y+len/2, z-len/2);
   glVertex3f(x-len/2, y+len/2, z+len/2);
   glVertex3f(x+len/2, y+len/2, z+len/2);
   glVertex3f(x+len/2, y+len/2, z-len/2);

   glNormal3f(0.0f, -1.0f, 0.0f);
   glVertex3f(x-len/2, y-len/2, z-len/2);
   glVertex3f(x-len/2, y-len/2, z+len/2);
   glVertex3f(x+len/2, y-len/2, z+len/2);
   glVertex3f(x+len/2, y-len/2, z-len/2);
glEnd();
}

void drawLine(float x, float y, float z, float xx, float yy, float zz)
{
float dx = xx-x;
float dy = yy-y;
float dz = zz-z;
/*
x  y  z
x1 y1 z1
i  j  k
*/
glBegin(GL_QUADS);
	//glNormal3f(0, dz*SMALL_SIDE, -dy*SMALL_SIDE);
	glNormal3f(0, 0, 1);
	glVertex3f(x, y, z);
	glVertex3f(x+SMALL_SIDE, y, z);
	glVertex3f(xx+SMALL_SIDE, yy, zz);
	glVertex3f(xx, yy, zz);

	glNormal3f(0, 0, -1);
	glVertex3f(x, y+SMALL_SIDE, z);
	glVertex3f(x+SMALL_SIDE, y+SMALL_SIDE, z);
	glVertex3f(xx+SMALL_SIDE, yy+SMALL_SIDE, zz);
	glVertex3f(xx, yy+SMALL_SIDE, zz);
	
	//glNormal3f(-dz*SMALL_SIDE, 0, dx*SMALL_SIDE);
	glNormal3f(-1, 0, 0);
	glVertex3f(x, y, z);
	glVertex3f(x, y+SMALL_SIDE, z);
	glVertex3f(xx, yy+SMALL_SIDE, zz);
	glVertex3f(xx, yy, zz);

	glNormal3f(1, 0, 0);
	glVertex3f(x+SMALL_SIDE, y, z);
	glVertex3f(x+SMALL_SIDE, y+SMALL_SIDE, z);
	glVertex3f(xx+SMALL_SIDE, yy+SMALL_SIDE, zz);
	glVertex3f(xx+SMALL_SIDE, yy, zz);

	glNormal3f(0, 1, 0);
	glVertex3f(x, y, z);
	glVertex3f(x, y, z+SMALL_SIDE);
	glVertex3f(xx, yy, zz+SMALL_SIDE);
	glVertex3f(xx, yy, zz);

	glNormal3f(0, -1, 0);
	glVertex3f(x+SMALL_SIDE, y, z);
	glVertex3f(x+SMALL_SIDE, y, z+SMALL_SIDE);
	glVertex3f(xx+SMALL_SIDE, yy, zz+SMALL_SIDE);
	glVertex3f(xx+SMALL_SIDE, yy, zz);

	//glNormal3f(0, -1, 0);
	//glVertex3f(x, y+SMALL_SIDE, z);
	//glVertex3f(x, y+SMALL_SIDE, z+SMALL_SIDE);
	//glVertex3f(xx, yy+SMALL_SIDE, zz+SMALL_SIDE);
	//glVertex3f(xx, yy+SMALL_SIDE, zz);
glEnd();
}

void readData(void)
{
	XnStatus rc = XN_STATUS_OK;

	// Read a new frame
	rc = g_context.WaitAnyUpdateAll();
	if (rc != XN_STATUS_OK)
	{
		printf("Read failed: %s\n", xnGetStatusString(rc));
		return;
	}

	xnFPSMarkFrame(&g_xnFPS);
	printf("FPS: %f\n", xnFPSCalc(&g_xnFPS));

	g_depth.GetMetaData(g_depthMD);
	//g_image.GetMetaData(g_imageMD);

	const XnDepthPixel* pDepth = g_depthMD.Data();
	//const XnUInt8* pImage = g_imageMD.Data();

	for (XnUInt y = 0; y < g_depthMD.YRes(); ++y)
	{
		for (XnUInt x = 0; x < g_depthMD.XRes(); ++x, ++pDepth)
		{
			g_depthFrame[y*g_depthMD.XRes()+x] = *pDepth;
		}
	}
}

XnRGB24Pixel* getTexPixel(int i, int j)
{
	return g_pTexMap + g_depthMD.YOffset()*g_nTexMapX + g_depthMD.XOffset() + g_nTexMapX*(j) + (i);
}

void glutIdle (void)
{
	// Display the frame
	glutPostRedisplay();
}

void glutDisplay (void)
{
	readData(); //pulls depth data using OpenNI; updates g_depthFrame

	FingerFrame frame = g_tracker.analyzeDepthArray(g_depthFrame, g_depthMD.XRes(), g_depthMD.YRes());
	FingerList fingers = frame.fingers;
	
	printf("Avg X change in position %d\n", frame.deltaPos.x);
	
	if(!DRAW_MODE)
	{
		unsigned int nImageScale = GL_WIN_SIZE_X / g_depthMD.FullXRes();

	 //Copied from SimpleViewer
	 //Clear the OpenGL buffers
		glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Setup the OpenGL viewpoint
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glOrtho(0, GL_WIN_SIZE_X, GL_WIN_SIZE_Y, 0, -1.0, 1.0);

		xnOSMemSet(g_pTexMap, 0, g_nTexMapX*g_nTexMapY*sizeof(XnRGB24Pixel));
		for(XnUInt y = 0; y < g_depthMD.YRes(); y++)
		{
			for(XnUInt x = 0; x < g_depthMD.XRes(); x++)
			{
				if(g_depthFrame[y*g_depthMD.XRes()+x] > 500 &&
					g_depthFrame[y*g_depthMD.XRes()+x] < 800)
				{
					XnRGB24Pixel* pTex = getTexPixel(x, y);
					pTex->nBlue = 0;
					pTex->nGreen = 125;
					pTex->nRed = 125;
				}
			}
		}
	
		const XnDepthPixel* pDepthRow = g_depthMD.Data();
		XnRGB24Pixel* pTexRow = g_pTexMap + g_depthMD.YOffset() * g_nTexMapX;
		for (XnUInt y = 0; y < g_depthMD.YRes(); ++y)
		{
			const XnDepthPixel* pDepth = pDepthRow;
			XnRGB24Pixel* pTex = pTexRow + g_depthMD.XOffset();
			for (XnUInt x = 0; x < g_depthMD.XRes(); ++x, ++pDepth, ++pTex)
			{
				if (*pDepth != 0)
				if(*pDepth < DEPTH_THRESHOLD_MAX
					&& *pDepth > DEPTH_THRESHOLD_MIN)
				{
					pTex->nBlue = 0;
					pTex->nGreen = 125;
					pTex->nRed = 125;
				}
			}
			pDepthRow += g_depthMD.XRes();
			pTexRow += g_nTexMapX;
		}

		for(int i = 0; i < fingers.size(); i++)
		{
			FingerVector point = fingers[i];
			for(int yy = (point.py)-2; yy <= (point.py)+2; yy++)
				for(int xx = (point.px)-2; xx <= (point.px)+2; xx++)
				{
					XnRGB24Pixel* pTex = getTexPixel(xx, yy);
					pTex->nBlue = 0;
					pTex->nGreen = 0;
					pTex->nRed = 255;
				}
			//printf("Real space coordinates: %d %d %d\n", point.x, point.y, point.z);
		}
		
		for(int i = 0; i < frame.curve.size(); i++)
		{
			FingerVector point = frame.curve[i];
			for(int yy = (point.py)-0; yy <= (point.py)+0; yy++)
				for(int xx = (point.px)-0; xx <= (point.px)+0; xx++)
				{
					XnRGB24Pixel* pTex = getTexPixel(xx, yy);
					pTex->nBlue = 0;
					pTex->nGreen = 255;
					pTex->nRed = 0;
				}
		}
		
		for(int i = 0; i < frame.vertices.size(); i++)
		{
			FingerVector point = frame.vertices[i];
			for(int yy = (point.py)-2; yy <= (point.py)+2; yy++)
				for(int xx = (point.px)-2; xx <= (point.px)+2; xx++)
				{
					XnRGB24Pixel* pTex = getTexPixel(xx, yy);
					pTex->nBlue = 255;
					pTex->nGreen = 0;
					pTex->nRed = 0;
				}
			//printf("Real space coordinates: %d %d %d\n", point.x, point.y, point.z);
		}

		//printf("Avg Pos: %d %d %d\n", frame.avgPos.x, frame.avgPos.y, frame.avgPos.z);
		//printf("Avg Z change in position %d\n", frame.deltaPos.z);
		//printf("Avg Z \t\tvelocity %d\n", frame.avgVelocity.z);
		//printf("Avg X change in position %d\n", frame.deltaPos.x);
		//
	
		// Create the OpenGL texture map
		//glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAPS_SGI, GL_TRUE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, g_nTexMapX, g_nTexMapY, 0, GL_RGB, GL_UNSIGNED_BYTE, g_pTexMap);
	
		// Display the OpenGL texture map
		glColor4f(1,1,1,1);
		glBegin(GL_QUADS);
		int nXRes = g_depthMD.FullXRes();
		int nYRes = g_depthMD.FullYRes();

		// upper left
		glTexCoord2f(0, 0);
		glVertex2f(0, 0);
		// upper right
		glTexCoord2f((float)nXRes/(float)g_nTexMapX, 0);
		glVertex2f(GL_WIN_SIZE_X, 0);
		// bottom right
		glTexCoord2f((float)nXRes/(float)g_nTexMapX, (float)nYRes/(float)g_nTexMapY);
		glVertex2f(GL_WIN_SIZE_X, GL_WIN_SIZE_Y);
		// bottom left
		glTexCoord2f(0, (float)nYRes/(float)g_nTexMapY);
		glVertex2f(0, GL_WIN_SIZE_Y);

		glEnd();
	}

	

	if(DRAW_MODE)
	{
		//1 finger -- cursor
		//2 fingers -- draw
		//3 fingers -- camera
		if(fingers.size() == 1)
		{
			if(drawing)
			{
				drawing = false;
				currentLine.x2 = sx;
				currentLine.y2 = sy;
				currentLine.z2 = sz;
				lines.push_back(currentLine);
			}
			if(tracking)
			{
				if(abs(frame.deltaPos.x) < TOO_BIG || abs(frame.deltaPos.y) < TOO_BIG || abs(frame.deltaPos.z) < TOO_BIG)
				{
					sx += frame.deltaPos.x * SCALE_FACTOR * cos(anglex*DEG_TO_RAD);
					sx += frame.deltaPos.z * SCALE_FACTOR * sin(anglex*DEG_TO_RAD);
					sy += frame.deltaPos.y * SCALE_FACTOR;
					sz += frame.deltaPos.z * SCALE_FACTOR * cos(anglex*DEG_TO_RAD);
					sz -= frame.deltaPos.x * SCALE_FACTOR * sin(anglex*DEG_TO_RAD);
				}
			}
			else
			{
				tracking = true;
			}
		}
		if(fingers.size() == 2)
		{
			if(!drawing)
			{
				drawing = true;
				currentLine.x1 = sx;
				currentLine.y1 = sy;
				currentLine.z1 = sz;
			}
			if(tracking)
			{
				if(abs(frame.deltaPos.x) < TOO_BIG || abs(frame.deltaPos.y) < TOO_BIG || abs(frame.deltaPos.z) < TOO_BIG)
				{
					sx += frame.deltaPos.x * SCALE_FACTOR * cos(anglex*DEG_TO_RAD);
					sx += frame.deltaPos.z * SCALE_FACTOR * sin(anglex*DEG_TO_RAD);
					sy += frame.deltaPos.y * SCALE_FACTOR;
					sz += frame.deltaPos.z * SCALE_FACTOR * cos(anglex*DEG_TO_RAD);
					sz -= frame.deltaPos.x * SCALE_FACTOR * sin(anglex*DEG_TO_RAD);
				}
			}
			else
			{
				tracking = true;
			}
		}
		if(fingers.size() > 2)
		{
			if(tracking)
			{
				if(abs(frame.deltaPos.x) < TOO_BIG || abs(frame.deltaPos.y) < TOO_BIG || abs(frame.deltaPos.z) < TOO_BIG)
				{
				anglex += frame.deltaPos.x * ANGLE_SCALE_FACTOR;
				}
			}
			else
			{
				tracking = true;
			}
		}
		if(fingers.size() == 0)
		{
			tracking = false;
		}
	

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		     // Clear Screen and Depth Buffer
		glLoadIdentity();
		glTranslatef(0.0f,0.0f,-3.0f);
		glRotatef(-anglex, 0, 1, 0);
		glRotatef(-angley, 1, 0, 0);
		GLfloat lightpos[] = {0.0f, 0.0f, 0.0f, 1.0f};
		GLfloat cyan[] = {0.f, .8f, .8f, 1.f};
		GLfloat red[] = {1.f, 0.f, 0.f, 1.f};
		GLfloat green[] = {0.f, 0.8f, 0.f, 1.f};
		glLightfv(GL_LIGHT0, GL_POSITION, lightpos);
		glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, green);
		drawCube(0.0f, 0.0f, 0.0f, 0.1f);
		glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, red);
		drawCube(sx, sy, sz, 0.1);
		glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, cyan);
		for(std::vector<lineSegment>::iterator it = lines.begin(); it != lines.end(); it++)
		{
			drawLine(it->x1, it->y1, it->z1, it->x2, it->y2, it->z2);
		}

		// Swap the OpenGL display buffers
	
	}
	glutSwapBuffers();
}

void glutKeyboard (unsigned char key, int x, int y)
{
  switch ( key ) 
  {
	    case 27:        
          exit ( 0 );   
          break;  
		case 'r':
			sx = 0;
			sy = 0;
			sz = 0;
			lines.clear();
			drawing = false;
		case 'z':
			sx -= 0.1*cos(anglex*DEG_TO_RAD);
			sz += 0.1*sin(anglex*DEG_TO_RAD);
			break;
		case 'x':
			sy -= 0.1;
			break;
		case 'c':
			sx += 0.1*cos(anglex*DEG_TO_RAD);
			sz -= 0.1*sin(anglex*DEG_TO_RAD);
			break;
		case 's':
			sy += 0.1;
			break;
		case 'f':
			sx -= 0.1*sin(anglex*DEG_TO_RAD);
			sz -= 0.1*cos(anglex*DEG_TO_RAD);
			break;
		case 'v':
			sx += 0.1*sin(anglex*DEG_TO_RAD);
			sz += 0.1*cos(anglex*DEG_TO_RAD);
			break;
		case 'j':
			anglex += -5;
			break;
		case 'k':
			anglex += 5;
			break;
		case 'i':
			angley += -5;
			break;
		case 'm':
			angley += 5;
			break;
		case VK_RETURN:
			if(!drawing)
			{
				currentLine.x1 = sx;
				currentLine.y1 = sy;
				currentLine.z1 = sz;
			}
			else
			{
				currentLine.x2 = sx;
				currentLine.y2 = sy;
				currentLine.z2 = sz;
				lines.push_back(currentLine);
			}
			drawing = !drawing;
			break;
    default:      
      break;
  }
}

//int notmain(int argc, char* argv[])
int main(int argc, char* argv[])
{
	//Kinect OpenNI init
	
	XnStatus rc = XN_STATUS_OK;

	rc = g_context.Init();
	CHECK_RC(rc, "Initialize context");

	DepthGenerator depth;
	rc = g_depth.Create(g_context);
	CHECK_RC(rc, "Create depth generator");

	rc = g_context.StartGeneratingAll();
	CHECK_RC(rc, "StartGeneratingAll");

    xnFPSInit(&g_xnFPS, 180);
	
	g_context.SetGlobalMirror(true);
	g_depth.GetMetaData(g_depthMD);
	
	// Texture map init
	if(!DRAW_MODE)
	{
		//g_image.GetMetaData(g_imageMD);
		g_nTexMapX = (((unsigned short)(g_depthMD.FullXRes()-1) / 512) + 1) * 512;
		g_nTexMapY = (((unsigned short)(g_depthMD.FullYRes()-1) / 512) + 1) * 512;
		g_pTexMap = (XnRGB24Pixel*)malloc(g_nTexMapX * g_nTexMapY * sizeof(XnRGB24Pixel));
	}

	// OpenGL init
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(GL_WIN_SIZE_X, GL_WIN_SIZE_Y);
	glutCreateWindow ("Kinect Finger Tracking");
	//glutFullScreen();
	//glutSetCursor(GLUT_CURSOR_NONE);

	glutDisplayFunc(glutDisplay);
	glutIdleFunc(glutIdle);
	glutKeyboardFunc(glutKeyboard);

    if(DRAW_MODE)
	{
		glMatrixMode(GL_PROJECTION);												// select projection matrix
		glViewport(0, 0, GL_WIN_SIZE_X, GL_WIN_SIZE_Y);									// set the viewport
		glMatrixMode(GL_PROJECTION);												// set matrix mode
		glLoadIdentity();															// reset projection matrix
		GLfloat aspect = (GLfloat) GL_WIN_SIZE_X / GL_WIN_SIZE_Y;
		gluPerspective(45.0, aspect, 1.0, 500.0);		// set up a perspective projection matrix
		glMatrixMode(GL_MODELVIEW);													// specify which matrix is the current matrix
		glShadeModel( GL_SMOOTH );
		glClearDepth( 1.0f );							// specify the clear value for the depth buffer
		glEnable( GL_DEPTH_TEST );
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
		glDepthFunc( GL_LEQUAL );
	    glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );						// specify implementation-specific hints
		glClearColor(0.0, 0.0, 0.0, 1.0);											// specify clear values for the color buffers
	}
	if(!DRAW_MODE)
	{
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_TEXTURE_2D);
	}
	
	// Per frame code is in glutDisplay
	glutMainLoop();

	return 0;
}