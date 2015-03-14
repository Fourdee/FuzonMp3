#pragma once
//-----------------------------------------------------------------
//-----------------------------------------------------------------
class Math_System
{
	//-------------------------------------------------------------
public:
	//---------------------------------------------------------
	//---------------------------------------------------------
	//constructors
	Math_System(void);
	~Math_System(void);
	//---------------------------------------------------------
	void	TendTo(float &Input, float Target, float Speed);
	void	TendTo(double &Input, double Target, double Speed);

	float	Interpolate(float input, float srcmin, float srcmax, float dstmin, float dstmax);
	double	Interpolate(double input, double srcmin, double srcmax, double dstmin, double dstmax);

	void	CapValue(char &input, char min, char max);
	void	CapValue(short &input, short min, short max);
	void	CapValue(int &input, int min, int max);
	void	CapValue(long &input, long min, long max);
	void	CapValue(float &input, float min, float max);
	void	CapValue(double &input, double min, double max);

	float	Lerp(float v0, float v1, float t);
	double	Lerp(double v0, double v1, double t);
	//-------------------------------------------------------------
private:
	//-------------------------------------------------------------
	//-------------------------------------------------------------
};
