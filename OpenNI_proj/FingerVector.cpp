#include "FingerVector.h"

//declare static vars
int FingerVector::XRes = __FINGER_VECTOR_DEF_X_RES__;
int FingerVector::YRes = __FINGER_VECTOR_DEF_Y_RES__;
int FingerVector::ZRes = __FINGER_VECTOR_DEF_Z_RES__;
//

void FingerVector::setMaxValues(int xx, int yy, int zz)
{
	XRes = xx;
	YRes = yy;
	ZRes = zz;
}

FingerVector::FingerVector() 
{
	px = __FINGER_VECTOR_BAD_VAL__;
	py = __FINGER_VECTOR_BAD_VAL__;
	pz = __FINGER_VECTOR_BAD_VAL__;
	x = __FINGER_VECTOR_BAD_VAL__;
	y = __FINGER_VECTOR_BAD_VAL__;
	z = __FINGER_VECTOR_BAD_VAL__;
}

FingerVector::FingerVector(int xx, int yy, int zz) 
{ 
	px = xx; 
	py = yy; 
	pz = zz;
	x = __FINGER_VECTOR_BAD_VAL__;
	y = __FINGER_VECTOR_BAD_VAL__;
	z = __FINGER_VECTOR_BAD_VAL__;
}

FingerVector::FingerVector(int xx, int yy, int zz, bool flag) 
{ 
	px = xx; 
	py = yy; 
	pz = zz;
	z = zz;
	if(flag)
		transformLateralPixels();
	else
	{
		x = xx;
		y = yy;
	}
}

void FingerVector::transformLateralPixels()
{
	double xx = px*1.0;
	double yy = py*1.0; //copies
	xx = xx-(XRes/2);
	yy = -1*(yy-(YRes/2)); //translating origin (assuming it's at top-left corner of array)
	int xrange = (int)(tan(__FINGER_VECTOR_HORIZONTAL_ANGLE__/2)*pz);
	int yrange = (int)(tan(__FINGER_VECTOR_VERTICAL_ANGLE__/2)*pz);
	xx = xx/(XRes/2);
	yy = yy/(YRes/2);
	xx = xx*xrange;
	yy = yy*yrange;

	x = (int)xx;
	y = (int)yy;
}

double FingerVector::magnitude()
{
	return sqrt((x*x*1.0)+(y*y*1.0)+(z*z*1.0));
}

FingerVector FingerVector::operator+ (FingerVector param)
{
	FingerVector ans;
	ans.x = this->x + param.x;
	ans.y = this->y + param.y;
	ans.z = this->z + param.z;
	ans.px = this->px + param.px;
	ans.py = this->py + param.py;
	ans.pz = this->pz + param.pz;
	return ans;
}

FingerVector FingerVector::operator- (FingerVector param)
{
	FingerVector ans;
	ans.x = this->x - param.x;
	ans.y = this->y - param.y;
	ans.z = this->z - param.z;
	ans.px = this->px -param.px;
	ans.py = this->py - param.py;
	ans.pz = this->pz - param.pz;
	return ans;
}

FingerVector FingerVector::operator/ (double param)
{
	FingerVector ans;
	ans.x = (int)(this->x / param);
	ans.y = (int)(this->y / param);
	ans.z = (int)(this->z / param);
	ans.px = (int)(this->px / param);
	ans.py = (int)(this->py / param);
	ans.pz = (int)(this->pz / param);
	return ans;
}

FingerVector FingerVector::operator* (double param)
{
	FingerVector ans;
	ans.x = (int)(this->x * param);
	ans.y = (int)(this->y * param);
	ans.z = (int)(this->z * param);
	ans.px = (int)(this->px * param);
	ans.py = (int)(this->py * param);
	ans.pz = (int)(this->pz * param);
	return ans;
}