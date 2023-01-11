// Copyright (c) 2010-2022 Lockheed Martin Corporation. All rights reserved.
// Use of this file is bound by the PREPAR3D® SOFTWARE DEVELOPER KIT END USER LICENSE AGREEMENT

// RadarPanelCallback.cpp

#define _CRT_RAND_S
#include "gauges.h"
#include "NetInOutPublic.h"
#include "PDK.h"
#include "ISimulatedRadar.h"

GAUGE_CALLBACK gauge_callback;

using namespace Radar;

// Note: The items in the property table correspond to the indices that
// will be returned in the Get/Set Property functions
struct PROPERTY_TABLE
{
    PCSTRINGZ szPropertyName;
    PCSTRINGZ szUnitsName;
    ENUM      units;
};

class RadarPanelCallback : public IPanelCCallback
{
    DECLARE_PANEL_CALLBACK_REFCOUNT(PanelCallback);
public:
    RadarPanelCallback( ISimulatedRadarV400 * pSimRadar );
    
    // ******* IPanelCCallback Methods *****************    
    virtual IPanelCCallback* QueryInterface(LPCSTR pszInterface ) override  { return nullptr; }
    virtual UINT32 GetVersion() override                                        { return 1;    }
    virtual bool ConvertStringToProperty( PCSTRINGZ keyword, SINT32 * pID ) override;
    virtual bool ConvertPropertyToString( SINT32 id, PPCSTRINGZ pKeyword ) override;
    virtual bool GetPropertyUnits( SINT32 id, ENUM * pEnum ) override;
    virtual IAircraftCCallback * CreateAircraftCCallback( UINT32 ContainerID ) override;
protected:
    const PROPERTY_TABLE *GetPropertyTable( UINT &uLength );
    CComPtr<ISimulatedRadarV400> m_pRadar;
};

class RadarAircraftCallback : public IAircraftCCallback 
{
    DECLARE_PANEL_CALLBACK_REFCOUNT(AircraftCallback);
public:
    RadarAircraftCallback( UINT32 containerId, ISimulatedRadarV400 * pRadar );

    // ******* IAircraftCCallback Methods ************* 
    virtual  IAircraftCCallback* QueryInterface(LPCSTR pszInterface ) override  {  return nullptr;  }
    virtual void Update() override {}
    virtual IGaugeCCallback * CreateGaugeCCallback() override;
protected:    
    UINT32 GetContainerId()  const  { return m_containerId; }

private:
    UINT32 m_containerId;
    CComPtr<ISimulatedRadarV400> m_pRadar;
};

//
// Class that implements IGaugeCCallback
//
class RadarGaugeCallback : public ISerializableGaugeCCallback
{
    DECLARE_PANEL_CALLBACK_REFCOUNT(P3DRADARGaugeCallback);
	
public:
    RadarGaugeCallback(UINT32 containerId, ISimulatedRadarV400 * pRadar);
        
    // ************* IGaugeCCallback Methods ***************
    bool GetPropertyValue (SINT32 id, FLOAT64* pValue);
    bool SetPropertyValue (SINT32 id, FLOAT64 value);

    // **** IGaugeCCallback Methods Not being used by this implementation ****
    IGaugeCCallback* QueryInterface(LPCSTR pszInterface);
    void Update()                                           {               }
    bool GetPropertyValue(SINT32 id, LPCSTR* pszValue)      { return false; }
    bool SetPropertyValue(SINT32 id, LPCSTR szValue)        { return false; }
    bool GetPropertyValue(SINT32 id, LPCWSTR* pszValue)     { return false; }
    bool SetPropertyValue(SINT32 id, LPCWSTR szValue)       { return false; }
    IGaugeCDrawable* CreateGaugeCDrawable(SINT32 id, const IGaugeCDrawableCreateParameters* pParameters) { return nullptr; }

    // Note: As of Prepar3D 2.3 these will get called on flight load/save in addition to the original shared cockpit use case.
    bool Serialize(NetOutPublic& netout);
    bool Deserialize(NetInPublic& netin);

protected:
    // Because this callback acts as a pass-though to the radar system serialization is a bit more complex than the general case.
    // Where all the property values are stored in member variables that can be written out and read in. 
    // Because of this an internal struct is used to wrap the serialization data. 

    // use a magic value as the radar header
    #define RADAR_CALLBACK_SERIALIZATION_HEADER_ID 0xffa41af2
    // include a version number in case this plug-in gets a new version that
    // needs to serialize more data
    #define RADAR_CALLBACK_SERIALIZATION_VERSIONS 1
    struct RadarGaugeSerializationData
    {
        struct DataHeader
        {
            DataHeader(): HeaderID(RADAR_CALLBACK_SERIALIZATION_HEADER_ID),
                                                SizeInBytes(sizeof(RadarGaugeSerializationData)),
                                                Version(RADAR_CALLBACK_SERIALIZATION_VERSIONS)    
            { }
            // Check the magic value against the define above to make sure the this plugin has data to load
            bool IsValid()           const  { return (HeaderID == RADAR_CALLBACK_SERIALIZATION_HEADER_ID); }
            // Check version to make sure it is the same
            bool IsCurrentVersion()  const  { return (Version == RADAR_CALLBACK_SERIALIZATION_VERSIONS);  }
            // Check size of data stored
            bool ValidateSize()      const  { return SizeInBytes == sizeof(RadarGaugeSerializationData); }
            unsigned long HeaderID;
            int SizeInBytes;
            int Version;
        };

        DataHeader Header;
        bool ShowRangeRings;
        bool ShowCursor;
        bool FarShoreEnhance;
        double VisualZoom;
        double DataZoom;
        double ScanAzimuth;
        double SweepRate;
        double RangeMiles;
        bool RenderingEnabled;
        double CursorPositionX;
        double CursorPositionY;
        bool FreezeEnabled;
        double FrontBlindspotDegrees;
        double SideBlindspotDegrees;
        double RadarResolutionX;
        double RadarResolutionY;
        double GaugeResolutionX;
        double GaugeResolutionY;
    };

    ~RadarGaugeCallback();
private:
    bool   m_bRadarNeedsDeinit;
    UINT32 m_containerId;
    ISimulatedRadarV400 * m_pRadar;
};

// Enum that contains the properties 
enum P3DRADAR_VAR
{
    P3DRADAR_ClearRadarImage,
    P3DRADAR_ShowRangeRings,
    P3DRADAR_ShowCursor,
    P3DRADAR_FarShoreEnhance,
    P3DRADAR_VisualZoom,
    P3DRADAR_DataZoom,
    P3DRADAR_ScanAzimuth,
    P3DRADAR_SweepRate,
    P3DRADAR_RangeMiles,
    P3DRADAR_RenderingEnabled,
    P3DRADAR_FreezeEnabled,
    P3DRADAR_CursorPositionX,
    P3DRADAR_CursorPositionY,
    P3DRADAR_CursorPositionLat,
    P3DRADAR_CursorPositionLon,
    P3DRADAR_FrontBlindSpotDegrees,
    P3DRADAR_SideBlindSpotDegrees,
    P3DRADAR_RadarResolutionX,
    P3DRADAR_RadarResolutionY,
    P3DRADAR_GaugeResolutionX,
    P3DRADAR_GaugeResolutionY,
    P3DRADAR_CurrentRadarScanElevationDegrees,
    P3DRADAR_CurrentRadarBeamOffset,    
};
// table of property info.  Must stay lined up with enum above
static PROPERTY_TABLE P3DRADAR_PROPERTY_TABLE[] = 
{
    { "ClearRadarImage",	                "Number",   UNITS_UNKNOWN },
    { "ShowRangeRings",	                    "Number",   UNITS_UNKNOWN }, 
    { "ShowCursor",			                "Number",   UNITS_UNKNOWN },         
    { "FarShoreEnhance",	                "Number",   UNITS_UNKNOWN },         
    { "VisualZoom",			                "Number",   UNITS_UNKNOWN },         
    { "DataZoom",	                        "Number",   UNITS_UNKNOWN },
    { "ScanAzimuth",	                    "Number",   UNITS_UNKNOWN },
    { "SweepRate",	                        "Number",   UNITS_UNKNOWN },
    { "RangeMiles",	                        "Number",   UNITS_UNKNOWN },
    { "RenderingEnabled",	                "Number",   UNITS_UNKNOWN },
    { "FreezeEnabled",	                    "Number",   UNITS_UNKNOWN },
    { "CursorPositionX",	                "Number",   UNITS_UNKNOWN },
    { "CursorPositionY",	                "Number",   UNITS_UNKNOWN },
    { "CursorPositionLat",	                "Number",   UNITS_UNKNOWN },
    { "CursorPositionLon",	                "Number",   UNITS_UNKNOWN },
    { "FrontBlindSpotDegrees",	            "Number",   UNITS_UNKNOWN },
    { "SideBlindSpotDegrees",	            "Number",   UNITS_UNKNOWN },
    { "RadarResolutionX",	                "Number",   UNITS_UNKNOWN },
    { "RadarResolutionY",	                "Number",   UNITS_UNKNOWN },
    { "GaugeResolutionX",	                "Number",   UNITS_UNKNOWN },
    { "GaugeResolutionY",	                "Number",   UNITS_UNKNOWN },
    { "CurrentRadarScanElevationDegrees",   "Number",   UNITS_UNKNOWN },
    { "CurrentRadarBeamOffset",	            "Number",   UNITS_UNKNOWN },
};

///----------------------------------------------------------------------------
/// RadarPanelPallback function  Definitions
///----------------------------------------------------------------------------
DEFINE_PANEL_CALLBACK_REFCOUNT(RadarPanelCallback);

RadarPanelCallback::RadarPanelCallback( ISimulatedRadarV400 * pSimRadar )
    : m_RefCount(1), m_pRadar( pSimRadar )
{
    // init property table
    for (int n = 0; n < 5; n++)
    {
        if (ImportTable.PANELSentry.fnptr != NULL &&
            P3DRADAR_PROPERTY_TABLE[n].units == UNITS_UNKNOWN)
        {
            P3DRADAR_PROPERTY_TABLE[n].units = get_units_enum ( P3DRADAR_PROPERTY_TABLE[n].szUnitsName );
        }
    }
}

bool RadarPanelCallback::ConvertStringToProperty (PCSTRINGZ keyword, SINT32* pID)
{
    if(!keyword)
    {
        return false;
    }
    if(!pID)
    {
        return false;
    }

    UINT uNumProperties;
    const PROPERTY_TABLE *parPropertyTable = GetPropertyTable(uNumProperties);
    
    for(UINT i = 0; i < uNumProperties; i++)
    {
        if(_stricmp(parPropertyTable[i].szPropertyName, keyword) == 0)
        {
            *pID = i;
            return true;
        }
    }
    return false;         
}

bool RadarPanelCallback::ConvertPropertyToString (SINT32 id, PPCSTRINGZ pKeyword)
{
    if(!pKeyword)
    {
        return false;
    }
    
    UINT uNumProperties;
    const PROPERTY_TABLE *parPropertyTable = GetPropertyTable(uNumProperties);

    if(id < 0 || id >= (SINT32)uNumProperties)
    {
        return false;
    }
    *pKeyword = parPropertyTable[id].szPropertyName;
    return true; 
}

bool RadarPanelCallback::GetPropertyUnits (SINT32 id, ENUM* pEnum)
{
    if(!pEnum)
    {
        return false;
    }

    UINT uNumProperties;
    const PROPERTY_TABLE *parPropertyTable = GetPropertyTable(uNumProperties);
    
    if(id < 0 || id >= (SINT32)uNumProperties)
    {
        return false;
    }

    *pEnum = parPropertyTable[id].units;
    return true;
}

IAircraftCCallback * RadarPanelCallback::CreateAircraftCCallback( UINT32 ContainerID )
{
    return new RadarAircraftCallback( ContainerID, m_pRadar );
}

const PROPERTY_TABLE *RadarPanelCallback::GetPropertyTable( UINT &uLength )
{
    uLength = LENGTHOF(P3DRADAR_PROPERTY_TABLE);
    return P3DRADAR_PROPERTY_TABLE;
}

///----------------------------------------------------------------------------
/// RadarAircraftCallback Function Definitions
///----------------------------------------------------------------------------
DEFINE_PANEL_CALLBACK_REFCOUNT( RadarAircraftCallback );

RadarAircraftCallback::RadarAircraftCallback( UINT32 containerId, ISimulatedRadarV400 * pRadar )
    : m_containerId(containerId), m_RefCount(1), m_pRadar( pRadar )
{
}

IGaugeCCallback* RadarAircraftCallback::CreateGaugeCCallback()
{
    return new RadarGaugeCallback( GetContainerId(), m_pRadar );
}

///----------------------------------------------------------------------------
/// RadarGaugeCallback Function Definitions
///----------------------------------------------------------------------------
DEFINE_PANEL_CALLBACK_REFCOUNT(RadarGaugeCallback)

RadarGaugeCallback::RadarGaugeCallback( UINT32 containerId, ISimulatedRadarV400 * pSimRadar )
    : m_RefCount(1), m_containerId(containerId), m_pRadar( pSimRadar ), m_bRadarNeedsDeinit(false)
{}

 RadarGaugeCallback::~RadarGaugeCallback()
 {
     // if this gauge callback was responsible for initializing the radar then
     // it should deinitialize it.
     if(m_pRadar && m_pRadar->IsInitialized() && m_bRadarNeedsDeinit)
     {
         m_pRadar->DeInit();
     }
 }

//
// Getting float/numeric values
//
bool RadarGaugeCallback::GetPropertyValue(SINT32 id, FLOAT64* pValue)
{
    if(!pValue || ! m_pRadar )
    {
       return false;
    }

    *pValue = 1.0;      // Start with a reasonable default
   
    P3DRADAR_VAR eP3DRADARVar = (P3DRADAR_VAR)id;
    double x, y;
    LLA lla;
    switch(eP3DRADARVar)
    {
    case P3DRADAR_ShowRangeRings:
        *pValue = m_pRadar->ShowRangeRings();
        break;
    case P3DRADAR_ShowCursor:
        *pValue = m_pRadar->ShowCursor();
        break;
    case P3DRADAR_FarShoreEnhance:
        *pValue = m_pRadar->FarShoreEnhance();
        break;
    case P3DRADAR_VisualZoom:
        *pValue = m_pRadar->GetVisualZoom();
        break;
    case P3DRADAR_DataZoom:
        *pValue = m_pRadar->GetDataZoom();
        break;
    case P3DRADAR_ScanAzimuth:
        *pValue = m_pRadar->GetScanAzimuth();
        break;
    case P3DRADAR_SweepRate:
        *pValue = m_pRadar->GetSweepRate();
        break;
    case P3DRADAR_RangeMiles:
        *pValue = m_pRadar->GetRangeMiles();
        break;
    case P3DRADAR_RenderingEnabled:
        *pValue = m_pRadar->RenderingEnabled();
        break;
    case P3DRADAR_CursorPositionX:
        m_pRadar->GetCursorPositionXY( x, y );
        *pValue = x;
        break;
    case P3DRADAR_CursorPositionY:
        m_pRadar->GetCursorPositionXY( x, y );
        *pValue = y;
        break;
    case P3DRADAR_CursorPositionLat:
        m_pRadar->GetCursorPositionLLA( lla );
        *pValue = lla.Lat;
        break;
    case P3DRADAR_CursorPositionLon:
        m_pRadar->GetCursorPositionLLA( lla );
        *pValue = lla.Lon;
        break;
    case P3DRADAR_FreezeEnabled:
        if(m_pRadar->FreezeEnabled())
        {
            *pValue = 1.0;
        }
        else
        {
            *pValue = 0.0;
        }
        break;
    case P3DRADAR_FrontBlindSpotDegrees:
        *pValue = m_pRadar->GetFrontBlindspotDegrees();
        break;
    case P3DRADAR_SideBlindSpotDegrees:
        *pValue = m_pRadar->GetSideBlindspotDegrees();
        break;
    case P3DRADAR_RadarResolutionX:
        m_pRadar->GetRadarResolution( x, y );
        *pValue = x;
        break;
    case P3DRADAR_RadarResolutionY:
        m_pRadar->GetRadarResolution( x, y );
        *pValue = y;
        break;
    case P3DRADAR_GaugeResolutionX:
        m_pRadar->GetGaugeResolution( x, y );
        *pValue = x;
        break;
    case P3DRADAR_GaugeResolutionY:
        m_pRadar->GetGaugeResolution( x, y );
        *pValue = y;
        break;
    case P3DRADAR_CurrentRadarScanElevationDegrees:
        *pValue = m_pRadar->GetCurrentRadarScanElevationDegrees();
        break;
    case P3DRADAR_CurrentRadarBeamOffset:
        *pValue = m_pRadar->GetCurrentRadarBeamOffsetDegrees();
        break;
    default:
        return false;
    }
    return true; 
}

//
// Setting float/numeric values
//
bool RadarGaugeCallback::SetPropertyValue( SINT32 id, FLOAT64 value )
{
    P3DRADAR_VAR eP3DRADARVar = (P3DRADAR_VAR)id;
    double x, y;
    LLA lla;
    // If radar is being used for the first time initialize it.
    // Another plugin or Prepar3D's internal version of this callback 
    // may have already initialized the radar service so check first
    if(!m_pRadar->IsInitialized())
    {
        // Could do this in the constructor but this will prevent
        // the initialization from happening unless it is actually used
        // (Prepar3D's internal callback does this too so as to not step
        // on the toes of other plugins such as this one.) More than
        // one plugin can interface with the radar system but there
        // is only one instance to share so it should be avoided if possible.
        //
        // If another plugin gets here first then the texture name will be different
        // and your example gauge may stop displaying.  It will still be controlling 
        // the radar though.
        m_pRadar->Init(TEXT("P3DRadarExampleTexture"),256,256);
        m_bRadarNeedsDeinit = true;
    }
    switch(eP3DRADARVar)
    {
    case P3DRADAR_ClearRadarImage:
        m_pRadar->ClearRadarImage();
        break;
    case P3DRADAR_ShowRangeRings:
        if(value >= 1.0)
        {
            m_pRadar->SetShowRangeRings( true );
        }
        else
        {
            m_pRadar->SetShowRangeRings( false );
        }
        break;
    case P3DRADAR_ShowCursor:
        if(value >= 1.0)
        {
            m_pRadar->SetShowCursor( true );
        }
        else
        {
            m_pRadar->SetShowCursor( false );
        }
        break;
    case P3DRADAR_FarShoreEnhance:
        if(value >= 1.0)
        {
            m_pRadar->SetFarShoreEnhancementEnabled( true );
        }
        else
        {
            m_pRadar->SetFarShoreEnhancementEnabled( false );
        }
        break;
    case P3DRADAR_VisualZoom:
        m_pRadar->SetVisualZoom( value );
        break;
    case P3DRADAR_DataZoom:
        m_pRadar->SetDataZoom( value );
        break;
    case P3DRADAR_ScanAzimuth:
        m_pRadar->SetScanAzimuthDegrees( value );
        break;
    case P3DRADAR_SweepRate:
        m_pRadar->SetScanRateDegreesPerSecond( value );
        break;
    case P3DRADAR_RangeMiles:
        m_pRadar->SetRangeMiles( value );
        break;
    case P3DRADAR_RenderingEnabled:
        if( value >= 1.0 )
        {
            m_pRadar->SetRenderingEnabled( true );
        }
        else
        {
            m_pRadar->SetRenderingEnabled( false );
        }
        break;
    case P3DRADAR_FreezeEnabled:
        if( value >= 1.0 )
        {
            m_pRadar->SetFreeze( true );
        }
        else
        {
            m_pRadar->SetFreeze( false );
        }
        break;
    case P3DRADAR_CursorPositionX:
        m_pRadar->GetCursorPositionXY( x, y );
        x = value;
        m_pRadar->SetCursorPositionXY( x, y );
        break;
    case P3DRADAR_CursorPositionY:
        m_pRadar->GetCursorPositionXY( x, y );
        y = value;
        m_pRadar->SetCursorPositionXY( x, y );
        break;
    case P3DRADAR_CursorPositionLat:
        m_pRadar->GetCursorPositionLLA( lla );
        lla.Lat = value;
        m_pRadar->SetCursorPositionLLA( lla );
        break;
    case P3DRADAR_CursorPositionLon:
        m_pRadar->GetCursorPositionLLA( lla );
        lla.Lon = value;
        m_pRadar->SetCursorPositionLLA( lla );
        break;
    case P3DRADAR_FrontBlindSpotDegrees:
        m_pRadar->SetFrontBlindSpotDegrees( value );
        break;
    case P3DRADAR_SideBlindSpotDegrees:
        m_pRadar->SetSideBlindSpotDegrees( value );
        break;
    case P3DRADAR_RadarResolutionX:
        m_pRadar->GetRadarResolution( x, y );
        x = value;
        m_pRadar->SetRadarImageResolution( x, y );
        break;
    case P3DRADAR_RadarResolutionY:
        m_pRadar->GetRadarResolution( x, y );
        y = value;
        m_pRadar->SetRadarImageResolution( x, y );
        break;
    case P3DRADAR_GaugeResolutionX:
        m_pRadar->GetGaugeResolution( x, y );
        x = value;
        m_pRadar->SetRadarGaugeResolution( x, y );
        break;
    case P3DRADAR_GaugeResolutionY:
        m_pRadar->GetGaugeResolution( x, y );
        y = value;
        m_pRadar->SetRadarGaugeResolution( x, y );
        break;
    default:
        return false;
    }
    return true; 
}

IGaugeCCallback* RadarGaugeCallback::QueryInterface(LPCSTR pszInterface)
{
    if( strncmp( pszInterface, ISERIALIZABLE_GAUGECCALLBACK_NAME, strlen( ISERIALIZABLE_GAUGECCALLBACK_NAME ) ) == 0 )
    {
        return this;
    }
    return NULL;
}

bool RadarGaugeCallback::Serialize(NetOutPublic& netout)
{
    // Only serialize if the callback has actually used the radar and explicitly initialized it
    // this is just a safe guard in case other plug-ins use the radar.  Future versions of the radar
    // may support multiple instances but for now there is one instance.
    if(m_bRadarNeedsDeinit)
    {
        RadarGaugeSerializationData data;
        data.ShowRangeRings = m_pRadar->ShowRangeRings();
        data.ShowCursor = m_pRadar->ShowCursor();
        data.FarShoreEnhance = m_pRadar->FarShoreEnhance();
        data.VisualZoom = m_pRadar->GetVisualZoom();
        data.DataZoom = m_pRadar->GetDataZoom();
        data.ScanAzimuth = m_pRadar->GetScanAzimuth();
        data.SweepRate = m_pRadar->GetSweepRate();
        data.RangeMiles = m_pRadar->GetRangeMiles();
        data.RenderingEnabled = m_pRadar->RenderingEnabled();
        m_pRadar->GetCursorPositionXY( data.CursorPositionX, data.CursorPositionY );
        data.FreezeEnabled = m_pRadar->FreezeEnabled();
        data.FrontBlindspotDegrees = m_pRadar->GetFrontBlindspotDegrees();
        data.SideBlindspotDegrees = m_pRadar->GetSideBlindspotDegrees();
        m_pRadar->GetRadarResolution( data.RadarResolutionX, data.RadarResolutionY );
        m_pRadar->GetGaugeResolution(data.GaugeResolutionX ,data.GaugeResolutionY  );
        //Write the entire struct out to the network packet.
        netout.WriteData(&data,sizeof(RadarGaugeSerializationData));
    }
    return true;
}

bool RadarGaugeCallback::Deserialize(NetInPublic& netin)
{
    const RadarGaugeSerializationData* pData;
    pData = (RadarGaugeSerializationData*) netin.GetCurBuffer();
    // If this plugin serialized data a valid header should be at the current read location
    if(pData->Header.IsValid())
    {
        // found a valid header as expected so go ahead and read the size specified in the header 
        // so that the netin's current position is correct when returned to the caller
        pData = (RadarGaugeSerializationData*) netin.Read(pData->Header.SizeInBytes);
        // Only deserialize if the version and size matches.
        if(pData->Header.IsCurrentVersion() && pData->Header.ValidateSize())
        {
            // if the radar service has not been initialized then initialize it now.
            if(!m_pRadar->IsInitialized())
            {
                m_pRadar->Init(TEXT("P3DRadarExampleTexture"),256,256);
                m_bRadarNeedsDeinit = true;
            }
            // set all the serialized data into the radar
            m_pRadar->SetShowRangeRings(pData->ShowRangeRings );
            m_pRadar->SetShowCursor(pData->ShowCursor);
            m_pRadar->SetFarShoreEnhancementEnabled(pData->FarShoreEnhance);
            m_pRadar->SetVisualZoom(pData->VisualZoom);
            m_pRadar->SetDataZoom(pData->DataZoom);
            m_pRadar->SetScanAzimuthDegrees(pData->ScanAzimuth);
            m_pRadar->SetScanRateDegreesPerSecond(pData->SweepRate);
            m_pRadar->SetRangeMiles(pData->RangeMiles);
            m_pRadar->SetRenderingEnabled(pData->RenderingEnabled );
            m_pRadar->SetCursorPositionXY( pData->CursorPositionX, pData->CursorPositionY );
            m_pRadar->SetFreeze(pData->FreezeEnabled);
            m_pRadar->SetFrontBlindSpotDegrees(pData->FrontBlindspotDegrees);
            m_pRadar->SetSideBlindSpotDegrees(pData->SideBlindspotDegrees);
            m_pRadar->SetRadarGaugeResolution(pData->RadarResolutionX, pData->RadarResolutionY);
            m_pRadar->SetRadarGaugeResolution(pData->GaugeResolutionX, pData->GaugeResolutionY);     
        }
    }
    return true;
}

// The Panels pointer will get filled in during the loading process
// if this DLL is listed in DLL.XML
//  
//  entry in dll.xml should look like this where PATH_TO_DLL is local to the Prepar3d install directory
//  or is an absolute path:
//
//  <Launch.Addon>
//      <Name>RadarSample</Name>
//      <DLLType>PDK</DLLType>
//      <Disabled>False</Disabled>
//      <ManualLoad>False</ManualLoad>
//      <Path>PATH_TO_DLL\RadarPanelCallback.dll</Path>
//  </Launch.Addon>
//

PPANELS Panels = NULL;

static const char RADAR_EXAMPLE_CALLBACK_NAME[] = "P3DRadarExample";

GAUGESIMPORT    ImportTable =                           
{                                                       
    { 0x0000000F, (PPANELS)NULL },                     
    { 0x00000000, NULL }                                
};                                                      

BOOL WINAPI DllMain (HINSTANCE hDLL, DWORD dwReason, LPVOID lpReserved) 
{                                                       
    return TRUE;                                        
}                  
 
extern "C" 
{
    void __stdcall DLLStart( __in __notnull P3D::IPdk* pPdk )
    {  
        if( pPdk != nullptr && Panels != nullptr)
        {
            // Check for radar interface
            ISimulatedRadarV400* pRadar;
            pPdk->QueryService( Radar::SID_SimulatedRadar, &pRadar );
            if( pRadar != nullptr )
            {
                // create panel callback passing in the radar service pointer
                RadarPanelCallback * pPanelCallback = new RadarPanelCallback( pRadar );
                // hook up the panels table for good measure even though its not being used 
                ImportTable.PANELSentry.fnptr = (PPANELS)Panels;
                // register the panel callback by name for mixed mode C: variables 
	            panel_register_c_callback( RADAR_EXAMPLE_CALLBACK_NAME, pPanelCallback);
                // clean up local ref counted resources
                pPanelCallback->Release();
                pRadar->Release();
            }
        }
        return;
    }

    void __stdcall DLLStop( void )
    {
        // unregister the callback
        panel_register_c_callback(RADAR_EXAMPLE_CALLBACK_NAME, nullptr);
    }
}

// This is the module's export table.
GAUGESLINKAGE   Linkage =                               
{                                                       
    0x00000013,                                         
    0,                                       
    0,                                      
    0,                                                  
    0,                        

    FS9LINK_VERSION, { 0 }
};