/*
Declaration of FingerTracker class
*/
//Include Guard
#ifndef __FINGER_TRACKER_INCLUDED__
#define __FINGER_TRACKER_INCLUDED__

//Includes
#include <cmath>
#include <vector>
#include "FingerVector.h"
#include "FingerFrame.h"

//Defines
#define __FINGER_TRACKER_NOISE_TOLERANCE__ 100					//in mm
#define __FINGER_TRACKER_CURVE_LENGTH_MIN__ 250
#define __FINGER_TRACKER_CURVE_LENGTH_MAX__ 1000
#define __FINGER_TRACKER_DEPTH_VARY_MAX__ 150
#define __FINGER_TRACKER_ADVANCE_LEN__ 80
#define __FINGER_TRACKER_K_CURVE_MIN__ 35
#define __FINGER_TRACKER_K_CURVE_MAX__ 45
#define __FINGER_TRACKER_K_DEPTH_MIN__ 500
#define __FINGER_TRACKER_K_DEPTH_MAX__ 750
#define __FINGER_TRACKER_MAX_FINGERS__ 10
#define __FINGER_TRACKER_X_RES__ 640		//Kinect's x-resolution (change if needed)
#define __FINGER_TRACKER_Y_RES__ 480		//Kinect's y-resolution (change if needed)

//Code
/**
FingerTracker exposes a method that runs the finger detection algorithm (analyzeDepthArray), and it stores all customizable parameters for use in that algorithm.
*/
class FingerTracker
{
public:
	/**
	Default constructor; sets algorithm parameters to those used when testing
	*/
	FingerTracker()
	{
		c_radindex = 5;
		c_mindepth = 500;
		c_maxdepth = 1000;
		c_thetathresh = 40;
	}
	/**
	Constructor allowing for customization of algorithm parameters
	*/
	FingerTracker(int min, int max, int radius, double theta)
	{
		c_mindepth = min;
		c_maxdepth = max;
		c_radindex = radius;
		c_thetathresh = theta;
	}
	//main function (finger analysis)
	/**
	Runs the finger detection algorithm; input a ROW-MAJOR depth array, the x resolution and y resolution
	*/
	FingerFrame analyzeDepthArray(unsigned int* arr, int XRes, int YRes);
	
	//ability to adjust parameters
	/** Sets the radius, in pixels, for which the algorithm searches for neighbors when sorting edge pixels*/
	bool setSearchRadius(int r);
	/** Sets the minimum depth, in millimeters, for which depth data will be analyzed*/
	bool setMinDepth(int d);
	/** Sets the maximum depth, in millimeters, for which depth data will be analyzed*/
	bool setMaxDepth(int d);
	/** Sets the maximum value, in degrees, of the angle that can be formed by a finger curve for detection*/
	bool setThetaThreshold(double d);
	
private:	
	static const int c_dx[180];
	static const int c_dy[180];
	static const int c_radii[10];
	int c_radindex;
	
	unsigned int c_mindepth;
	unsigned int c_maxdepth;
	double c_thetathresh;

	unsigned int g_handPixelFrame[__FINGER_TRACKER_X_RES__ * __FINGER_TRACKER_Y_RES__];
	short g_segBuff[__FINGER_TRACKER_X_RES__ * __FINGER_TRACKER_Y_RES__];
	std::vector<int> g_pixelCurve;

	double getAngle(int i, int j, int k, int w, int h);
	int getCentroid(int i, int j, int k, int w, int h);
};

#endif