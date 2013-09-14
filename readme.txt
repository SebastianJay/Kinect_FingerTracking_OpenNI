The Kinect Finger-tracker API
by Jay Sebastian

To view some API documentation, look in the "html" file and open "index.html" with any browser.

Hello! This API was made in C++, using only the standard library.
Please place all source files and headers in the same directory as your project.
No additional build options are necessary (of course, your middleware probably has some).

Basic usage:
1)	Include the file "FingerAnalysisAPI.h" in the source file that needs the algorithm.
2)	Insert a frame of depth data into an unsigned int array in row-major order (it is assumed the size of this array is 640x480)
3)	Call the method analyzeDepthArray(unsigned int* arr, int XRes, int YRes) and retrieve the returned value (a FingerFrame class)
4)	The FingerFrame class has various sorts of data.
		The avgPos member is the average position of all detected finger
		The deltaPos member is the change of the average position from the previous frame
		The fingers member is a list of all detected fingers
5)	The FingerList class is a wrapper for std::vector<FingerPoint>. I didn't bother to implement an iterator for this class,
	 so you must iterate through the list using an int (going from zero to size()), and access elements using typical vector square bracket notation.
6)	The FingerVector class represents a point or vector. 
		The x, y, and z members represent real space, in mm. The origin for this frame of reference is the center of the Kinect, with positive
		 x being to the right of the camera (when looking at it), positive y being above the camera, and positive z being pointing away.
		The px and py members (pz and z are the same) represent pixel space. They show where the fingers would be in the array that was passed
		 into the original method as an argument (the depth frame). This data is useful for showing the fingers graphically.

Algorithm explanation:
This algorithm doesn't use any other libraries - it just uses iterations and simple math. It functions as follows:

1)	Locate the nearest pixel (smallest depth value) in the given array.
2)	Look around that pixel (within a certain radius) for a neighboring pixel of similar depth, and store the result.
3)	Repeatedly perform (2) on the newfound pixel to build a sorted array called the "hand curve."
4)	Once the hand curve is finished, place three pointers a given distance apart within the curve.
5)	Have the three pointers iterate simultaneously through the curve. At each step, compare the angle between the three pixels the pointers point at.
	 If the angle is below a certain threshold, we can consider the center of those pixels to be a finger (since fingers, when seen onscreen, 
	 form acute angles).
6)	When a finger is found, advance the pointers a certain distance to avoid redetection of the same finger.

There are a couple constants in the algorithm; some were described earlier (i.e. the "certain" values). If you find the algorithm does not work to your
liking, you can try to adjust those constants, using methods in FingerTracker. The "theta threshold" describes the maximum angle between 3 pixels for its
centroid to be considered a finger (step (5)). The "search radius" is the distance for which the algorithm will search for neighboring pixels in step (2).
You can also adjust the minimum and maximum depths that the algorithm will work with, if you know your interface will only react to input from that range.

