/**********
This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 3 of the License, or (at your
option) any later version. (See <http://www.gnu.org/copyleft/lesser.html>.)

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
**********/
// Copyright (c) 1996-2020, Live Networks, Inc.  All rights reserved
// A test program that demonstrates how to stream - via unicast RTP
// - various kinds of file on demand, using a built-in RTSP server.
// main program

#include <liveMedia.hh>
#include <BasicUsageEnvironment.hh>
#include "H264FramedLiveSource.hh"

UsageEnvironment *env;

// To make the second and subsequent client for each stream reuse the same
// input stream as the first client (rather than playing the file from the
// start for each client), change the following "False" to "True":
Boolean reuseFirstSource = False;

// To stream *only* MPEG-1 or 2 video "I" frames
// (e.g., to reduce network bandwidth),
// change the following "False" to "True":
Boolean iFramesOnly = False;

static void announceStream(RTSPServer *rtspServer, ServerMediaSession *sms,
                           char const *streamName, char const *inputFileName); // fwd

static char newDemuxWatchVariable;

static MatroskaFileServerDemux *matroskaDemux;
static void onMatroskaDemuxCreation(MatroskaFileServerDemux *newDemux, void * /*clientData*/)
{
  matroskaDemux = newDemux;
  newDemuxWatchVariable = 1;
}

static OggFileServerDemux *oggDemux;
static void onOggDemuxCreation(OggFileServerDemux *newDemux, void * /*clientData*/)
{
  oggDemux = newDemux;
  newDemuxWatchVariable = 1;
}

int main(int argc, char **argv)
{
  // Begin by setting up our usage environment:
  TaskScheduler *scheduler = BasicTaskScheduler::createNew();
  env = BasicUsageEnvironment::createNew(*scheduler);

  UserAuthenticationDatabase *authDB = NULL;
#ifdef ACCESS_CONTROL
  // To implement client access control to the RTSP server, do the following:
  authDB = new UserAuthenticationDatabase;
  authDB->addUserRecord("username1", "password1"); // replace these with real strings
  // Repeat the above with each <username>, <password> that you wish to allow
  // access to the server.
#endif

  // Create the RTSP server:
  RTSPServer *rtspServer = RTSPServer::createNew(*env, 8554, authDB);
  if (rtspServer == NULL)
  {
    *env << "Failed to create RTSP server: " << env->getResultMsg() << "\n";
    exit(1);
  }

  char const *descriptionString = "Session streamed by \"testOnDemandRTSPServer\"";

  // Set up each of the possible streams that can be served by the
  // RTSP server.  Each such stream is implemented using a
  // "ServerMediaSession" object, plus one or more
  // "ServerMediaSubsession" objects for each audio/video substream.

  { //这里是自己的ServerMediaSubssion

    //这个是输出缓冲区的大小，要设置到比任意一帧h264 大
    OutPacketBuffer::maxSize = 2000000;
    char const *streamName = "h264Live";
    char const *devicename = "/dev/video0";
    ServerMediaSession *sms = ServerMediaSession::createNew(*env, streamName, streamName,
                                                            descriptionString);
    sms->addSubsession(H264LiveVideoServerMediaSubssion ::createNew(*env, reuseFirstSource, devicename, 640, 480));
    rtspServer->addServerMediaSession(sms);

    announceStream(rtspServer, sms, streamName, devicename);
  }
  // Also, attempt to create a HTTP server for RTSP-over-HTTP tunneling.
  // Try first with the default HTTP port (80), and then with the alternative HTTP
  // port numbers (8000 and 8080).

  if (rtspServer->setUpTunnelingOverHTTP(80) || rtspServer->setUpTunnelingOverHTTP(8000) || rtspServer->setUpTunnelingOverHTTP(8080))
  {
    *env << "\n(We use port " << rtspServer->httpServerPortNum() << " for optional RTSP-over-HTTP tunneling.)\n";
  }
  else
  {
    *env << "\n(RTSP-over-HTTP tunneling is not available.)\n";
  }

  env->taskScheduler().doEventLoop(); // does not return

  return 0; // only to prevent compiler warning
}

static void announceStream(RTSPServer *rtspServer, ServerMediaSession *sms,
                           char const *streamName, char const *inputFileName)
{
  char *url = rtspServer->rtspURL(sms);
  UsageEnvironment &env = rtspServer->envir();
  env << "\n\"" << streamName << "\" stream, from the file \""
      << inputFileName << "\"\n";
  env << "Play this stream using the URL \"" << url << "\"\n";
  delete[] url;
}
