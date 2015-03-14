//-----------------------------------------------------------------
#include "global.h"
//#include "Math_System.h"
//-----------------------------------------------------------------
///*****************************************************************
//Math_System - CONSTRUCTORS
///*****************************************************************
Math_System::Math_System(void)
{
	//-------------------------------------------------------------
	//-------------------------------------------------------------
}

Math_System::~Math_System(void)
{
	//-------------------------------------------------------------
	//-------------------------------------------------------------
}
//*****************************************************************
//TENDTO
// Tend a value to its target, based on speed
// - Speed must be greater than 0.000001 and less than 1.0;
//*****************************************************************
void Math_System::TendTo(float &Input, float Target, float Speed)
{
	//-------------------------------------------------------------
	//Input += ((Target - Input) / (Speed / float(gET_Multi)));
	//Input += ((Target - Input) / Speed) * float(gET_Multi);
	//Input += (Target - Input) / Speed;

	Input += (Target - Input) * (Speed * float(gET_Multi));
	//-------------------------------------------------------------
}
void Math_System::TendTo(double &Input, double Target, double Speed)
{
	//-------------------------------------------------------------
	//Input += ((Target - Input) / (Speed / gET_Multi));
	//Input += ((Target - Input) / Speed) * gET_Multi;
	//Input += (Target - Input) / Speed;
	Input += (Target - Input) * (Speed * gET_Multi);
	//-------------------------------------------------------------
}
//*****************************************************************
//INTERPOLATE
//Interpolate input value from srcMin and srcMax, to dstMin and dstMax
//*****************************************************************
float Math_System::Interpolate(float input, float srcmin, float srcmax, float dstmin, float dstmax)
{
	float Value = dstmin + (dstmax - dstmin) * ((input - srcmin) / (srcmax - srcmin));
	//Cap to destination value min/max
	CapValue(Value, dstmin, dstmax);

	return Value;
}
double Math_System::Interpolate(double input, double srcmin, double srcmax, double dstmin, double dstmax)
{
	double Value = dstmin + (dstmax - dstmin) * ((input - srcmin) / (srcmax - srcmin));
	//Cap to destination value min/max
	CapValue(Value, dstmin, dstmax);

	return Value;
}
//*****************************************************************
//CAPVALUE
//Caps the input value between the Min and Max figures.
// - WARN: Ensure the min and max values do not exceed the Data Type Range for the input
//*****************************************************************
void Math_System::CapValue(char &input, char min, char max)
{
	if (input > max)
		input = max;
	else if (input < min)
		input = min;
}
void Math_System::CapValue(short &input, short min, short max)
{
	if (input > max)
		input = max;
	else if (input < min)
		input = min;
}
void Math_System::CapValue(int &input, int min, int max)
{
	if (input > max)
		input = max;
	else if (input < min)
		input = min;
}
void Math_System::CapValue(long &input, long min, long max)
{
	if (input > max)
		input = max;
	else if (input < min)
		input = min;
}
void Math_System::CapValue(float &input, float min, float max)
{
	if (input > max)
		input = max;
	else if (input < min)
		input = min;
}
void Math_System::CapValue(double &input, double min, double max)
{
	if (input > max)
		input = max;
	else if (input < min)
		input = min;
}
//*****************************************************************
//LERP
//*****************************************************************
float Lerp(float v0, float v1, float t)
{
	////Imprecise (v=v1, t!=1)
	//return v0 + t*(v1 - v0);

	//Precise (v=v1 if t=1)
	return (1.0f - t)*v0 + t*v1;
}
double Lerp(double v0, double v1, double t)
{
	////Imprecise (v=v1, t!=1)
	//return v0 + t*(v1 - v0);

	//Precise (v=v1 if t=1)
	return (1.0 - t)*v0 + t*v1;
}

