#pragma once

//#include <cmath>
#include <math.h>
#include <corecrt_math.h>

constexpr auto M_PI = 3.14159265358979323846;

double getBearing(double x1, double y1, double x2, double y2);

double nmToMeters(double nm);
double metersToNm(double meters);

long double distance(long double lat1, long double long1, long double lat2, long double long2);
double rangeWithAlt(double x1, double y1, double alt1, double x2, double y2, double alt2);
