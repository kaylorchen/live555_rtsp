cmake_minimum_required(VERSION 3.5)
project(live555_rtsp)

set(CMAKE_CXX_STANDARD 11)
set( CMAKE_VERBOSE_MAKEFILE on )
set(CMAKE_EXPORT_COMPILE_COMMANDS on)
set( LINKER_FLAGS "${LINKER_FLAGS} -g -O2 -W -Wall" )
set(COMMON_FLAGS "-O2 -W -Wall")

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMMON_FLAGS} -m128bit-long-double -m64 -m80387 -mabi=sysv \
-maccumulate-outgoing-args -maddress-mode=long -maes -malign-data=compat\
 -malign-functions=0 -malign-jumps=0 -malign-loops=0 -malign-stringops -march=silvermont\
 -masm=att -mbranch-cost=3 -mcx16 -mfancy-math-387 -mfp-ret-in-387\
 -mfpmath=sse -mfunction-return=keep -mfxsr -mglibc -mhard-float \
 -mieee-fp -mindirect-branch=keep \
 -mlarge-data-threshold=65536 -mlong-double-80 -mmmx -mmovbe \
 -mpclmul -mpopcnt -mpreferred-stack-boundary=4 -mprfchw \
 -mpush-args -mrdrnd -mred-zone -mregparm=6 -msahf -msse -msse2 \
 -msse3 -msse4 -msse4.1 -msse4.2 -mssse3 -mstack-protector-guard=tls \
 -mstv -mtls-dialect=gnu -mtls-direct-seg-refs -mtune=silvermont \
 -mvzeroupper" )
 set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMMON_FLAGS} -m128bit-long-double -m64 -m80387 -mabi=sysv \
-maccumulate-outgoing-args -maddress-mode=long -maes -malign-data=compat\
 -malign-functions=0 -malign-jumps=0 -malign-loops=0 -malign-stringops -march=silvermont\
 -masm=att -mbranch-cost=3 -mcx16 -mfancy-math-387 -mfp-ret-in-387\
 -mfpmath=sse -mfunction-return=keep -mfxsr -mglibc -mhard-float \
 -mieee-fp -mindirect-branch=keep \
 -mlarge-data-threshold=65536 -mlong-double-80 -mmmx -mmovbe \
 -mpclmul -mpopcnt -mpreferred-stack-boundary=4 -mprfchw \
 -mpush-args -mrdrnd -mred-zone -mregparm=6 -msahf -msse -msse2 \
 -msse3 -msse4 -msse4.1 -msse4.2 -mssse3 -mstack-protector-guard=tls \
 -mstv -mtls-dialect=gnu -mtls-direct-seg-refs -mtune=silvermont \
 -mvzeroupper" )

include_directories(
    /usr/local/include/liveMedia/
    /usr/local/include/groupsock/
    /usr/local/include/UsageEnvironment/
    /usr/local/include/BasicUsageEnvironment
    ./
)
set(CMAKE_BUILD_TYPE Release)

add_executable(OnDemandRTSPServer OnDemandRTSPServer.cpp H264FramedLiveSource.cpp camera.cpp)
set(LIVE_LIBRARIES liveMedia groupsock  BasicUsageEnvironment UsageEnvironment)
target_link_libraries(OnDemandRTSPServer ${LIVE_LIBRARIES} -lssl -lcrypto -lx264)





