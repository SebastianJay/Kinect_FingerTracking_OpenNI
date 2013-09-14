#include "FingerTracker.h"
#include "FingerVector.h"
#include "FingerList.h"
#include "FingerFrame.h"
#include <vector>

const int FingerTracker::c_dx[] = {   
	 0, 1, 0,-1
	,0, 1, 2, 1, 0,-1,-2,-1
	,0, 1, 2, 3, 2, 1, 0,-1,-2,-3,-2,-1
	,0, 1, 2, 3, 4, 3, 2, 1, 0,-1,-2,-3,-4,-3,-2,-1
	,0, 1, 2, 3, 4, 5, 4, 3, 2, 1, 0,-1,-2,-3,-4,-5,-4,-3,-2,-1
	,0, 1, 2, 3, 4, 5, 6, 5, 4, 3, 2, 1, 0,-1,-2,-3,-4,-5,-6,-5,-4,-3,-2,-1
	,0, 1, 2, 3, 4, 5, 6, 7, 6, 5, 4, 3, 2, 1, 0,-1,-2,-3,-4,-5,-6,-7,-6,-5,-4,-3,-2,-1
	,0, 1, 2, 3, 4, 5, 6, 7, 8, 7, 6, 5, 4, 3, 2, 1, 0,-1,-2,-3,-4,-5,-6,-7,-8,-7,-6,-5,-4,-3,-2,-1
	,0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,-1,-2,-3,-4,-5,-6,-7,-8,-9,-8,-7,-6,-5,-4,-3,-2,-1};

const int FingerTracker::c_dy[] = {   
	 1, 0,-1, 0
	,2, 1, 0,-1,-2,-1, 0, 1
	,3, 2, 1, 0,-1,-2,-3,-2,-1, 0, 1, 2
	,4, 3, 2, 1, 0,-1,-2,-3,-4,-3,-2,-1, 0, 1, 2, 3
	,5, 4, 3, 2, 1, 0,-1,-2,-3,-4,-5,-4,-3,-2,-1, 0, 1, 2, 3, 4
	,6, 5, 4, 3, 2, 1, 0,-1,-2,-3,-4,-5,-6,-5,-4,-3,-2,-1, 0, 1, 2, 3, 4, 5
	,7, 6, 5, 4, 3, 2, 1, 0,-1,-2,-3,-4,-5,-6,-7,-6,-5,-4,-3,-2,-1, 0, 1, 2, 3, 4, 5, 6
	,8, 7, 6, 5, 4, 3, 2, 1, 0,-1,-2,-3,-4,-5,-6,-7,-8,-7,-6,-5,-4,-3,-2,-1, 0, 1, 2, 3, 4, 5, 6, 7
	,9, 8, 7, 6, 5, 4, 3, 2, 1, 0,-1,-2,-3,-4,-5,-6,-7,-8,-9,-8,-7,-6,-5,-4,-3,-2,-1, 0, 1, 2, 3, 4, 5, 6, 7, 8};

const int FingerTracker::c_radii[] = {
	 0, 4, 12, 24, 40, 60, 84, 112, 144, 180};

bool FingerTracker::setSearchRadius(int r)
{
	if(r < 0 || r > 9)
		return false;
	c_radindex = r;
	return true;
}

bool FingerTracker::setMinDepth(int d)
{
	if(d <= 0)
		return false;
	c_mindepth = d;
	return true;
}

bool FingerTracker::setMaxDepth(int d)
{
	//if(d < MAX_DEPTH)
	//	return false;
	c_maxdepth = d;
	return true;
}

bool FingerTracker::setThetaThreshold(double a)
{
	if(a < 0 || a > 180)
		return false;
	c_thetathresh = a;
	return true;
}

double FingerTracker::getAngle(int i, int j, int k, int w, int h) {
	int x1 = i % w;
	int y1 = i / w;
	int x2 = j % w;
	int y2 = j / w;
	int x3 = k % w;
	int y3 = k / w;
	int d12 = (x2-x1)*(x2-x1) + (y2-y1)*(y2-y1);
	int d13 = (x3-x1)*(x3-x1) + (y3-y1)*(y3-y1);
	int d23 = (x2-x3)*(x2-x3) + (y2-y3)*(y2-y3);
	double theta = acos((d13-d12-d23)/(-2*sqrt(1.0*d12)*sqrt(1.0*d23)));
	return theta * 180 / 3.14159265;
}

int FingerTracker::getCentroid(int i, int j, int k, int w, int h) {
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

FingerFrame FingerTracker::analyzeDepthArray(unsigned int* arr, int XRes, int YRes)
{
	memset(g_handPixelFrame, 0, __FINGER_TRACKER_X_RES__ * __FINGER_TRACKER_Y_RES__*sizeof(unsigned int));
	memset(g_segBuff, 0, __FINGER_TRACKER_X_RES__ * __FINGER_TRACKER_Y_RES__*sizeof(short));
	g_pixelCurve.clear();

	for (int y = 0; y < YRes; ++y)
	{
		for (int x = 0; x < XRes; ++x)
		{
			if (arr[y*XRes+x] != 0)
			if(arr[y*XRes+x]  < c_maxdepth
				&& arr[y*XRes+x] > c_mindepth)
			{
				if(x==0||x==XRes-1
					||y==0||y==YRes-1) continue;

				int nhand = 0;
				if(abs((int)(arr[(y+1)*XRes+x] - arr[y*XRes+x])) < __FINGER_TRACKER_NOISE_TOLERANCE__) nhand++;
				if(abs((int)(arr[(y-1)*XRes+x] - arr[y*XRes+x])) < __FINGER_TRACKER_NOISE_TOLERANCE__) nhand++;
				if(abs((int)(arr[y*XRes+(x-1)] - arr[y*XRes+x])) < __FINGER_TRACKER_NOISE_TOLERANCE__) nhand++;
				if(abs((int)(arr[y*XRes+(x+1)] - arr[y*XRes+x])) < __FINGER_TRACKER_NOISE_TOLERANCE__) nhand++;
				//if(abs((int)(arr[y+1][x+1] - arr[y][x])) < NOISE_TOLERANCE) nhand++;
				//if(abs((int)(arr[y+1][x-1] - arr[y][x])) < NOISE_TOLERANCE) nhand++;
				//if(abs((int)(arr[y-1][x+1] - arr[y][x])) < NOISE_TOLERANCE) nhand++;
				//if(abs((int)(arr[y-1][x-1] - arr[y][x])) < NOISE_TOLERANCE) nhand++;

				if(nhand != 3 && nhand != 2) continue;
				g_handPixelFrame[y*XRes+x] = arr[y*XRes+x];
			}
		}
	}
	
	//BEGIN ANALYSIS
	FingerList curve;
	int arraySize = 0;
	int totalDepth = 0;
	while(arraySize < __FINGER_TRACKER_CURVE_LENGTH_MIN__) //prevent analysis of small bodies which are clearly not hands
	{
		arraySize = 0;
		totalDepth = 0;
		g_pixelCurve.clear();
		
		int minind = -1;
		for(int i=0; i<XRes*YRes; i++) //we search for the pixel closest to the camera (in our defined range)
		{
			if(minind == -1 && g_handPixelFrame[i] != 0 && g_segBuff[i] == 0)
				minind = i; //set initial min
			else if(minind != -1 && g_handPixelFrame[i] != 0 && g_handPixelFrame[i] < g_handPixelFrame[minind] && g_segBuff[i] == 0)
				minind = i;
		}
		if(minind==-1) break;
		//for(int i=0; i<XRes*YRes; i++) //we search for one pixel, then expand to fill the curve
		else
		{
			//if(g_handPixelFrame[i]==0) continue;
			//if(g_segBuff[i]==1) continue;
			//g_pixelCurve.push_back(i);
			//g_segBuff[i] = 1;
			g_pixelCurve.push_back(minind);
			g_segBuff[minind] = 1;
			totalDepth += g_handPixelFrame[minind];
			for(int dir=0; dir < 2; dir++) //the first pixel gets the opportunity to expand 2 ways
			{
				//int curr = i;
				int curr = minind;
				while(true)
				{
					int r = curr / XRes; //x
					int c = curr % XRes; //y
					bool done = true;
					for(int c_dstep = 0; c_dstep < c_radii[c_radindex]; c_dstep++)
					{
						int rr = r + c_dx[c_dstep];
						int cc = c + c_dy[c_dstep];
						if(rr < 0 || rr >= YRes || cc < 0 || cc >= XRes) continue;
						int ii = rr*XRes+cc;
						if(g_segBuff[ii]==1 || g_handPixelFrame[ii]==0) continue;
					
						if(abs(arr[ii]-arr[minind]*1.0) >= __FINGER_TRACKER_DEPTH_VARY_MAX__)
							continue;

						if(dir==0)
							g_pixelCurve.push_back(ii);
						else
							g_pixelCurve.insert(g_pixelCurve.begin(), ii);
						curve.add(FingerVector(ii%XRes, ii/XRes, (int)arr[ii], true));

						arraySize++;
						if(arraySize >= __FINGER_TRACKER_CURVE_LENGTH_MAX__) break;
						totalDepth += g_handPixelFrame[ii];
						g_segBuff[ii] = 1;
						curr = ii;
						done = false; break;
					}
					if(done) { break; }
				}
			}
			//printf("Number of pixels in contour array: %d\n", arraySize);
			//printf("Average depth of whole curve: %f\n", totalDepth*1.0 / arraySize);
			//break;
		}
	}

	int tipNum=0;
	//adjust K according to distance from camera
	double kslope = (__FINGER_TRACKER_K_CURVE_MAX__- __FINGER_TRACKER_K_CURVE_MIN__)*1.0/(__FINGER_TRACKER_K_DEPTH_MAX__ - __FINGER_TRACKER_K_DEPTH_MIN__);
	int kcurv = (int)(__FINGER_TRACKER_K_CURVE_MAX__ - ((totalDepth*1.0/arraySize)-__FINGER_TRACKER_K_DEPTH_MIN__)*kslope);
	if(kcurv < 7) kcurv = 7;
	FingerList fingers;
	FingerList vertices;
	for(std::vector<int>::iterator it = g_pixelCurve.begin(); it != g_pixelCurve.end(); ++it)
	{
		int i = *it;
		std::vector<int>::iterator it1 = it;
		for(int counter = 0; it1 != g_pixelCurve.end() && counter < kcurv; ++it1, ++counter);
		if(it1 != g_pixelCurve.end())
		{
			int j = *it1;
			std::vector<int>::iterator it2 = it1;
			for(int counter = 0; it2 != g_pixelCurve.end() && counter < kcurv; ++it2, ++counter);
			if(it2 != g_pixelCurve.end())
			{
				int k = *it2;
				double angle = getAngle(i, j, k, XRes, YRes);
				if(angle < c_thetathresh) 
				{
					int center = getCentroid(i, j, k, XRes, YRes);
					double avgdepth = (arr[i] + arr[j] + arr[k])*1.0 / 3;
					if(arr[center] > avgdepth - __FINGER_TRACKER_NOISE_TOLERANCE__ && 
						arr[center] < avgdepth + __FINGER_TRACKER_NOISE_TOLERANCE__)
					{
						if(tipNum < __FINGER_TRACKER_MAX_FINGERS__)
						{
							fingers.add(FingerVector(center%XRes, center/XRes, (int)arr[center], true));
							vertices.add(FingerVector(i%XRes, i/XRes, (int)arr[i], true));
							vertices.add(FingerVector(j%XRes, j/XRes, (int)arr[j], true));
							vertices.add(FingerVector(k%XRes, k/XRes, (int)arr[k], true));
							tipNum++;
						}
						for (int counter = 0; it != g_pixelCurve.end() && counter < kcurv*2; ++it, ++counter);
						if(it == g_pixelCurve.end()) break;
					}
				}
			}
		}
	}
	FingerFrame frame(fingers);
	frame.vertices = vertices;
	frame.curve = curve;
	return frame;
	//END ANALYSIS
}