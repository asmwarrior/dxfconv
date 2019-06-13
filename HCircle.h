// HCircle.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "IdNamedObj.h"
#include "ExtrudedObj.h"
#include "HeeksColor.h"

class HCircle : public ExtrudedObj<IdNamedObj>{
private:
	HeeksColor color;

public:
	gp_Ax1 m_axis;
	double m_radius;

	~HCircle(void);
	HCircle(const gp_Circ &c, const HeeksColor* col);
	HCircle(const HCircle &c);
	HCircle(){}

	const HCircle& operator=(const HCircle &c);

	// HeeksObj's virtual functions
	int GetType()const{ return CircleType; }
	long GetMarkingMask()const{ return MARKING_FILTER_CIRCLE; }
	void GetBox(CBox &box);
	const wchar_t* GetTypeString(void)const{ return L"Circle"; }
	int GetIDGroupType()const{ return SketchType; }
	HeeksObj *MakeACopy(void)const;
	void ModifyByMatrix(const double *mat);
	void SetColor(const HeeksColor &col){ color = col; }
	const HeeksColor* GetColor()const{ return &color; }
	bool FindNearPoint(const double* ray_start, const double* ray_direction, double *point);
	bool FindPossTangentPoint(const double* ray_start, const double* ray_direction, double *point);
	bool Stretch(const double *p, const double* shift, void* data);
	void GetSegments(void(*callbackfunc)(const double *p), double pixels_per_mm, bool want_start_point = true)const;
	bool GetCentrePoint(double* pos);
	int Intersects(const HeeksObj *object, std::list< double > *rl)const;
	bool DescendForUndo(){ return false; }
	bool IsDifferent(HeeksObj* other);

	static bool GetLineTangentPoints(const gp_Circ& c1, const gp_Circ& c2, const gp_Pnt& a, const gp_Pnt& b, gp_Pnt& p1, gp_Pnt& p2);
	static bool GetLineTangentPoint(const gp_Circ& c, const gp_Pnt& a, const gp_Pnt& b, gp_Pnt& p);
	static bool GetArcTangentPoints(const gp_Circ& c, const gp_Lin &line, const gp_Pnt& p, double radius, gp_Pnt& p1, gp_Pnt& p2, gp_Pnt& centre, gp_Dir& axis);
	static bool GetArcTangentPoints(const gp_Circ& c1, const gp_Circ &c2, const gp_Pnt& a, const gp_Pnt& b, double radius, gp_Pnt& p1, gp_Pnt& p2, gp_Pnt& centre, gp_Dir& axis);
	static bool GetArcTangentPoints(const gp_Lin& l1, const gp_Lin &l2, const gp_Pnt& a, const gp_Pnt& b, double radius, gp_Pnt& p1, gp_Pnt& p2, gp_Pnt& centre, gp_Dir& axis);
	static bool GetArcTangentPoint(const gp_Lin& l, const gp_Pnt& a, const gp_Pnt& b, const gp_Vec *final_direction, double* radius, gp_Pnt& p, gp_Pnt& centre, gp_Dir& axis);
	static bool GetArcTangentPoint(const gp_Circ& c, const gp_Pnt& a, const gp_Pnt& b, const gp_Vec *final_direction, double* radius, gp_Pnt& p, gp_Pnt& centre, gp_Dir& axis);

	void SetCircle(gp_Circ c);
	gp_Circ GetCircle() const;
	double GetDiameter()const;
};
