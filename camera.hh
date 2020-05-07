
#ifndef _CAMERA_HH
#define _CAMERA_HH
#include <FramedSource.hh>
#include <UsageEnvironment.hh>
#include "encoder_define.hh"

class Device
{
public:
    void init_mmap(void);
    void init_camera(void);
    void init_encoder(void);
    void open_camera(const char *devicename);
    
    void close_camera(void);
    void read_one_frame(unsigned int &framesize, unsigned char *fTo);
	void getnextframe(unsigned int &framesize, unsigned char *fTo);
    void start_capture(void);
    void stop_capture(void);
    void close_encoder(); 
    int  camera_able_read(void);
    void compress_begin(Encoder *en, int width, int height);
    int  compress_frame(Encoder *en, int type, char *in, int len, char *out);
    void compress_end(Encoder *en);
    void Init(const char *devicename, int width, int height);
	void intoloop();
    void Destory();
    ~Device();
public:
    int fd;
	FILE *save_fd;
    int n_nal;
    int frame_len;
	char *h264_buf;
	unsigned int n_buffer;
	Encoder en;
	FILE *h264_fp;
    BUFTYPE *usr_buf;
	FILE *pipe_fd;
    private:
    int mHeight;
    int mWidth;
    const char *mDevicename;
    struct timeval last_time = {0 ,0}; 
};
#endif
