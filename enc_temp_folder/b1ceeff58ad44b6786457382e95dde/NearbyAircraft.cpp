#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <strsafe.h>
#include <conio.h>

#include "SimConnect.h"
#include "Utilities.h"

bool shouldQuit = false;
HANDLE  hSimConnect = NULL;
double nmRadius = 10;
double lat = 32.951917;
double lon = -97.264323;
double alt = 3799;

struct AircraftInfo
{
    char    title[256];
    double  isUser;
    double  onGround;
    double  trueHeading;
    double  magHeading;
    double  altitude;
    double  latitude;
    double  longitude;
};

enum EVENT_ID {
    EVENT_SIM_START,
};

enum DATA_DEFINE_ID {
    DEFINITION_LOCAL_AIRCRAFT,
};

enum DATA_REQUEST_ID {
    REQUEST_LOCAL_AIRCRAFT,
};

void CALLBACK MyDispatchProcRD(SIMCONNECT_RECV* pData, DWORD cbData, void* pContext)
{

    printf("...\n");

    HRESULT hr;

    switch (pData->dwID)
    {
        case SIMCONNECT_RECV_ID_EVENT:
        {
            SIMCONNECT_RECV_EVENT* evt = (SIMCONNECT_RECV_EVENT*)pData;

            switch (evt->uEventID)
            {
                case EVENT_SIM_START:

                    // Now the sim is running, request information on the user aircraft
                    hr = SimConnect_RequestDataOnSimObjectType(hSimConnect, REQUEST_LOCAL_AIRCRAFT, DEFINITION_LOCAL_AIRCRAFT, 0, SIMCONNECT_SIMOBJECT_TYPE_USER);

                    break;

                default:
                    break;
            }
            break;
        }

        case SIMCONNECT_RECV_ID_SIMOBJECT_DATA_BYTYPE:
        {
            SIMCONNECT_RECV_SIMOBJECT_DATA_BYTYPE* pObjData = (SIMCONNECT_RECV_SIMOBJECT_DATA_BYTYPE*)pData;

            switch (pObjData->dwRequestID)
            {
                case REQUEST_LOCAL_AIRCRAFT:
                {
                    DWORD ObjectID = pObjData->dwObjectID;
                    AircraftInfo* pS = (AircraftInfo*)&pObjData->dwData;
                    if (SUCCEEDED(StringCbLengthA(&pS->title[0], sizeof(pS->title), NULL))) // security check
                    {
                        //printf("\nObjectID=%d  Title=\"%s\"\nLat=%f  Lon=%f  Alt=%f  Kohlsman=%.2f", ObjectID, pS->title, pS->latitude, pS->longitude, pS->altitude, pS->kohlsmann);
                        printf("\nObjectID=%d  Title=\"%s\"\nLat=%f  Lon=%f  Alt=%f\n", ObjectID, pS->title, pS->latitude, pS->longitude, pS->altitude);
                    }
                    break;
                }

                default:
                    break;
            }
            break;
        }


        case SIMCONNECT_RECV_ID_QUIT:
        {
            shouldQuit = true;
            break;
        }

        default:
            printf("\nReceived:%d", pData->dwID);
            break;
    }
}



void CALLBACK TestDispatchProc(SIMCONNECT_RECV* pData, DWORD cbData, void* pContext)
{


    HRESULT hr;

    switch (pData->dwID)
    {
    case SIMCONNECT_RECV_ID_SIMOBJECT_DATA_BYTYPE:
    {
        //printf("\nDEBUG: Parsing aircraft nearby...\n");
        SIMCONNECT_RECV_SIMOBJECT_DATA_BYTYPE* simObjData = (SIMCONNECT_RECV_SIMOBJECT_DATA_BYTYPE*)pData;

        switch (simObjData->dwRequestID)
        {
        case REQUEST_LOCAL_AIRCRAFT:
        {
            //printf("\nDEBUG: Aircraft Found...\n");

            DWORD ObjectID = simObjData->dwObjectID;
            AircraftInfo* aircraft = (AircraftInfo*)&simObjData->dwData;
            if (SUCCEEDED(StringCbLengthA(&aircraft->title[0], sizeof(aircraft->title), NULL))) // security check
            {   
                if (!aircraft->onGround)
                {
                    if (aircraft->isUser)
                    {
                        printf("\nUSER AIRCRAFT!!!\nObjectID=%d  Title=\"%s\"\nLat=%f  Lon=%f  Alt=%fft  HdgT=%.2f  HdgM=%.2f  OnGround=%f\n", ObjectID, aircraft->title, aircraft->latitude, aircraft->longitude, aircraft->altitude, aircraft->trueHeading * (180 / M_PI), aircraft->magHeading * (180 / M_PI), aircraft->onGround);
                        lat = aircraft->latitude;
                        lon = aircraft->longitude;
                    }
                    else
                    {
                        //printf("\nObjectID=%d  Title=\"%s\"\nLat=%f  Lon=%f  Alt=%fft  HdgT=%.2f  HdgM=%.2f  OnGround=%f  Range=%.2fnm  Brng=%.2f\n", ObjectID, aircraft->title, aircraft->latitude, aircraft->longitude, aircraft->altitude, aircraft->trueHeading * (180 / M_PI), aircraft->magHeading * (180 / M_PI), aircraft->onGround, rangeWithAlt(lat, lon, alt, aircraft->latitude, aircraft->longitude, aircraft->altitude), getBearing(lat, lon, aircraft->latitude, aircraft->longitude));
                        printf("\nObjectID=%d  Title=\"%s\"\nLat=%f  Lon=%f  Alt=%fft  HdgT=%.2f  HdgM=%.2f  OnGround=%f  Range=%.2fnm  Brng=%.2f\n", ObjectID, aircraft->title, aircraft->latitude, aircraft->longitude, aircraft->altitude, aircraft->trueHeading * (180 / M_PI), aircraft->magHeading * (180 / M_PI), aircraft->onGround, rangeWithAlt(lat, lon, alt, aircraft->latitude, aircraft->longitude, aircraft->altitude), getBearing(lat, lon, aircraft->latitude, aircraft->longitude));
                    }
                }
                //if (aircraftInfo->altitude >= 2500)
                //{
                //    printf("\nObjectID=%d  Title=\"%s\"\nLat=%f  Lon=%f  Alt=%fft  Range=%.2fnm  Brng=%.2f\n", ObjectID, aircraftInfo->title, aircraftInfo->latitude, aircraftInfo->longitude, aircraftInfo->altitude, distance(lat, lon, aircraftInfo->latitude, aircraftInfo->longitude), getBearing(lat, lon, aircraftInfo->latitude, aircraftInfo->longitude));
                //}
            }
            break;
        }

        default:
            printf("\nDEBUG: Not an Aircraft...\n");
            break;
        }
        break;
    }

    case SIMCONNECT_RECV_ID_QUIT:
    {
        printf("/nDEBUGGING: SIMCONNECT_RECV_ID_QUIT recieved...");
        shouldQuit = true;
        break;
    }

    default:
        printf("\nRequesting aircraft nearby...\n");
        hr = SimConnect_RequestDataOnSimObjectType(hSimConnect, REQUEST_LOCAL_AIRCRAFT, DEFINITION_LOCAL_AIRCRAFT, nmToMeters(nmRadius), SIMCONNECT_SIMOBJECT_TYPE_AIRCRAFT);
        break;
    }
}

void testDataRequest()
{
    HRESULT hr;

    if (SUCCEEDED(SimConnect_Open(&hSimConnect, "Request Data", NULL, 0, 0, 0)))
    {
        printf("\nConnected to Prepar3D!");
        printf("\nSearching a %.2f nm (%.2f m) radius\n", nmRadius, nmToMeters(nmRadius));

        // Set up the data definition, but do not yet do anything with it
        hr = SimConnect_AddToDataDefinition(hSimConnect, DEFINITION_LOCAL_AIRCRAFT, "Title", NULL, SIMCONNECT_DATATYPE_STRING256);
        hr = SimConnect_AddToDataDefinition(hSimConnect, DEFINITION_LOCAL_AIRCRAFT, "Is User Sim", "bool");
        hr = SimConnect_AddToDataDefinition(hSimConnect, DEFINITION_LOCAL_AIRCRAFT, "Sim On Ground", "bool");
        hr = SimConnect_AddToDataDefinition(hSimConnect, DEFINITION_LOCAL_AIRCRAFT, "Plane Heading Degrees True", "radians");
        hr = SimConnect_AddToDataDefinition(hSimConnect, DEFINITION_LOCAL_AIRCRAFT, "Plane Heading Degrees Magnetic", "radians");
        hr = SimConnect_AddToDataDefinition(hSimConnect, DEFINITION_LOCAL_AIRCRAFT, "Plane Altitude", "feet");
        hr = SimConnect_AddToDataDefinition(hSimConnect, DEFINITION_LOCAL_AIRCRAFT, "Plane Latitude", "degrees");
        hr = SimConnect_AddToDataDefinition(hSimConnect, DEFINITION_LOCAL_AIRCRAFT, "Plane Longitude", "degrees");

        // Request an event when the simulation starts
        //hr = SimConnect_SubscribeToSystemEvent(hSimConnect, EVENT_SIM_START, "SimStart");

        //SimConnect_RequestDataOnSimObject(hSimConnect, REQUEST_2, DEFINITION_1, SIMCONNECT_OBJECT_ID_, SIMCONNECT_PERIOD_SECOND);

        while (!shouldQuit)
        {
            //printf("Searching...");
            //SimConnect_CallDispatch(hSimConnect, MyDispatchProcRD, NULL);

            //char myVar;
            //myVar = getch();
            //if (myVar == 'f')
            //{
            //    printf("\nRefreshing list...\n");
            //}
            SimConnect_CallDispatch(hSimConnect, TestDispatchProc, NULL);

            Sleep(1000);
        }

        hr = SimConnect_Close(hSimConnect);
    }
}

int __cdecl _tmain(int argc, _TCHAR* argv[])
{

    testDataRequest();

    return 0;
}





