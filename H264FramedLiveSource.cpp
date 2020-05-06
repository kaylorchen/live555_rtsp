
// #include "H264LiveVideoServerMediaSubssion.hh"
#include "H264FramedLiveSource.hh"
#include "H264VideoStreamFramer.hh"
#include "H264VideoStreamDiscreteFramer.hh"
#include "H264VideoRTPSink.hh"


H264LiveVideoServerMediaSubssion *H264LiveVideoServerMediaSubssion::createNew(UsageEnvironment &env, Boolean reuseFirstSource, char const *devicename , int width, int height)
{
    /*静态函数不能直接给类的成员赋值，所以需要通过实例化类的时候将数值传入*/
    return new H264LiveVideoServerMediaSubssion(env, reuseFirstSource, devicename, width, height); 
}

H264LiveVideoServerMediaSubssion::H264LiveVideoServerMediaSubssion(UsageEnvironment &env, Boolean reuseFirstSource, char const *devicename , int width, int height)
    : OnDemandServerMediaSubsession(env, reuseFirstSource)
{
    /*实例化的时候将传入的参数用来初始化相应的摄像头*/
    Camera.Init(devicename, width, height);
}

H264LiveVideoServerMediaSubssion::~H264LiveVideoServerMediaSubssion()
{
    Camera.Destory();
}

FramedSource *H264LiveVideoServerMediaSubssion::createNewStreamSource(unsigned clientSessionId, unsigned &estBitrate)
{
    //创建视频源,参照H264VideoFileServerMediaSubsession
    H264FramedLiveSource *liveSource = H264FramedLiveSource::createNew(envir(), &Camera);
    if (liveSource == NULL)
    {
        return NULL;
    }
    // Create a framer for the Video Elementary Stream:
    //  return H264VideoStreamFramer::createNew(envir(), liveSource);
    return H264VideoStreamDiscreteFramer::createNew(envir(), liveSource);
}

RTPSink *H264LiveVideoServerMediaSubssion ::createNewRTPSink(Groupsock *rtpGroupsock,
                                                             unsigned char rtpPayloadTypeIfDynamic,
                                                             FramedSource * /*inputSource*/)
{
    return H264VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);
}

H264FramedLiveSource::H264FramedLiveSource(UsageEnvironment &env, Device *cam)
    : FramedSource(env)
{
    pCamera = cam;
}

H264FramedLiveSource *H264FramedLiveSource::createNew(UsageEnvironment &env, Device *cam)
{
    H264FramedLiveSource *newSource = new H264FramedLiveSource(env, cam);
    return newSource;
}

H264FramedLiveSource::~H264FramedLiveSource()
{
}
unsigned H264FramedLiveSource::maxFrameSize() const
{
    //printf("wangmaxframesize %d %s\n",__LINE__,__FUNCTION__);
    //这里返回本地h264帧数据的最大长度
    return 1024 * 120;
}

void H264FramedLiveSource::doGetNextFrame()
{
    pCamera->getnextframe(fFrameSize, fTo);
    // printf("LINE: %d %s fMaxSize %d ,fFrameSize %d  \n", __LINE__, __FUNCTION__, fMaxSize, fFrameSize);

    if (fFrameSize == 0)
    {
        handleClosure();
        return;
    }

    //设置时间戳
    gettimeofday(&fPresentationTime, NULL);

    // Inform the reader that he has data:
    // To avoid possible infinite recursion, we need to return to the event loop to do this:
    nextTask() = envir().taskScheduler().scheduleDelayedTask(0,
                                                             (TaskFunc *)FramedSource::afterGetting, this);
}
