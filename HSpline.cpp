// HSpline.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"

#include "HSpline.h"
#include "HLine.h"
#include "HILine.h"
#include "HArc.h"

CTangentialArc::CTangentialArc(const gp_Pnt &p0, const gp_Vec &v0, const gp_Pnt &p1):m_p0(p0), m_v0(v0), m_p1(p1)
{
	// calculate a tangential arc that goes through p0 and p1, with a direction of v0 at p0
	m_is_a_line = !HArc::TangentialArc(m_p0, m_v0, m_p1, m_c, m_a);
}

bool CTangentialArc::radius_equal(const gp_Pnt &p, double tolerance)const
{
	if(m_is_a_line)return true;

	double point_radius = gp_Vec(m_c.XYZ() - p.XYZ()).Magnitude();
	double diff =  fabs(point_radius - radius());

	return diff <= tolerance;
}

double CTangentialArc::radius()const
{
	double r0 = gp_Vec(m_p0.XYZ() - m_c.XYZ()).Magnitude();
	double r1 = gp_Vec(m_p1.XYZ() - m_c.XYZ()).Magnitude();
	double r = (r0 + r1)/2;
	return r;
}

HeeksObj* CTangentialArc::MakeHArc()const
{
	if(m_is_a_line)
	{
		return new HLine(m_p0, m_p1, &(theApp.current_color));
	}

	gp_Circ c(gp_Ax2(m_c, m_a), radius());
	HArc* new_object = new HArc(m_p0, m_p1, c, &(theApp.current_color));
	return new_object;
}

HSpline::HSpline(const HSpline &s):EndedObject(){
	operator=(s);
}

HSpline::HSpline(const Geom_BSplineCurve &s, const HeeksColor* col):EndedObject(){
	m_spline = Handle(Geom_BSplineCurve)::DownCast(s.Copy());	
	m_spline->D0(m_spline->FirstParameter(), A);
	m_spline->D0(m_spline->LastParameter() , B);
	SetColor(*col);
}

HSpline::HSpline(Handle_Geom_BSplineCurve s, const HeeksColor* col):EndedObject(){
	m_spline = s;//Handle(Geom_BSplineCurve)::DownCast(s->Copy());
	m_spline->D0(m_spline->FirstParameter(), A);
	m_spline->D0(m_spline->LastParameter() , B);
	SetColor(*col);
}

HSpline::HSpline(const std::list<gp_Pnt> &points, const HeeksColor* col):EndedObject()
{
	Standard_Boolean periodicity = points.front().IsEqual(points.back(), theApp.m_geom_tol);

	unsigned int size = points.size();
	if(periodicity == Standard_True)size--;

#ifdef _DEBUG
#undef new
#endif
	TColgp_HArray1OfPnt *Array = new TColgp_HArray1OfPnt(1, size);
#ifdef _DEBUG
#define new  WXDEBUG_NEW
#endif

	unsigned int i = 1;
	for(std::list<gp_Pnt>::const_iterator It = points.begin(); i <= size; It++, i++)
	{
		Array->SetValue(i, *It);
	}

	GeomAPI_Interpolate anInterpolation(Array, periodicity, Precision::Approximation());
	anInterpolation.Perform();
	m_spline = anInterpolation.Curve();
	m_spline->D0(m_spline->FirstParameter(), A);
	m_spline->D0(m_spline->LastParameter() , B);
	SetColor(*col);
}

HSpline::~HSpline(){
}

const HSpline& HSpline::operator=(const HSpline &s){
	EndedObject::operator=(s);
	m_spline = Handle(Geom_BSplineCurve)::DownCast((s.m_spline)->Copy());;
	return *this;
}

bool HSpline::IsDifferent(HeeksObj* o)
{
	HSpline* other = (HSpline*)o;
	for(int i=1; i <= m_spline->NbPoles(); i++)
	{
		if(m_spline->Pole(i).Distance(other->m_spline->Pole(i))>theApp.m_geom_tol)
			return true;
	}
	
	return EndedObject::IsDifferent(o);
}

//segments - number of segments per full revolution!
void HSpline::GetSegments(void(*callbackfunc)(const double *p), double pixels_per_mm, bool want_start_point)const
{

	//TODO: calculate length
	double u0 = m_spline->FirstParameter();
	double u1 = m_spline->LastParameter();
	double uw = u1 - u0;

	gp_Pnt p0, phalf, p1;
	m_spline->D0(u0, p0);
	m_spline->D0(u0 + uw*0.5, phalf);
	m_spline->D0(u1, p1);
	double approx_length = p0.Distance(phalf) + phalf.Distance(p1);

	int segments = (int)(fabs(pixels_per_mm * approx_length + 1));
	double pp[3];

	for(int i = 0; i <= segments; i++)
    {
		gp_Pnt p;
		m_spline->D0(u0 + ((double)i / segments) * uw,p);
		extract(p, pp);
		(*callbackfunc)(pp);
    } 
}

HeeksObj *HSpline::MakeACopy(void)const{
		HSpline *new_object = new HSpline(*this);
		return new_object;
}

void HSpline::ModifyByMatrix(const double* m){
	gp_Trsf mat = make_matrix(m);
	m_spline->Transform(mat);
}

void HSpline::GetBox(CBox &box){
	//TODO: get rid of magic number
	double pp[3];
	double u0 = m_spline->FirstParameter();
	double u1 = m_spline->LastParameter();
	double uw = u1 - u0;

	for(int i = 0; i <= 100; i++)
    {
		gp_Pnt p;
		m_spline->D0(u0 + uw * .01,p);
		extract(p, pp);
		box.Insert(pp);
    } 
}

bool HSpline::FindNearPoint(const double* ray_start, const double* ray_direction, double *point){
	gp_Lin ray(make_point(ray_start), make_vector(ray_direction));
	std::list< gp_Pnt > rl;

	return false; 
}

bool HSpline::FindPossTangentPoint(const double* ray_start, const double* ray_direction, double *point){
	// any point on this ellipse is a possible tangent point
	return FindNearPoint(ray_start, ray_direction, point);
}

bool HSpline::Stretch(const double *p, const double* shift, void* data){

	gp_Pnt vp = make_point(p);
	gp_Vec vshift = make_vector(shift);

	//There may be multiple control points at the same location, so we must move them all
	// This could break if user drags one set of points over another
	// The correct mapping of handles to control points can be determined
	// from the multiplicity set
	for(int i = 1; i <= m_spline->NbPoles(); i++)
	{
		if(m_spline->Pole(i).IsEqual(vp, theApp.m_geom_tol))
		{
			m_spline->SetPole(i,m_spline->Pole(i).XYZ() + vshift.XYZ());
		}
  	}
	return false; 
}

int HSpline::Intersects(const HeeksObj *object, std::list< double > *rl)const
{
/*	int numi = 0;

	switch(object->GetType())
	{
	case LineType:
		{
			std::list<gp_Pnt> plist;
			intersect(((HLine*)object)->GetLine(), m_spline, plist);
			for(std::list<gp_Pnt>::iterator It = plist.begin(); It != plist.end(); It++)
			{
				gp_Pnt& pnt = *It;
				if(((HLine*)object)->Intersects(pnt))
				{
					if(rl)add_pnt_to_doubles(pnt, *rl);
					numi++;
				}
			}
		}
		break;

	case ILineType:
		{
			std::list<gp_Pnt> plist;
			intersect(((HILine*)object)->GetLine(), m_spline, plist);
			if(rl)convert_pnts_to_doubles(plist, *rl);
			numi += plist.size();
		}
		break;

	case ArcType:
		{
			std::list<gp_Pnt> plist;
			intersect(m_spline, ((HArc*)object)->m_circle, plist);
			for(std::list<gp_Pnt>::iterator It = plist.begin(); It != plist.end(); It++)
			{
				gp_Pnt& pnt = *It;
				if(((HArc*)object)->Intersects(pnt))
				{
					if(rl)add_pnt_to_doubles(pnt, *rl);
					numi++;
				}
			}
		}
		break;

	case CircleType:
		{
			std::list<gp_Pnt> plist;
			intersect(m_spline, ((HSpline*)object)->m_circle, plist);
			if(rl)convert_pnts_to_doubles(plist, *rl);
			numi += plist.size();
		}
		break; 
	}
*/
	return 0; //return numi; 
}

static bool calculate_biarc_points(const gp_Pnt &p0, gp_Vec v_start, const gp_Pnt &p4, gp_Vec v_end, gp_Pnt &p1, gp_Pnt &p2, gp_Pnt &p3)
{
	if(v_start.Magnitude() < 0.0000000001)v_start = gp_Vec(p0, p1);
    if(v_end.Magnitude() < 0.0000000001)v_end = gp_Vec(p3, p4);

	v_start.Normalize();
	v_end.Normalize();

    gp_Vec v = p0.XYZ() - p4.XYZ();

    double a = 2*(v_start*v_end-1);
    double c = v*v;
    double b = (v*2)*(v_start+v_end);

	if(fabs(a) < 0.000000000000001)return false;

    double d = b*b-4*a*c;

    if(d < 0.0)return false;

	double sd = sqrt(d);

    double e1 = (-b - sd) / (2.0 * a);
    double e2 = (-b + sd) / (2.0 * a);

    if(e1 > 0 && e2 > 0)return false;

	double e = e1;
	if(e2 > e)e = e2;

    if(e < 0)return false;

    p1 = p0.XYZ() + v_start.XYZ() * e;
    p3 = p4.XYZ() - v_end.XYZ() * e;
    p2 = p1.XYZ() * 0.5 + p3.XYZ() * 0.5;

	return true;
}

static std::list<HeeksObj*>* new_spans_for_CreateArcs = NULL;
static double tolerance_for_CreateArcs = 1.0;
static Handle(Geom_BSplineCurve) spline_for_CreateArcs = NULL;

void CreateArcs(const gp_Pnt &p_start, const gp_Vec &v_start, double t_start, double t_end, gp_Pnt &p_end, gp_Vec &v_end)
{
	spline_for_CreateArcs->D1(t_end, p_end, v_end);

	gp_Pnt p1, p2, p3;

	bool can_do_spline_whole = calculate_biarc_points(p_start, v_start, p_end, v_end, p1, p2, p3);

	HeeksObj* arc_object1 = NULL;
	HeeksObj* arc_object2 = NULL;

	if(can_do_spline_whole)
	{
		CTangentialArc arc1(p_start, v_start, p2);
		CTangentialArc arc2(p2, gp_Vec(p3.XYZ() - p2.XYZ()), p_end);

		gp_Pnt p_middle1, p_middle2;
		spline_for_CreateArcs->D0(t_start + ((t_end - t_start) * 0.25), p_middle1);
		spline_for_CreateArcs->D0(t_start + ((t_end - t_start) * 0.75), p_middle2);

		if(!arc1.radius_equal(p_middle1, tolerance_for_CreateArcs) || !arc2.radius_equal(p_middle2, tolerance_for_CreateArcs))
			can_do_spline_whole = false;
		else
		{
			arc_object1 = arc1.MakeHArc();
			arc_object2 = arc2.MakeHArc();
		}
	}
	else
	{
		// calculate_biarc_points failed, just add a line
		new_spans_for_CreateArcs->push_back(new HLine(p_start, p_end, &(theApp.current_color)));
		return;
	}
	
	if(can_do_spline_whole)
	{
		new_spans_for_CreateArcs->push_back(arc_object1);
		new_spans_for_CreateArcs->push_back(arc_object2);
	}
	else
	{
		double t_middle = t_start + ((t_end - t_start) * 0.5);
		gp_Pnt p_middle;
		gp_Vec v_middle;
		CreateArcs(p_start, v_start, t_start, t_middle, p_middle, v_middle);// recursive
		gp_Pnt new_p_end;
		gp_Vec new_v_end;
		CreateArcs(p_middle, v_middle, t_middle, t_end, new_p_end, new_v_end);
	}
}

bool HSpline::GetStartPoint(double* pos)
{
	gp_Pnt p;
	m_spline->D0(m_spline->FirstParameter(), p);
	extract(p, pos);
	return true;
}

bool HSpline::GetEndPoint(double* pos)
{
	gp_Pnt p;
	m_spline->D0(m_spline->LastParameter(), p);
	extract(p, pos);
	return true;
}

void HSpline::ToBiarcs(const Handle_Geom_BSplineCurve s, std::list<HeeksObj*> &new_spans, double tolerance, double first_parameter, double last_parameter)
{
	new_spans_for_CreateArcs = &new_spans;
	if(tolerance < 0.000000000000001)tolerance = 0.000000000000001;
	tolerance_for_CreateArcs = tolerance;
	gp_Pnt p_start;
	gp_Vec v_start;
	gp_Pnt p_end;
	gp_Vec v_end;
	s->D1(first_parameter, p_start, v_start);
	spline_for_CreateArcs = s;
	CreateArcs(p_start, v_start, first_parameter, last_parameter, p_end, v_end);
}

void HSpline::ToBiarcs(std::list<HeeksObj*> &new_spans, double tolerance)const
{
	ToBiarcs(m_spline, new_spans, tolerance, m_spline->FirstParameter(), m_spline->LastParameter());
}

void HSpline::Reverse()
{
	m_spline->Reverse();
}
