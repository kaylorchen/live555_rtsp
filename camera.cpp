/*
*  H264FramedLiveSource.cpp
*/
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <linux/types.h>
#include <linux/videodev2.h>
#include <malloc.h>
#include <math.h>
#include <string.h>
#include <sys/mman.h>
#include <errno.h>
#include <assert.h>
#include "encoder_define.hh"
#include "H264FramedLiveSource.hh"
#include <BasicUsageEnvironment.hh>
#include <iostream>

void Device::init_mmap(void)
{
	struct v4l2_requestbuffers reqbufs;
	memset(&reqbufs, 0, sizeof(reqbufs));
	reqbufs.count = 4;
	reqbufs.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	reqbufs.memory = V4L2_MEMORY_MMAP;

	if (-1 == ioctl(fd, VIDIOC_REQBUFS, &reqbufs))
	{
		perror("Fail to ioctl 'VIDIOC_REQBUFS'");
		exit(EXIT_FAILURE);
	}

	n_buffer = reqbufs.count;
	printf("n_buffer = %d\n", n_buffer);
	usr_buf = (BUFTYPE *)calloc(reqbufs.count, sizeof(BUFTYPE));
	if (usr_buf == NULL)
	{
		printf("Out of memory\n");
		exit(-1);
	}

	for (n_buffer = 0; n_buffer < reqbufs.count; ++n_buffer)
	{

		struct v4l2_buffer buf;
		memset(&buf, 0, sizeof(buf));
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = n_buffer;

		if (-1 == ioctl(fd, VIDIOC_QUERYBUF, &buf))
		{
			perror("Fail to ioctl : VIDIOC_QUERYBUF");
			exit(EXIT_FAILURE);
		}

		usr_buf[n_buffer].length = buf.length;
		usr_buf[n_buffer].start = (char *)mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);

		if (MAP_FAILED == usr_buf[n_buffer].start)
		{
			perror("Fail to mmap");
			exit(EXIT_FAILURE);
		}
	}
}

void Device::open_camera(const char *devicename)
{
	struct v4l2_input inp;

	fd = open(devicename, O_RDWR | O_NONBLOCK, 0);
	if (fd < 0)
	{
		fprintf(stderr, "%s open err \n", devicename);
		exit(EXIT_FAILURE);
	};

	inp.index = 0;
	if (-1 == ioctl(fd, VIDIOC_S_INPUT, &inp))
	{
		fprintf(stderr, "VIDIOC_S_INPUT \n");
	}
}

void Device::init_camera(void)
{
	struct v4l2_capability cap;
	struct v4l2_format tv_fmt;
	struct v4l2_fmtdesc fmtdesc;
	int ret;

	memset(&fmtdesc, 0, sizeof(fmtdesc));
	fmtdesc.index = 0;
	fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	ret = ioctl(fd, VIDIOC_QUERYCAP, &cap);
	if (ret < 0)
	{
		fprintf(stderr, "fail to ioctl VIDEO_QUERYCAP \n");
		exit(EXIT_FAILURE);
	}

	if (!(cap.capabilities & V4L2_BUF_TYPE_VIDEO_CAPTURE))
	{
		fprintf(stderr, "The Current device is not a video capture device \n");
		exit(EXIT_FAILURE);
	}

	if (!(cap.capabilities & V4L2_CAP_STREAMING))
	{
		printf("The Current device does not support streaming i/o\n");
		exit(EXIT_FAILURE);
	}

	printf("\ncamera driver name is : %s\n", cap.driver);
	printf("camera device name is : %s\n", cap.card);
	printf("camera bus information: %s\n", cap.bus_info);

	struct v4l2_input input;
	uint32_t current_input;
	if (0 != ioctl(fd, VIDIOC_G_INPUT, &current_input))
	{
		perror("VIDIOC_G_INPUT failed");
	}
	memset(&input, 0, sizeof(input));
	input.index = current_input;
	if (0 == ioctl(fd, VIDIOC_ENUMINPUT, &input))
	{
		std::cout << "\tindex: " << std::dec << input.index
				  << "\n\tname: " << (char *)input.name
				  << "\n\ttype: " << std::dec << input.type
				  << "\n\taudioset: 0x" << std::hex << input.audioset
				  << "\n\ttuner: " << std::dec << input.tuner
				  << "\n\tstd: 0x" << std::hex << input.std
				  << "\n\tstatus: 0x" << std::hex << input.status << std::endl;
	}
	else
	{
		perror("VIDIOC_ENUMINPUT failed");
	}

	// *Video Standard*
	// here we pick a video standard (NTSC, PAL, etc...)
	struct v4l2_standard standard;
	v4l2_std_id std_id = V4L2_STD_NTSC;
	// try set the video standard
	if (-1 == ioctl(fd, VIDIOC_S_STD, &std_id))
	{
		perror("VIDIOC_S_STD failed");
	}
	// query current video standard
	// TO DO:
	// query failed --> Invalid argument (EINVAL) --> This ioctl is not
	// supported, or the VIDIOC_S_STD parameter was unsuitable
	if (-1 == ioctl(fd, VIDIOC_G_STD, &std_id))
	{
		perror("VIDIOC_G_STD failed");
	}
	else
	{
		std::cout << "Video Standard for camera std_id: 0x" << std::hex << std_id;
	}
	// enumerate the supported video standards, and highlight the one we just
	// set above
	// TO DO:
	memset(&standard, 0, sizeof(standard));
	standard.index = 0;
	while (0 == ioctl(fd, VIDIOC_ENUMSTD, &standard))
	{
		if (standard.id & std_id)
			printf("\t%s (current)\n", standard.name);
		else
			printf("\t%s\n", standard.name);
		standard.index++;
	}

	tv_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	tv_fmt.fmt.pix.width = mWidth;
	tv_fmt.fmt.pix.height = mHeight;
	tv_fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
	tv_fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
	if (ioctl(fd, VIDIOC_S_FMT, &tv_fmt) < 0)
	{
		perror("VIDIOC_S_FMT set err");
		exit(-1);
		close(fd);
	}

	printf("Configure framerate\n");
	struct v4l2_streamparm parm;

	memset(&parm, 0, sizeof(parm));
	parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	ioctl(fd, VIDIOC_G_PARM, &parm);
	printf("   Original: numerator = %d, denominator = %d, type = %d\n", parm.parm.capture.timeperframe.numerator,
		   parm.parm.capture.timeperframe.denominator, parm.type);
	parm.parm.capture.timeperframe.numerator = 1;
	parm.parm.capture.timeperframe.denominator = 15;

	printf("Expectation: numerator = %d, denominator = %d\n", parm.parm.capture.timeperframe.numerator, parm.parm.capture.timeperframe.denominator);

	if (0 == ioctl(fd, VIDIOC_S_PARM, &parm))
	{
		printf("Configure camera framerate successful!!\n");
	}
	ioctl(fd, VIDIOC_G_PARM, &parm);
	printf("     Actual: numerator = %d, denominator = %d\n", parm.parm.capture.timeperframe.numerator, parm.parm.capture.timeperframe.denominator);
}

void Device::start_capture(void)
{
	unsigned int i;
	enum v4l2_buf_type type;

	for (i = 0; i < n_buffer; i++)
	{
		struct v4l2_buffer buf;
		memset(&buf, 0, sizeof(buf));
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = i;

		if (-1 == ioctl(fd, VIDIOC_QBUF, &buf))
		{
			perror("Fail to ioctl 'VIDIOC_QBUF'");
			exit(EXIT_FAILURE);
		}
	}

	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (-1 == ioctl(fd, VIDIOC_STREAMON, &type))
	{
		printf("i=%d.\n", i);
		perror("VIDIOC_STREAMON");
		close(fd);
		exit(EXIT_FAILURE);
	}
}

void Device::read_one_frame(unsigned int &framesize, unsigned char *fTo)
{
	struct v4l2_buffer buf;

	memset(&buf, 0, sizeof(buf));
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;

	if (-1 == ioctl(fd, VIDIOC_DQBUF, &buf))
	{
		perror("Fail to ioctl 'VIDIOC_DQBUF'");
		exit(EXIT_FAILURE);
	}
	assert(buf.index < n_buffer);

	//encode_frame(usr_buf[buf.index].start, usr_buf[buf.index].length);
	struct timeval tv;
	float duration;
	gettimeofday(&tv, NULL);
	duration = (tv.tv_usec + tv.tv_sec*1000000 - last_time.tv_usec - last_time.tv_sec*1000000)/1000000.0f;
	/*此处反映了实时帧率，单摄像头的时候相对帧率跳动不大，但是多摄像头的时候会因为调度的问题可能连续读几帧导致看到帧率瞬时飙升*/
	printf("Device: %s, timestamp: %d.%06d , duration = %.6f, framerate = %.2f\n",mDevicename, (int)tv.tv_sec, (int)tv.tv_usec, duration, 1/duration);
	last_time = tv;
	framesize = compress_frame(&en, -1, usr_buf[buf.index].start, usr_buf[buf.index].length, (char *)fTo);

	if (-1 == ioctl(fd, VIDIOC_QBUF, &buf))
	{
		perror("Fail to ioctl 'VIDIOC_QBUF'");
		exit(EXIT_FAILURE);
	}
}

void Device::stop_capture(void)
{
	enum v4l2_buf_type type;
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (-1 == ioctl(fd, VIDIOC_STREAMOFF, &type))
	{
		perror("Fail to ioctl 'VIDIOC_STREAMOFF'");
		exit(EXIT_FAILURE);
	}
}

void Device::close_camera(void)
{
	unsigned int i;
	for (i = 0; i < n_buffer; i++)
	{
		if (-1 == munmap(usr_buf[i].start, usr_buf[i].length))
		{
			exit(-1);
		}
	}

	free(usr_buf);
	if (-1 == close(fd))
	{
		perror("Fail to close fd");
		exit(EXIT_FAILURE);
	}
}

int Device::camera_able_read(void)
{
	fd_set fds;
	struct timeval tv;
	int ret;
	FD_ZERO(&fds);
	FD_SET(fd, &fds); /*Timeout*/
	tv.tv_sec = 2;
	tv.tv_usec = 0;
	ret = select(fd + 1, &fds, NULL, NULL, &tv);

	if (-1 == ret)
	{
		if (EINTR == errno)
			return -1;

		perror("Fail to select");
		exit(EXIT_FAILURE);
	}

	if (0 == ret)
	{
		fprintf(stderr, "select Timeout\n");
		//exit(-1);
	}

	return ret;
}

void Device::init_encoder(void)
{
	compress_begin(&en, mWidth, mHeight);
}

void Device::compress_begin(Encoder *en, int width, int height)
{
	en->param = (x264_param_t *)malloc(sizeof(x264_param_t));
	en->picture = (x264_picture_t *)malloc(sizeof(x264_picture_t));
	// x264_param_default(en->param); //set default param
	x264_param_default_preset(en->param, "ultrafast", "zerolatency");

	en->param->i_frame_reference = 3;
	en->param->rc.i_rc_method = X264_RC_ABR;
	en->param->b_cabac = 0;
	en->param->b_interlaced = 0;
	en->param->i_level_idc = 10; //算法复杂度
	en->param->i_keyint_max = en->param->i_fps_num * 1.5;
	en->param->i_keyint_min = 1;
	en->param->i_threads = 1; //不使用并行编码
	en->param->rc.i_lookahead = 0;
	en->param->i_sync_lookahead = 0;
	en->param->i_bframe = 0;		 //连续最大的B帧，由于需要缓存多帧，会降低编码效率，所以设置为0
	en->param->b_sliced_threads = 1; //基于分片的线程，默认为off，开启该方法在压缩率和编码效率上都略低于默认方法，但是没有编码延时。除非在编码实时流或者对延迟要求较高的场合开启该方法，一般情况都是off
	en->param->rc.b_mb_tree = 0;	 //在实时编码时,必须为0
	en->param->b_vfr_input = 0;		 //与force-cfr选项相对应：vfr_input=1时，为可变帧率，使用timebase和timestamps做码率控制；vfr_input=0时，为固定帧率，使用fps来控制
	en->param->i_width = mWidth;
	en->param->i_height = mHeight;
	// en->param->i_frame_total = 0;
	// en->param->i_keyint_max = 10;
	// en->param->b_open_gop = 0;
	// en->param->i_bframe_pyramid = 0;
	// en->param->i_bframe_adaptive = X264_B_ADAPT_TRELLIS;
	// en->param->rc.i_bitrate = 1024 * 10;
	en->param->rc.i_bitrate = 1024;
	en->param->i_fps_num = 25;
	en->param->i_fps_den = 1;

#if 0
	en->param->i_threads  = X264_SYNC_LOOKAHEAD_AUTO;
	en->param->i_width = WIDTH; //set frame width
	en->param->i_height = HEIGHT; //set frame height
	en->param->rc.i_lookahead = 0; 
	en->param->i_fps_num = 30; 
	en->param->i_fps_den = 1;
#endif

	en->param->i_csp = X264_CSP_I422;
	x264_param_apply_profile(en->param, x264_profile_names[4]);

	if ((en->handle = x264_encoder_open(en->param)) == 0)
	{
		return;
	}

	x264_picture_alloc(en->picture, X264_CSP_I422, en->param->i_width,
					   en->param->i_height);
}

int Device::compress_frame(Encoder *en, int type, char *in, int len, char *out)
{
	x264_picture_t pic_out;
	int index_y, index_u, index_v;
	int num;
	int nNal = -1;
	int result = 0;
	int i = 0;
	char *p_out = out;
	char *y = (char *)en->picture->img.plane[0];
	char *u = (char *)en->picture->img.plane[1];
	char *v = (char *)en->picture->img.plane[2];

	index_y = 0;
	index_u = 0;
	index_v = 0;

	num = mWidth * mHeight * 2 - 4;

	for (i = 0; i < num; i = i + 4)
	{
		*(y + (index_y++)) = *(in + i);
		*(u + (index_u++)) = *(in + i + 1);
		*(y + (index_y++)) = *(in + i + 2);
		*(v + (index_v++)) = *(in + i + 3);
	}

	switch (type)
	{
	case 0:
		en->picture->i_type = X264_TYPE_P;
		break;
	case 1:
		en->picture->i_type = X264_TYPE_IDR;
		break;
	case 2:
		en->picture->i_type = X264_TYPE_I;
		break;
	default:
		en->picture->i_type = X264_TYPE_AUTO;
		break;
	}

	en->picture->i_pts++;

	if (x264_encoder_encode(en->handle, &(en->nal), &nNal, en->picture,
							&pic_out) < 0)
	{
		return -1;
	}

	for (i = 0; i < nNal; i++)
	{
		memcpy(p_out, en->nal[i].p_payload, en->nal[i].i_payload);
		p_out += en->nal[i].i_payload;
		result += en->nal[i].i_payload;
	}

	return result;
}

void Device::getnextframe(unsigned int &framesize, unsigned char *fTo)
{
	int ret;
	ret = camera_able_read();
	if (ret > 0)
	{

		read_one_frame(framesize, fTo);
	}
	else
	{
	}
}

void Device::compress_end(Encoder *en)
{
	if (en->handle)
	{
		x264_encoder_close(en->handle);
	}
	if (en->picture)
	{
		x264_picture_clean(en->picture);
		free(en->picture);
		en->picture = 0;
	}
	if (en->param)
	{
		free(en->param);
		en->param = 0;
	}
}

void Device::close_encoder()
{
	compress_end(&en);
}

void Device::Init(const char *devicename, int width, int height)
{
	mWidth = width;
	mHeight = height;
	mDevicename = devicename;
	open_camera(devicename);
	init_camera();
	init_mmap();
	start_capture();
	init_encoder();
}

void Device::Destory()
{
	stop_capture();
	close_encoder();
	close_camera();
}

Device::~Device()
{
	printf("Terminate the camera..\n");
	Destory();
}
