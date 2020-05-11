#ifndef _ENCODER_DEFINE_HH
#define _ENCODER_DEFINE_HH


extern "C" {
#include <stdint.h>
//#include <unistd.h>
#include <x264.h>

typedef struct 
{
	x264_param_t *param; ///< 编码器参数指针
	x264_t *handle; ///< 编码器句柄指针
	x264_picture_t *picture; ///< 图像参数指针
	x264_nal_t *nal; ///< NAL头参数指针
} Encoder; ///< H264编码器结构体

typedef struct
{
    char *start; ///< 数据起始地址
    int length; ///< 数据长度
}BUFTYPE;

}

#endif
