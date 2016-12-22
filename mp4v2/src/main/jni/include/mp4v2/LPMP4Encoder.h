#pragma once
//
//  $Id$
//  Media
//
//  Created by hu_danyuan on 14-7-24.
//   (c) Copyright 2014,  杭州泺平电子有限公司
//                            All Rights Reserved
//
//	描    述: MP4写文件
//

#include <mp4v2/mp4v2.h>
#include <string>

/***************************
网络抽象层单元类型 (NALU)
NALU 头由一个字节组成, 它的语法如下:
      +---------------+
      |0|1|2|3|4|5|6|7|
      +-+-+-+-+-+-+-+-+
      |F|NRI|  Type   |
      +---------------+

F: 1 个比特.
  forbidden_zero_bit. 在 H.264 规范中规定了这一位必须为 0.
NRI: 2 个比特.
  nal_ref_idc. 取 00 ~ 11, 似乎指示这个 NALU 的重要性, 如 00 的 NALU 解码器可以丢弃它而不影响图像的回放. 不过一般情况下不太关心这个属性.
Type: 5 个比特.
  nal_unit_type. 标识NAL单元中的RBSP数据类型，其中，nal_unit_type为1， 2， 3， 4， 5及12的NAL单元称为VCL的NAL单元，其他类型的NAL单元为非VCL的NAL单元。简述如下:
  0		未规定
  1		非IDR图像中不采用数据划分的片段
  2		非IDR图像中A类数据划分片段
  3		非IDR图像中B类数据划分片段
  4		非IDR图像中C类数据划分片段
  5		IDR图像的片段
  6		补充增强信息 (SEI)
  7		序列参数集
  8		图像参数集
  9		分割符
  10	序列结束符
  11	流结束符
  12	填充数据
  13 – 23：保留
  24    STAP-A   单一时间的组合包
  25    STAP-B   单一时间的组合包
  26    MTAP16   多个时间的组合包
  27    MTAP24   多个时间的组合包
  28    FU-A     分片的单元
  29    FU-B     分片的单元
  30-31 没有定义
***************************/
typedef struct _H264NALU {
	_H264NALU() {
		head = 0;
		data = NULL;
		size = 0;
	}
	unsigned char head; // NALU头部信息
	uint8_t *data; // 帧数据
	int size; // 帧大小
}H264NALU;

class LPMP4Encoder
{
public:
    LPMP4Encoder();
    virtual ~LPMP4Encoder();
    
    // 创建文件
    bool createFile(const char *filepath, unsigned int timescale);
    // 关闭文件
    void closeFile();
    // 写h264数据到文件，不支持多线程写入
    int writeH264Data(const char *data, unsigned int size, unsigned int width, unsigned int height, unsigned int framerate);
    
private:
    // 设置视频宽高
    void setWidthAndHeight(int width, int height);
    // 解析head
	int readFirstNaluFromBuf(const char *buffer, unsigned int len, unsigned int offset, H264NALU &nalu);
    
private:
    MP4FileHandle  m_file_handle; // mp4文件handle
    MP4TrackId 	   m_track_id; // track id
    
    bool m_is_add_h264; // 是否已经add h264 VideoTrack
    bool m_is_add_sps; // 是否已经add h264 SequenceParameterSet
    bool m_is_add_pps; // 是否已经add h264 PictureParameterSet
    
    //unsigned char *m_buffer; // 缓存
    unsigned char *m_buffer; // 缓存
    int 		   m_buf_size; // 缓存大小
    
    unsigned int  m_video_width; // 视频宽度
    unsigned int  m_video_height; // 视频高度
    unsigned int  m_timescale; // 时间标度
};
