
#ifndef _CAMERA_HH
#define _CAMERA_HH
#include <FramedSource.hh>
#include <UsageEnvironment.hh>
#include "encoder_define.hh"

/// 这是一个配置摄像头和H264编码器的类
///
/// 这个类可以对摄像头进行操作并编码成H264的数据帧
class Device
{
public:
    /// 初始化摄像头内存映射
    void init_mmap(void);
    /// 初始化摄像头
    void init_camera(void);
    /// 初始化H264编码器
    void init_encoder(void);
    /// 打开摄像头
    /// @param devicename 摄像头设备名，如 /dev/video0
    void open_camera(const char *devicename);
    /// 关闭摄像头
    void close_camera(void);
    /// 从缓存中读取一帧数据并编码成H264格式
    /// @param framesize 该H264数据帧的大小
    /// @param fTo 指向该H264数据帧实际数据的指针
    void read_one_frame(unsigned int &framesize, unsigned char *fTo);
     /// 从缓存中读取下一个H264帧
    /// @param framesize 该H264数据帧的大小
    /// @param fTo 指向该H264数据帧实际数据的指针
    void getnextframe(unsigned int &framesize, unsigned char *fTo);
    /// 启动摄像头图像捕获
    void start_capture(void);
    /// 停止摄像头图像捕获
    void stop_capture(void);
    /// 关闭H264编码器
    void close_encoder();
    /// 检测摄像头是否可读
    /// @return 返回摄像头可读状态，大于0为可读
    int camera_able_read(void);
    /// 配置H264压缩参数
    /// @param en H264编码的结构体指针
    /// @param width 图像分辨率的宽度
    /// @param height 图像分辨率的高度
    void compress_begin(Encoder *en, int width, int height);
    /// 原始数据编码成H264数据帧
    /// @param en H264编码的结构体指针
    /// @param type H264编码帧的TYPE
    /// @param in 指向原始数据的指针
    /// @param len 原始数据的长度
    /// @param out 指向H264数据的指针
    /// @return 返回H264帧的长度
    int compress_frame(Encoder *en, int type, char *in, int len, char *out);
    /// 关闭已经配置的编码器
    void compress_end(Encoder *en);
    /// 初始化该类，即初始化摄像头和编码器
    /// @param devicename 摄像头设备名，如 /dev/video0
    /// @param width 图像分辨率的宽度
    /// @param height 图像分辨率的高度
    void Init(const char *devicename, int width, int height);
    /// 停止图像捕获，关闭编码器和关闭摄像头
    void Destory();
    /// Device类的析构函数
    ~Device();

public:
    int fd; ///< 摄像头设备的句柄
    unsigned int n_buffer; ///< 临时变量，多处用于循环下标使用
    Encoder en; ///< H264结构体
    BUFTYPE *usr_buf; ///< BUFTYPE指针

private:
    int mHeight; ///< 图像分辨率高度
    int mWidth;///<  图像分辨率宽度
    const char *mDevicename; ///< 摄像头设备名
    struct timeval last_time = {0, 0}; ///< 上一帧的编码起始时间
};
#endif
