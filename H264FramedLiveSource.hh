
#ifndef _H264_LIVE_VIDEO_SERVER_MEDIA_SUBSESSION_HH

#define _H264_LIVE_VIDEO_SERVER_MEDIA_SUBSESSION_HH

#include "OnDemandServerMediaSubsession.hh"
#include "camera.hh"


class H264LiveVideoServerMediaSubssion : public OnDemandServerMediaSubsession
{

public:
    static H264LiveVideoServerMediaSubssion *createNew(UsageEnvironment &env, Boolean reuseFirstSource, char const *devicename , int width, int height);

protected:
    H264LiveVideoServerMediaSubssion(UsageEnvironment &env, Boolean reuseFirstSource, char const *devicename , int width, int height);
    ~H264LiveVideoServerMediaSubssion();

protected: // redefined virtual functions
    FramedSource *createNewStreamSource(unsigned clientSessionId, unsigned &estBitrate);
    RTPSink *createNewRTPSink(Groupsock *rtpGroupsock,
                              unsigned char rtpPayloadTypeIfDynamic,
                              FramedSource *inputSource);

public:
    class Device Camera;
};
class H264FramedLiveSource : public FramedSource
{

public:
    static H264FramedLiveSource *createNew(UsageEnvironment &env, Device *cam);
    // redefined virtual functions
    virtual unsigned maxFrameSize() const;

    class Device *pCamera;

protected:
    H264FramedLiveSource(UsageEnvironment &env, Device *cam);

    virtual ~H264FramedLiveSource();

private:
    virtual void doGetNextFrame();

protected:
    // static TestFromFile *pTest;
};
#endif