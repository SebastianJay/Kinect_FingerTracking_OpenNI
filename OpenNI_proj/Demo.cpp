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

#define GL_WIN_SIZE_X 640
#define GL_WIN_SIZE_Y 480

#define DISPLAY_MODE_OVERLAY	1
#define DISPLAY_MODE_DEPTH		2
#define DISPLAY_MODE_IMAGE		3
#define DEFAULT_DISPLAY_MODE	DISPLAY_MODE_DEPTH

#define MAX_DEPTH 10000
#define DEPTH_THRESHOLD_MAX 750
#define DEPTH_THRESHOLD_MIN 500
#define THETA_THRESHOLD 30
#define NOISE_TOLERANCE 10
#define MAX_BODIES 100
#define CURVE_INC 5
#define PI 3.14159265

#define X_RES 640
#define Y_RES 480

#define PIXEL_AREA 1
#define ADVANCE_LEN 50
#define K_CURVE 30
#define MAX_FINGERS 50

#define CHECK_RC(rc, what)											\
	if (rc != XN_STATUS_OK)											\
	{																\
		printf("%s failed: %s\n", what, xnGetStatusString(rc));		\
		return rc;													\
	}

//class HandContourPixel{
//public:
//	int x, y, depth;
//	HandContourPixel() {};
//	HandContourPixel(int a, int b, int c)
//	{ x = a; y = b; depth = c; }
//};

//class CurvePixel{
//public:
//	int x, y, id;
//	CurvePixel() {};
//	CurvePixel(int a, int b, int c)
//	{ x=a; y=b; id=c; }
//};

typedef std::pair<int, int> pii;

float dist(int a, int b);
void swap(int a, int b);
float angleC(int a, int b, int c);

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

XnFPSData g_xnFPS;
unsigned int g_depthFrame[Y_RES][X_RES]; //correct for actual height and width
unsigned int g_handPixelFrame[Y_RES*X_RES];
int g_xHist[X_RES];
int g_yHist[Y_RES];

//std::set<pii> g_handPixelFrame;
//CurvePixel g_sortedHandPixelFrame[Y_RES*X_RES];
std::vector<int> g_pixelCurve;
short segBuff[X_RES*Y_RES];
const int c_dx[8] = { 0, 1, 0,-1,-1, 1, 1,-1};
const int c_dy[8] = { 1, 0,-1, 0, 1, 1,-1,-1};
const int c_dsize = 8;

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------


float getAngle(int i, int j, int k, int w, int h) {
  int x1 = i % w;
  int y1 = i / w;
  int x2 = j % w;
  int y2 = j / w;
  int x3 = k % w;
  int y3 = k / w;
  float d12 = (x2-x1)*(x2-x1) + (y2-y1)*(y2-y1);
  float d13 = (x3-x1)*(x3-x1) + (y3-y1)*(y3-y1);
  float d23 = (x2-x3)*(x2-x3) + (y2-y3)*(y2-y3);
  float theta = acos((d13-d12-d23)/(-2*sqrt(d12)*sqrt(d23)));
  return theta * 180 / 3.14159265;
}

int getCentroid(int i, int j, int k, int w, int h) {
  int x1 = i % w;
  int y1 = i / w;
  int x2 = j % w;
  int y2 = j / w;
  int x3 = k % w;
  int y3 = k / w;
  int x = (x1+x2+x3)/3;
  int y = (y1+y2+y3)/3;
  return y * w + x;
}

XnRGB24Pixel* getTexPixel(int i, int j)
{
	return g_pTexMap + g_depthMD.YOffset()*g_nTexMapX + g_depthMD.XOffset() + g_nTexMapX*(i) + (j);
}

void glutIdle (void)
{
	// Display the frame
	glutPostRedisplay();
}

void glutDisplay (void)
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

	unsigned int nImageScale = GL_WIN_SIZE_X / g_depthMD.FullXRes();

	// Copied from SimpleViewer
	// Clear the OpenGL buffers
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Setup the OpenGL viewpoint
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, GL_WIN_SIZE_X, GL_WIN_SIZE_Y, 0, -1.0, 1.0);

	// Calculate the accumulative histogram (the yellow display...)
	xnOSMemSet(g_pDepthHist, 0, MAX_DEPTH*sizeof(float));
	xnOSMemSet(g_depthFrame, 0, X_RES*Y_RES*sizeof(unsigned int));
	xnOSMemSet(g_handPixelFrame, 0, X_RES*Y_RES*sizeof(unsigned int));
	xnOSMemSet(segBuff, 0, X_RES*Y_RES*sizeof(short));
	g_pixelCurve.clear();

	unsigned int nNumberOfPoints = 0;
	for (XnUInt y = 0; y < g_depthMD.YRes(); ++y)
	{
		for (XnUInt x = 0; x < g_depthMD.XRes(); ++x, ++pDepth)
		{
			if (*pDepth != 0)
			{
				g_pDepthHist[*pDepth]++;
				nNumberOfPoints++;
			}
			g_depthFrame[y][x] = *pDepth;
		}
	}

	for (int nIndex=1; nIndex<MAX_DEPTH; nIndex++)
	{
		g_pDepthHist[nIndex] += g_pDepthHist[nIndex-1];
	}
	if (nNumberOfPoints)
	{
		for (int nIndex=1; nIndex<MAX_DEPTH; nIndex++)
		{
			g_pDepthHist[nIndex] = (unsigned int)(256 * (1.0f - (g_pDepthHist[nIndex] / nNumberOfPoints)));
		}
	}

	xnOSMemSet(g_pTexMap, 0, g_nTexMapX*g_nTexMapY*sizeof(XnRGB24Pixel));
	const XnDepthPixel* pDepthRow = g_depthMD.Data();
	XnRGB24Pixel* pTexRow = g_pTexMap + g_depthMD.YOffset() * g_nTexMapX;

	int handPixCount = 0;
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
				if(x==0||x==g_depthMD.XRes()-1
					||y==0||y==g_depthMD.YRes()-1) continue;
				if(abs((int)(g_depthFrame[y+1][x] - g_depthFrame[y][x])) < NOISE_TOLERANCE &&
					abs((int)(g_depthFrame[y-1][x] - g_depthFrame[y][x])) < NOISE_TOLERANCE &&
					abs((int)(g_depthFrame[y][x-1] - g_depthFrame[y][x])) < NOISE_TOLERANCE &&
					abs((int)(g_depthFrame[y][x+1] - g_depthFrame[y][x])) < NOISE_TOLERANCE) continue;
				//do stuff:
					//perhaps somehow normalize the data?
					//given depth, width + height in pixels
					//Depth to other dimensions may be proportional
					//

					//insert points into array
					//sort by point's proximity to other points (hopefully resulting in a continuous curve) -- O(N^2) algorithm
					//points far away are considered separate bodies (get their own list)
					//evaluate the angles between points along the curve (very acute angles are fingers)
				g_handPixelFrame[y*X_RES+x] = *pDepth;
				int nHistValue = g_pDepthHist[*pDepth];
				pTex->nRed = nHistValue;
				pTex->nGreen = nHistValue;
				pTex->nBlue = 0;
				handPixCount++;
			}
		}

		pDepthRow += g_depthMD.XRes();
		pTexRow += g_nTexMapX;
	}
	printf("Number of edge pixels: %d\n", handPixCount);

	//begin sort
	//std::set<pii>::iterator it = g_handPixelFrame.begin();
	//int pixelIndex = 0, bodyIndex = 0;
	//int currX = it->first;
	//int currY = it->second;
	//CurvePixel pix(currX, currY, bodyIndex);
	//g_sortedHandPixelFrame[pixelIndex++] = pix;
	//g_handPixelFrame.erase(it);
	//while(!g_handPixelFrame.empty())
	//{
	//	//stuff
	//	bool foundNeighbor=false;
	//	for(int dirX=-10; dirX<=10; dirX++)
	//	{
	//		for(int dirY=-10; dirY<=10; dirY++)
	//		{
	//			if(dirX==0 && dirY==0) continue;
	//			it = g_handPixelFrame.find(pii(currX + dirX, currY + dirY));
	//			if(it != g_handPixelFrame.end())
	//			{
	//				CurvePixel pix(currX+dirX, currY+dirY, bodyIndex);
	//				g_sortedHandPixelFrame[pixelIndex++] = pix;
	//				g_handPixelFrame.erase(it);
	//				foundNeighbor = true;
	//				currX += dirX; currY += dirY; 
	//				break;
	//			}
	//		}
	//		if(foundNeighbor) break;
	//	}
	//	if(!foundNeighbor)
	//	{
	//		it = g_handPixelFrame.begin();
	//		currX = it->first;
	//		currY = it->second;
	//		CurvePixel pix(currX, currY, bodyIndex++);
	//		g_sortedHandPixelFrame[pixelIndex++] = pix;
	//		g_handPixelFrame.erase(it);
	//	}
	//}
	//now it should be sorted.. hopefully quickly
	

	//BEGIN ANALYSIS
	int arraySize = 1;
	for(int i=0; i<X_RES*Y_RES; i++) //we search for one pixel, then expand to fill the curve
	{
		if(g_handPixelFrame[i]==0) continue;
		g_pixelCurve.push_back(i);
		segBuff[i] = 1;
		for(int dir=0; dir < 2; dir++) //the first pixel gets the opportunity to expand 3 ways
		{
			int curr = i;
			while(true)
			{
				int r = curr / X_RES; //x
				int c = curr % X_RES; //y
				bool done = true;
				for(int c_dstep = 0; c_dstep < c_dsize; c_dstep++)
				{
					int rr = r + c_dx[c_dstep];
					int cc = c + c_dy[c_dstep];
					if(rr==r && cc==c || rr<0 || rr>= Y_RES || cc < 0 || cc >= X_RES) continue;
					int ii = rr*X_RES+cc;
					if(ii < 0 || ii >= X_RES*Y_RES || segBuff[ii]==1 || g_handPixelFrame[ii]==0) continue;
					
					if(dir==0)
						g_pixelCurve.push_back(ii);
					else
						g_pixelCurve.insert(g_pixelCurve.begin(), ii);
					//
					XnRGB24Pixel* pTex = getTexPixel(ii/X_RES, ii%X_RES);
					pTex->nBlue=0;
					pTex->nGreen=255;
					pTex->nRed=0;
					//
					arraySize++;
					segBuff[ii] = 1;
					curr = ii;
					done = false; break;
				}
				if(done) break;
			}
		}
		break;
	}

	printf("Number of pixels in contour array: %d\n", arraySize);
	int extContour[50];
	std::vector<int>::iterator it = g_pixelCurve.begin();
	for(int counter=0; it != g_pixelCurve.end() && counter < 50; ++it, ++counter)
	{
		extContour[counter] = *it;
	}
	for(int counter=0; counter < 50; counter++)
	{
		g_pixelCurve.push_back(extContour[counter]);
	} //we append the first 50 pixels in the array to the end to allow for some wrap-around
	
	int tipNum=0;
	it = g_pixelCurve.begin();
	std::vector<int>::iterator it1 = it;
	it1 += K_CURVE;
	std::vector<int>::iterator it2 = it1;
	it2 += K_CURVE;
	//for(std::vector<int>::iterator it = g_pixelCurve.begin(); it != g_pixelCurve.end(); ++it)
	for(; it != g_pixelCurve.end() && it1 != g_pixelCurve.end() && it2 != g_pixelCurve.end(); ++it, ++it1, ++it2)
	{
		int i = *it;
		//std::vector<int>::iterator it1 = it;
		//for(int counter = 0; it1 != g_pixelCurve.end() && counter < K_CURVE; ++it1, ++counter);
		//if(it1 != g_pixelCurve.end())
		//{
		int j = *it1;
			//std::vector<int>::iterator it2 = it1;
			//for(int counter = 0; it2 != g_pixelCurve.end() && counter < K_CURVE; ++it2, ++counter);
			//if(it2 != g_pixelCurve.end())
			//{
		int k = *it2;
		float angle = getAngle(i, j, k, X_RES, Y_RES);
		if(angle < THETA_THRESHOLD) 
		{
			int center = getCentroid(i, j, k, X_RES, Y_RES);
			if(g_depthFrame[center/X_RES][center%X_RES] > DEPTH_THRESHOLD_MIN && 
				g_depthFrame[center/X_RES][center%X_RES] < DEPTH_THRESHOLD_MAX)
			{
				if(tipNum < MAX_FINGERS)
				{
					printf("Found finger at %d, %d\n", center/X_RES, center%X_RES);
					//XnRGB24Pixel* pTex = g_pTexMap + g_depthMD.YOffset()*g_nTexMapX + g_depthMD.XOffset() + g_nTexMapX*(center/X_RES) + (center%X_RES);
					XnRGB24Pixel* pTex = getTexPixel(center/X_RES, center%X_RES);
					pTex->nBlue = 0;
					pTex->nGreen = 0;
					pTex->nRed = 255;
					tipNum++;
				}
				for (int counter = 0; it != g_pixelCurve.end() && it1 != g_pixelCurve.end() && it2 != g_pixelCurve.end()
					&& counter < ADVANCE_LEN; ++it, ++it1, ++it2, ++counter);
			}
		}
			//}
		//}
	}
	//END ANALYSIS
	//printf("Number of detections: %d\n", tipNum);
	//printf("Num bodies: %d\n", bodyIndex);
	printf("\n");

	////begin analysis OLD
	//for(int i = CURVE_INC*2; i < pixelIndex; i+=CURVE_INC)
	//{
	//	int a = i-CURVE_INC*2;
	//	int b = i-CURVE_INC;
	//	int c = i;
	//	if(g_sortedHandPixelFrame[a].id != g_sortedHandPixelFrame[b].id
	//		|| g_sortedHandPixelFrame[b].id != g_sortedHandPixelFrame[c].id)
	//			continue;

	//	float dab = (g_sortedHandPixelFrame[b].x-g_sortedHandPixelFrame[a].x)
	//			   *(g_sortedHandPixelFrame[b].x-g_sortedHandPixelFrame[a].x)
	//			   +(g_sortedHandPixelFrame[b].y-g_sortedHandPixelFrame[a].y)
	//			   *(g_sortedHandPixelFrame[b].y-g_sortedHandPixelFrame[a].y);
	//	float dbc = (g_sortedHandPixelFrame[c].x-g_sortedHandPixelFrame[b].x)
	//			   *(g_sortedHandPixelFrame[c].x-g_sortedHandPixelFrame[b].x)
	//			   +(g_sortedHandPixelFrame[c].y-g_sortedHandPixelFrame[b].y)
	//			   *(g_sortedHandPixelFrame[c].y-g_sortedHandPixelFrame[b].y);
	//	float dac = (g_sortedHandPixelFrame[c].x-g_sortedHandPixelFrame[a].x)
	//			   *(g_sortedHandPixelFrame[c].x-g_sortedHandPixelFrame[a].x)
	//			   +(g_sortedHandPixelFrame[c].y-g_sortedHandPixelFrame[a].y)
	//			   *(g_sortedHandPixelFrame[c].y-g_sortedHandPixelFrame[a].y);
	//	float theta = acos((dac-dab-dbc)/(-2*sqrt(dab)*sqrt(dbc))) * 180 / PI;
	//	printf("Theta: %f\n", theta);
	//	if(theta < THETA_THRESHOLD)
	//	{
	//		int mid_x = (g_sortedHandPixelFrame[a].x + 
	//					 g_sortedHandPixelFrame[b].x +
	//					 g_sortedHandPixelFrame[c].x) / 3;
	//		int mid_y = (g_sortedHandPixelFrame[a].y + 
	//					 g_sortedHandPixelFrame[b].y +
	//					 g_sortedHandPixelFrame[c].y) / 3;
	//		if(g_depthFrame[mid_y][mid_x] > DEPTH_THRESHOLD_MIN
	//			&& g_depthFrame[mid_y][mid_x] < DEPTH_THRESHOLD_MAX)
	//		{
	//			printf("Found finger at %d %d\n", mid_x, mid_y);
	//			XnRGB24Pixel* pTex = g_pTexMap + g_depthMD.YOffset() * g_nTexMapX;
	//			pTex += g_nTexMapX * mid_y;
	//			pTex += g_depthMD.XOffset();
	//			pTex += mid_x;
	//			pTex->nGreen = 255;
	//		}
	//	}
	//}
	
	//end analysis

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

	// Swap the OpenGL display buffers
	glutSwapBuffers();
}

void glutKeyboard (unsigned char key, int x, int y)
{
	switch (key)
	{
		case 27:
			g_depth.Release();
			g_context.Release();
			exit (1);
		case '2':
			g_nViewState = DISPLAY_MODE_DEPTH;
			g_depth.GetAlternativeViewPointCap().ResetViewPoint();
			break;
		case 'm':
			g_context.SetGlobalMirror(!g_context.GetGlobalMirror());
			break;
	}
}

int main(int argc, char* argv[])
{
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
	g_nTexMapX = (((unsigned short)(g_depthMD.FullXRes()-1) / 512) + 1) * 512;
	g_nTexMapY = (((unsigned short)(g_depthMD.FullYRes()-1) / 512) + 1) * 512;
	g_pTexMap = (XnRGB24Pixel*)malloc(g_nTexMapX * g_nTexMapY * sizeof(XnRGB24Pixel));

	// OpenGL init
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(GL_WIN_SIZE_X, GL_WIN_SIZE_Y);
	glutCreateWindow ("OpenNI Simple Viewer");
	//glutFullScreen();
	glutSetCursor(GLUT_CURSOR_NONE);

	glutKeyboardFunc(glutKeyboard);
	glutDisplayFunc(glutDisplay);
	glutIdleFunc(glutIdle);

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);

	// Per frame code is in glutDisplay
	glutMainLoop();

	return 0;
}