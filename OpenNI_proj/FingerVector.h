#ifndef __FINGER_VECTOR_INCLUDED__
#define __FINGER_VECTOR_INCLUDED__

#include <cmath>

#define __FINGER_VECTOR_HORIZONTAL_ANGLE__ 0.994837674		//Kinect's horizontal range of vision (radians)
#define __FINGER_VECTOR_VERTICAL_ANGLE__ 0.750491578		//Kinect's vertical range of vision (radians)
#define __FINGER_VECTOR_BAD_VAL__ 0
#define __FINGER_VECTOR_DEF_X_RES__ 640
#define __FINGER_VECTOR_DEF_Y_RES__ 480
#define __FINGER_VECTOR_DEF_Z_RES__ 10000

/**
FingerVector is a 3D vector class, with x, y, and z coordinates. These coordinates represent the Cartesian distance, in millimeters, from the depth sensor (where the depth sensor is the origin, the positive z axis is away from the camera, the positive x axis is to the left of the sensor (when facing out) and the positive y axis is toward the top of the sensor). The px, py, and pz coordinates are used for pixel space; that is, if fingers need to be displayed graphically onscreen, they would appear at (px, py) in the depth array which was passed into the analyzeDepthArray function. pz and z are the same (since there is no 3rd dimension in pixel space).
*/
struct FingerVector{
public:
	int x, y, z;
	int px, py, pz;

	/**
	Sets the resolution of all three dimensions, for conversion purposes from pixel space to real space
	Default is 640, 480, 10000 (pixels, pixels, mm)
	*/
	static void setMaxValues(int, int, int);

	/**
	Default constructor
	*/
	FingerVector();
	/**
	Constructor with three ints as px, py, and pz (they will remain unconverted to real space)
	*/
	FingerVector(int, int, int);
	/**
	Constructor with three ints as px, py, and pz; the bool flag tells whether to convert and update x, y, and z as well
	*/
	FingerVector(int, int, int, bool);

	FingerVector operator+ (FingerVector);
	FingerVector operator- (FingerVector);
	FingerVector operator/ (double);
	FingerVector operator* (double);
	/**
	Magnitude of the vector
	*/
	double magnitude(); 

private:
	static int XRes, YRes, ZRes;
	void transformLateralPixels();
};

#endif