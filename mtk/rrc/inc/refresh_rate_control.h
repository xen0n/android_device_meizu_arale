#ifndef __REFRESH_RATE_CONTROL_H__
#define __REFRESH_RATE_CONTROL_H__


/*-----------------------------------------------------------------------------
 Refresh Rate Control Scenario
 -----------------------------------------------------------------------------*/
typedef enum {

    RRC_TYPE_NONE                      = 0,
    RRC_TYPE_VIDEO_NORMAL              ,
    RRC_TYPE_CAMERA_PREVIEW            ,
    RRC_TYPE_CAMERA_ZSD                ,
    RRC_TYPE_CAMERA_CAPTURE            ,
    RRC_TYPE_CAMERA_ICFP               ,
    RRC_TYPE_VIDEO_SWDEC_PLAYBACK      ,
    RRC_TYPE_VIDEO_PLAYBACK            ,
    RRC_TYPE_VIDEO_TELEPHONY           ,
    RRC_TYPE_VIDEO_RECORD              ,
    RRC_TYPE_VIDEO_RECORD_CAMERA       ,
    RRC_TYPE_VIDEO_RECORD_SLOWMOTION   ,
    RRC_TYPE_VIDEO_SNAPSHOT            ,
    RRC_TYPE_VIDEO_LIVE_PHOTO          ,
    RRC_TYPE_VIDEO_WIFI_DISPLAY        ,

    /* touch event */
    RRC_TYPE_TOUCH_EVENT               ,
    RRC_TYPE_VIDEO_120HZ               ,

    RRC_TYPE_MAX_SIZE


} RRC_SCENARIO_TYPE;


typedef enum {

    RRC_60Hz = 0,
    RRC_120Hz ,


} RRC_REFRESH_RATE;



class RefreshRateControl {


public:

     RefreshRateControl();
     ~RefreshRateControl();

     int setScenario(int scenario, bool enable);

     int getRefreshRate();

private:

     int getScenario();

     int setRefreshRate(int refresh);

};

#endif
