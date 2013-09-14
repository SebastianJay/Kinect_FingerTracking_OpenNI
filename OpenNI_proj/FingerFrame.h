#ifndef __FINGER_FRAME_INCLUDED__
#define __FINGER_FRAME_INCLUDED__

#include "FingerVector.h"
#include "FingerList.h"
#include <ctime>

/**
FingerFrame contains information about the finger data of one depth frame. It is returned by the FingerTracker method analyzeDepthArray.
*/
struct FingerFrame{
public:
	/**
	Default constructor (empty)
	*/
	FingerFrame();
	/**
	Constructor that takes a FingerList as initial input
	*/
	FingerFrame(FingerList);

	/**
	Frame index. Whenever a new FingerFrame is instantiated, the index increments.
	*/
	int frameNum;
	/**
	Contains data about all detected fingers for this frame
	*/
	FingerList fingers;
	//use this list to access finger data
	
	/**TEMP*/
	FingerList vertices;
	/**TEMP*/
	FingerList curve; 
	
	/**
	Represents the average position of all the detected fingers for this frame
	*/
	FingerVector avgPos;

	/**
	//Used to tell the change in finger position since the previous analyzed frame;
	//for the first frame, and during and after frames with no fingers, this member is zero.
	*/
	FingerVector deltaPos;


private:
	static int prevNumFingers;
	static FingerVector prevPos;
	static int frameCount;
};

#endif