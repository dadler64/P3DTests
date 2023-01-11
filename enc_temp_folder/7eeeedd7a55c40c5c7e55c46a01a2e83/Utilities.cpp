#include "Utilities.h"

/**
* Get the bearing (forward azimuth) in degrees between 2 sets of lat/lon coordinates
* https://www.movable-type.co.uk/scripts/latlong.html
*/
double getBearing(double srcLat, double srcLon, double destLat, double destLon)
{
    double y = sin(destLon - srcLon) * cos(destLat);
    double x = cos(srcLat) * sin(destLat) - sin(srcLat) * cos(destLat) * cos(destLon - srcLon);
	double radians = atan2(y, x);

    double degrees = radians * (180.0 / M_PI);
    return fmod(degrees + 180, 360);
}


/**
* Function to convert Nautical Miles (nm) to Meters (m) rounded to the nearest meter
* Function to convert Nautical Miles (nm) to Meters (m) rounded to the nearest meter
* https://en.wikipedia.org/wiki/Nautical_mile
*/
double nmToMeters(double nm)
{
	return round(nm * 1852);
}


/**
* Function to convert Meters (m) Nautical Miles (nm)
* https://en.wikipedia.org/wiki/Nautical_mile
*/
double metersToNm(double meters)
{
	return meters / 1852;
}

/** 
* Utility function for converting degrees to radians
* https://www.geeksforgeeks.org/program-distance-two-points-earth/
*/
long double toRadians(const long double degree)
{
    long double one_deg = (M_PI) / 180;
    return (one_deg * degree);
}

long double distance(long double lat1, long double long1, long double lat2, long double long2)
{
    // Convert the latitudes
    // and longitudes
    // from degree to radians.
    lat1 = toRadians(lat1);
    long1 = toRadians(long1);
    lat2 = toRadians(lat2);
    long2 = toRadians(long2);

    // Haversine Formula
    long double dlong = long2 - long1;
    long double dlat = lat2 - lat1;

    long double ans = pow(sin(dlat / 2), 2) +
        cos(lat1) * cos(lat2) *
        pow(sin(dlong / 2), 2);

    ans = 2 * asin(sqrt(ans));

    // Radius of Earth in
    // Kilometers, R = 6371
    // Use R = 3956 for miles
    long double R = 3443.9308855292; // Nuatical miles


    // Calculate the result
    ans = ans * R;

    return ans;
}


double rangeWithAlt(double x1, double y1, double alt1, double x2, double y2, double alt2)
{
    double deltaHeight;
    double alt1Nm = alt1 / 6076;
    double alt2Nm = alt2 / 6076;
    double range = distance(x1, y1, x2, y2);

    if (alt1 > alt2)
    {
        deltaHeight = alt1Nm - alt2Nm;
    }
    else
    {
        deltaHeight = alt1Nm - alt2Nm;
    }

    return sqrtf(powf(range, 2) + powf(deltaHeight, 2));
}