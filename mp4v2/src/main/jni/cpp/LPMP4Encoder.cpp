//
//  $Id: LPMP4Encoder.cpp 2214 2016-01-15 01:40:21Z hu_danyuan $
//  Media
//
//  Created by hu_danyuan on 14-7-24.
//   (c) Copyright 2014,  杭州泺平电子有限公司
//                            All Rights Reserved
//
//	描    述: MP4写文件
//

#include "mp4v2/LPMP4Encoder.h"
#include <stdlib.h>

#ifdef __LP_ANDROID
#include <android/log.h>
#define MP4LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG, "[MP4V2]: ", __VA_ARGS__)
#else
#include <stdio.h>
#define MP4LOGD(fmt,arg...) printf("[SHDT]: "fmt, ##arg)
#endif

#define NALUGetFZero(x) ((x) & 0x80)
#define NALUGetNRI(x) ((x) & 0x60)
#define NALUGetType(x) ((x) & 0x1f)

LPMP4Encoder::LPMP4Encoder()
{
    m_timescale = 0;
    m_video_width = 0;
    m_video_height = 0;
    m_file_handle = MP4_INVALID_FILE_HANDLE;
    m_track_id = MP4_INVALID_TRACK_ID;
    m_is_add_h264 = false;
    m_is_add_sps = false;
    m_is_add_pps = false;
    
    m_buf_size = 1920 * 1080 * 2;
    m_buffer = (unsigned char *)malloc(m_buf_size);
}

LPMP4Encoder::~LPMP4Encoder()
{
    if (m_buffer != NULL) {
        free(m_buffer);
    }
}
// 创建文件
bool LPMP4Encoder::createFile(const char *filepath, unsigned int timescale)
{
    if (filepath == NULL) {
        MP4LOGD("createFile filepath==NULL");
        return false;
    }
    
    // create mp4 file
    m_file_handle = MP4Create(filepath, 0);
    //	m_file_handle = MP4CreateEx(filepath, MP4_DETAILS_ALL, 0, 1, 1, 0, 0, 0, 0);
    if (m_file_handle == MP4_INVALID_FILE_HANDLE)
    {
        MP4LOGD("ERROR:Open file fialed");
        return false;
    }
    m_timescale = timescale;
    MP4SetTimeScale(m_file_handle, timescale);
    MP4SetVideoProfileLevel(m_file_handle, 0x01); //  Simple Profile @ Level 3
    
    return true;
}
// 关闭文件
void LPMP4Encoder::closeFile()
{
    m_track_id = MP4_INVALID_TRACK_ID;
    if (m_file_handle)
    {
        MP4Close(m_file_handle, 0);
        m_file_handle = MP4_INVALID_FILE_HANDLE;
    }
    m_is_add_h264 = false;
    m_is_add_sps = false;
    m_is_add_pps = false;
}
// 设置视频宽高
void LPMP4Encoder::setWidthAndHeight(int width, int height)
{
    m_video_width = width;
    m_video_height = height;
}
// 写h264数据到文件
int LPMP4Encoder::writeH264Data(const char *data, unsigned int size, unsigned int width, unsigned int height, unsigned int framerate)
{
    if (width <= 0 || height <= 0 || framerate <= 0)
    {
        return -1;
    }
    if (m_file_handle == MP4_INVALID_FILE_HANDLE)
    {
        MP4LOGD("writeH264Data file_handle == MP4_INVALID_FILE_HANDLE");
        return -1;
    }
    if (data == NULL || size <= 0)
    {
        return -1;
    }
    // 设置宽高，设置缓存大小
    setWidthAndHeight(width, height);
    
    int pos = 0;
    int len = 0;
    H264NALU nalu;
    // 解析head
    while ((len = readFirstNaluFromBuf(data, size, pos, nalu)) != 0)
    {
        unsigned char nal_ref_idc = NALUGetNRI(nalu.head);
        unsigned char nal_type = NALUGetType(nalu.head);
        if (nal_ref_idc == 0) {
            // 无效NALU, 丢弃
            pos += len;
            continue;
        }
        
        //sps pps顺序不定
        if (!m_is_add_h264 && (nal_type == 0x07 || nal_type == 0x08)) {
            //添加h264 track
            m_track_id = MP4AddH264VideoTrack(m_file_handle,
                                              m_timescale,
                                              m_timescale / framerate,
                                              m_video_width,     // width
                                              m_video_height,    // height
                                              nalu.data[1], // sps[1] AVCProfileIndication
                                              nalu.data[2], // sps[2] profile_compat
                                              nalu.data[3], // sps[3] AVCLevelIndication
                                              3);           // 4 bytes length before each NAL unit
        }
        if (m_track_id == MP4_INVALID_TRACK_ID)
        {
            MP4LOGD("add video track failed.");
            return -1;
        }
        else
        {
            m_is_add_h264 = true;
        }
        if (nal_type == 0x07) { // sps
            MP4AddH264SequenceParameterSet(m_file_handle, m_track_id, nalu.data, nalu.size);
            m_is_add_sps = true;
        }
        else if (nal_type == 0x08) { // pps
            MP4AddH264PictureParameterSet(m_file_handle, m_track_id, nalu.data, nalu.size);
            m_is_add_pps = true;
        }
        else {
            int datalen = nalu.size + 4;
            if (m_buf_size <= datalen) {
                MP4LOGD("low buffer");
                continue;
            }
            uint8_t *data = m_buffer;
            if (data != NULL) {
                // MP4 NALU长度 大端
                data[0] = nalu.size >>  24;
                data[1] = nalu.size >>  16;
                data[2] = nalu.size >>  8;
                data[3] = nalu.size &   0xff;
                memcpy(data + 4, nalu.data, nalu.size);
                if (!MP4WriteSample(m_file_handle, m_track_id, data, datalen, MP4_INVALID_DURATION, 0, 1))
                {
                    return -1;
                }
            }
        }
        
        pos += len;
    }
    return pos;
}

int LPMP4Encoder::readFirstNaluFromBuf(const char *buffer, unsigned int len, unsigned int offset, H264NALU &nalu)
{
    nalu.head = 0;
    nalu.data = NULL;
    nalu.size = 0;
    
    unsigned int i = offset;
    int syncCodeLen = 4;
    while (i + syncCodeLen < len)
    {
        //读取帧头0x00000001
        if (buffer[i + 0] == 0x00 &&
            buffer[i + 1] == 0x00 &&
            buffer[i + 2] == 0x00 &&
            buffer[i + 3] == 0x01)
        {
            i += syncCodeLen;
            int pos = i;
            //读取下一帧帧头
            while (pos + syncCodeLen <= len)
            {
                if (buffer[pos + 0] == 0x00 &&
                    buffer[pos + 1] == 0x00 &&
                    buffer[pos + 2] == 0x00 &&
                    buffer[pos + 3] == 0x01)
                {
                    break;
                }
                pos++;
            }
            if (pos + syncCodeLen >= len)
            {
                nalu.size = len - i;
            }
            else
            {
                nalu.size = pos - i;
            }
            nalu.head = buffer[i] & 0xff;
            nalu.data = (unsigned char *) & buffer[i];
            return (nalu.size + i - offset);
        }
        i++;
    }
    return 0;
}