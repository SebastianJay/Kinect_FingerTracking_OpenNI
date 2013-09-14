#include "FingerFrame.h"
#include "FingerVector.h"
#include <ctime>

// declare static vars
FingerVector FingerFrame::prevPos;
int FingerFrame::frameCount = 0;
int FingerFrame::prevNumFingers;
//

FingerFrame::FingerFrame()
{
}

FingerFrame::FingerFrame(FingerList param)
{
	//init data
	fingers = param;
	frameNum = ++frameCount;
	avgPos = FingerVector();
	deltaPos = FingerVector();

	//calculate average
	if(param.size() > 0)
	{
		FingerVector total;
		for(int i = 0; i < param.size(); i++)
		{
			total = total + param[i];
		}
		avgPos = total / param.size();
	}
	//calculate differences
	if(FingerFrame::prevNumFingers == param.size() && frameNum > 1 && param.size() > 0)
	{
		deltaPos = avgPos-prevPos;
	}

	FingerFrame::prevPos = avgPos;
	FingerFrame::prevNumFingers = param.size();
}