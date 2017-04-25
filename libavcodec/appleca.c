/*
 * Apple Core Audio Support
 * Copyright (C) 2016
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "appleca.h"
#include "libavutil/avassert.h"
#include "libavutil/thread.h"
#include <ctype.h>

//dynamic linking
#if defined(_WIN32)
    #define WIN32_LEAN_AND_MEAN 1
    #define NOMINMAX 1
    #include <windows.h>
    #include <ShlObj.h>
    #include <Shlwapi.h>
    #define DLL_LOAD(file) LoadLibrary(file)
    #define DLL_FREE(handle) FreeLibrary((HMODULE)(handle))
    #define DLL_GET_PROC(handle,name) GetProcAddress((HMODULE)(handle),name)
#else
    #include <dlfcn.h>
    #define DLL_LOAD(file) dlopen(file, RTLD_NOW | RTLD_LOCAL)
    #define DLL_FREE(handle) dlclose(handle)
    #define DLL_GET_PROC(handle,name) dlsym(handle,name)
#endif

#define EPRET(expr) FF_APPLECA_EXECP_RET(cactx->bctx.api->expr)
#define APPLECA_GET_PROC(pptr, ptype, pname) \
    do { \
        (pptr) = (ptype)DLL_GET_PROC(handle, pname); \
        if (!(pptr)) \
        { \
            av_log(avctx, AV_LOG_ERROR, "Failed loading proc '%s' from CoreAudio dll\n", pname); \
            goto ff_appleca_get_api_error; \
        } \
    } while(0)

static AcaApi appleca_api = { 0 };
static int appleca_api_ref_count = 0;
static AVMutex appleca_api_mutex;
av_cold void ff_appleca_static_init(AVCodec* codec)
{
    static int initialized = 0;
    if (!initialized)
    {
        ff_mutex_init(&appleca_api_mutex, NULL);
        initialized = 1;
    }
}

av_cold AcaApi* ff_appleca_get_api(AVCodecContext* avctx)
{
    AcaApi* api = NULL;
    void* handle = NULL;

    //TODO: thread safety actually needed?
    ff_mutex_lock(&appleca_api_mutex);
    if (appleca_api_ref_count == 0)
    {
#if defined(_WIN32)
        TCHAR path[MAX_PATH];
        HRESULT hr = SHGetFolderPathAndSubDir(NULL, CSIDL_PROGRAM_FILES_COMMON, NULL, 0, TEXT("Apple\\Apple Application Support"), path);
        if (SUCCEEDED(hr))
        {
            PathAppend(path, TEXT("CoreAudioToolbox.dll"));
            handle = LoadLibraryEx(path, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
        }
#else
        //TODO: test other systems, this is a guess...
        handle = DLL_LOAD("CoreAudioToolbox.so");
#endif

        //common to all below here...
        if (handle)
        {
            //load procs
            APPLECA_GET_PROC(appleca_api.ac_new, AudioConverterNewProc, "AudioConverterNew");
            APPLECA_GET_PROC(appleca_api.ac_get_prop, AudioConverterGetPropertyProc, "AudioConverterGetProperty");
            APPLECA_GET_PROC(appleca_api.ac_get_prop_info, AudioConverterGetPropertyInfoProc, "AudioConverterGetPropertyInfo");
            APPLECA_GET_PROC(appleca_api.ac_set_prop, AudioConverterSetPropertyProc, "AudioConverterSetProperty");
            APPLECA_GET_PROC(appleca_api.ac_fcb, AudioConverterFillComplexBufferProc, "AudioConverterFillComplexBuffer");
            APPLECA_GET_PROC(appleca_api.ac_dispose, AudioConverterDisposeProc, "AudioConverterDispose");
            APPLECA_GET_PROC(appleca_api.af_get_prop, AudioFormatGetPropertyProc, "AudioFormatGetProperty");
            appleca_api.handle = handle;
        }
        else
        {
            av_log(avctx, AV_LOG_ERROR, "Failed to load CoreAudio dll\n");
            goto ff_appleca_get_api_error;
        }
    }

    //set the pointer and return success
    api = &appleca_api;
    appleca_api_ref_count++;
    ff_mutex_unlock(&appleca_api_mutex);
    return api;

ff_appleca_get_api_error:
    if (handle)
    {
        DLL_FREE(handle);
    }

    memset(&appleca_api, 0, sizeof(appleca_api));
    ff_mutex_unlock(&appleca_api_mutex);
    return NULL;
}

av_cold void ff_appleca_release_api(AcaApi** api)
{
    if (api && *api)
    {
        //TODO: thread safety actually needed?
        ff_mutex_lock(&appleca_api_mutex);

        appleca_api_ref_count--;
        if (appleca_api_ref_count < 0)
        {
            appleca_api_ref_count = 0;
        }

        if (appleca_api_ref_count == 0)
        {
            DLL_FREE((*api)->handle);
            memset(*api, 0, sizeof(AcaApi));
        }

        *api = NULL;
        ff_mutex_unlock(&appleca_api_mutex);
    }
}

av_cold int ff_appleca_enc_close(AVCodecContext* avctx)
{
    AcaEncoderContext* cactx = avctx->priv_data;
    if (cactx->ac)
    {
        cactx->bctx.api->ac_dispose(cactx->ac);
        cactx->ac = NULL;
    }

    ff_appleca_release_api(&cactx->bctx.api);
    av_freep(&avctx->extradata);
    cactx->avctx = NULL;
    return 0;
}

av_cold int ff_appleca_enc_init(AVCodecContext* avctx, UInt32 outputFormatId)
{
    OSStatus ca_result;
    UInt32 propSize;
    AudioStreamBasicDescription input_desc = { 0 };
    AudioStreamBasicDescription output_desc = { 0 };
    AudioChannelLayout chl = { 0 };
    AcaEncoderContext* cactx = avctx->priv_data;

    //store pointer to avctx
    cactx->avctx = avctx;

    //get an api ref if needed
    if (!cactx->bctx.api)
    {
        cactx->bctx.api = ff_appleca_get_api(avctx);
        if (!cactx->bctx.api)
        {
            return AVERROR_EXTERNAL;
        }
    }

    //in_bytes_per_sample
    cactx->in_bytes_per_sample = av_get_bytes_per_sample(avctx->sample_fmt);
    if (!cactx->in_bytes_per_sample)
    {
        av_log(avctx, AV_LOG_ERROR, "Unsupported sample_fmt: %d\n", avctx->sample_fmt);
        return AVERROR(EINVAL);
    }

    //in_bytes_per_packet
    cactx->in_bytes_per_packet = avctx->channels * cactx->in_bytes_per_sample;

    //input_desc
    input_desc.mFormatID = kAudioFormatLinearPCM;
    input_desc.mSampleRate = avctx->sample_rate;
    input_desc.mChannelsPerFrame = avctx->channels;
    input_desc.mFramesPerPacket = 1;
    input_desc.mBytesPerFrame = cactx->in_bytes_per_packet;
    input_desc.mBytesPerPacket = input_desc.mBytesPerFrame; //since mFramesPerPacket==1
    input_desc.mFormatFlags = kAudioFormatFlagsNativeEndian;
    input_desc.mFormatFlags |= (avctx->sample_fmt == AV_SAMPLE_FMT_FLT) ? kAudioFormatFlagIsFloat : kAudioFormatFlagIsSignedInteger;
    av_log(avctx, AV_LOG_DEBUG, "bits_per_raw_sample: %d, bits_per_coded_sample: %d\n", avctx->bits_per_raw_sample, avctx->bits_per_coded_sample);
    if (avctx->sample_fmt != AV_SAMPLE_FMT_S32 || //16 or float use all bits
        avctx->bits_per_raw_sample == 0 ||        //its S32 and bits isn't set, assume all
        avctx->bits_per_raw_sample == 32)         //its 32 and bits is also 32, use all
    {
        input_desc.mFormatFlags |= kAudioFormatFlagIsPacked;
        input_desc.mBitsPerChannel = (8 * cactx->in_bytes_per_sample);
    }
    else //S32 and using only some of the bits...
    {
        //TODO - maybe drop s32 support and convert to floats instead?
        input_desc.mFormatFlags |= kAudioFormatFlagIsAlignedHigh;    //TODO - how to detect this properly??
        input_desc.mBitsPerChannel = avctx->bits_per_raw_sample;     //TODO - is this the right value??
        av_log(avctx, AV_LOG_VERBOSE, "S32 ALIGNED: bits_per_raw_sample: %d, bits_per_coded_sample: %d\n", avctx->bits_per_raw_sample, avctx->bits_per_coded_sample);
    }

    //output_desc
    output_desc.mSampleRate = avctx->sample_rate;
    output_desc.mChannelsPerFrame = avctx->channels;
    output_desc.mFormatID = outputFormatId;

    //let core audio fill in remaining...
    propSize = sizeof(output_desc);
    EPRET(af_get_prop(kAudioFormatProperty_FormatInfo, 0, NULL, &propSize, &output_desc));

    //create audio converter
    EPRET(ac_new(&input_desc, &output_desc, &cactx->ac));

    //set input channel layout
    chl.mChannelLayoutTag = kAudioChannelLayoutTag_UseChannelBitmap;
    chl.mChannelBitmap = (UInt32)avctx->channel_layout;
    EPRET(ac_set_prop(cactx->ac, kAudioConverterInputChannelLayout, ff_appleca_sizeof_acl(&chl), &chl));

    return 0;
}

av_cold int ff_appleca_enc_postinit(AVCodecContext* avctx)
{
    OSStatus ca_result;
    UInt32 val, size;
    AudioStreamBasicDescription output_desc;
    AcaEncoderContext* cactx = avctx->priv_data;

    //frame size
    size = sizeof(val);
    EPRET(ac_get_prop(cactx->ac, kAudioCodecPropertyPacketFrameSize, &size, &val));
    avctx->frame_size = (int)val;
    av_log(avctx, AV_LOG_DEBUG, "set avctx->frame size: %d\n", avctx->frame_size);

    //output_buffer_bytes(1 packet)
    size = sizeof(cactx->output_buffer_bytes);
    EPRET(ac_get_prop(cactx->ac, kAudioCodecPropertyMaximumPacketByteSize, &size, &cactx->output_buffer_bytes));
    av_log(avctx, AV_LOG_DEBUG, "set cactx->output_buffer_bytes: %u\n", cactx->output_buffer_bytes);

    //get output_desc
    size = sizeof(output_desc);
    EPRET(ac_get_prop(cactx->ac, kAudioConverterCurrentOutputStreamDescription, &size, &output_desc));

    //check if packet_desc is needed
    size = sizeof(val);
    EPRET(af_get_prop(kAudioFormatProperty_FormatIsExternallyFramed, sizeof(output_desc), &output_desc, &size, &val));
    cactx->use_pkt_desc = !!val;
    av_log(avctx, AV_LOG_DEBUG, "set cactx->use_pkt_desc: %u\n", cactx->use_pkt_desc);

    return 0;
}

//user data for complex_input_data_proc
typedef struct ComplexInputDataProcContext
{
    AcaEncoderContext* cactx;
    const AVFrame* frame;
} ComplexInputDataProcContext;

static OSStatus complex_input_data_proc(
    AudioConverterRef inAudioConverter,
    UInt32* ioNumberDataPackets,
    AudioBufferList* ioData,
    AudioStreamPacketDescription** outDataPacketDescription,
    void* inUserData)
{
    ComplexInputDataProcContext* dpctx = inUserData;
    AudioBuffer* ab = &ioData->mBuffers[0];
    if (!dpctx->frame)
    {
        av_log(dpctx->cactx->avctx, AV_LOG_DEBUG, "!frame: should only happen at end\n");
        ab->mData = NULL;
        ab->mDataByteSize = 0;
        *ioNumberDataPackets = 0;
    }
    else
    {
        ab->mData = dpctx->frame->data[0];
        ab->mDataByteSize = (dpctx->frame->nb_samples * dpctx->cactx->in_bytes_per_packet);
        *ioNumberDataPackets = (ab->mDataByteSize / dpctx->cactx->in_bytes_per_packet);
        dpctx->frame = NULL; //acts as a flag if called more than once(detect small frame(ie: end))
    }

    return 0;
}

int ff_appleca_encode(AVCodecContext* avctx, AVPacket* avpkt, const AVFrame* frame, int* got_packet_ptr)
{
    int ret;
    OSStatus status;
    AudioBufferList abl;
    AudioStreamPacketDescription packet_desc;
    UInt32 num_packets = 1;
    AcaEncoderContext* cactx = avctx->priv_data;
    ComplexInputDataProcContext convert_ctx = { cactx, frame };

    //alloc packet
    if ((ret = ff_alloc_packet2(cactx->avctx, avpkt, cactx->output_buffer_bytes, 0)) < 0)
    {
        return ret;
    }

    //setup abl
    abl.mNumberBuffers = 1;
    abl.mBuffers[0].mData = avpkt->data;
    abl.mBuffers[0].mDataByteSize = avpkt->size;
    abl.mBuffers[0].mNumberChannels = avctx->channels;

    //call the converter
    status = cactx->bctx.api->ac_fcb(
        cactx->ac,
        complex_input_data_proc,
        &convert_ctx,
        &num_packets,
        &abl,
        &packet_desc);

    if (status)
    {
        av_log(avctx, AV_LOG_ERROR, "AudioConverterFillComplexBuffer failed with error '%s'\n", ff_appleca_get_error_string(status));
        return AVERROR_EXTERNAL;
    }

    if (num_packets != 1)
    {
        av_log(avctx, AV_LOG_ERROR, "AudioConverterFillComplexBuffer returned %u packets, expected 1\n", num_packets);
        return AVERROR_BUG;
    }

    avpkt->data = abl.mBuffers[0].mData;
    avpkt->size = abl.mBuffers[0].mDataByteSize;
    if (cactx->use_pkt_desc)
    {
        avpkt->data += packet_desc.mStartOffset;
        avpkt->size = packet_desc.mDataByteSize;
    }

    *got_packet_ptr = 1;
    return 0;
}

int ff_appleca_setup_output_channels(AcaEncoderContext* cactx, ff_appleca_get_channel_map_info get_channel_map_info)
{
    OSStatus ca_result;
    UInt32 in_bitmap;
    AudioChannelLayout chl = { 0 };
    const SInt32* pmap = NULL;
    SInt32 channel_map[8] = { -1, -1, -1, -1, -1, -1, -1, -1 };

    av_assert0(cactx != NULL);
    av_assert0(cactx->avctx != NULL);
    av_assert0(get_channel_map_info != NULL);

    in_bitmap = (UInt32)cactx->avctx->channel_layout;
    chl.mChannelLayoutTag = kAudioChannelLayoutTag_Unknown;
    ca_result = get_channel_map_info(cactx, &pmap, &in_bitmap, &chl.mChannelLayoutTag);
    if (ca_result)
    {
        return ca_result;
    }

    if (chl.mChannelLayoutTag == kAudioChannelLayoutTag_Unknown)
    {
        av_log(cactx->avctx, AV_LOG_ERROR, "Unsupported channel layout 0x%02X", (UInt32)cactx->avctx->channel_layout);
        return AVERROR(EINVAL);
    }

    //set channel layout
    EPRET(ac_set_prop(cactx->ac, kAudioConverterOutputChannelLayout, ff_appleca_sizeof_acl(&chl), &chl));

    //ensure channel map
    if (!pmap)
    {
        //get a channel map from core audio
        pmap = channel_map;
        ca_result = ff_appleca_get_chanmap(cactx, channel_map, in_bitmap, chl.mChannelLayoutTag, cactx->avctx->channels);
        if (ca_result)
        {
            return ca_result;
        }
    }

    //set the channel map
    av_log(cactx->avctx, AV_LOG_VERBOSE,
        "setting chmap: %d,%d,%d,%d,%d,%d,%d,%d\n",
        pmap[0], pmap[1], pmap[2], pmap[3], pmap[4], pmap[5], pmap[6], pmap[7]);

    EPRET(ac_set_prop(cactx->ac, kAudioConverterChannelMap, (cactx->avctx->channels * sizeof(pmap[0])), pmap));

    return 0;
}

int ff_appleca_get_chanmap(AcaEncoderContext* cactx, SInt32* chmap, UInt32 in_bitmap, UInt32 out_tag, int num_channels)
{
    OSStatus ca_result;
    AudioChannelLayout in = { 0 };
    AudioChannelLayout out = { 0 };
    AudioChannelLayout* layouts[] = { &in , &out };
    UInt32 size = num_channels * sizeof(SInt32);
    in.mChannelLayoutTag = kAudioChannelLayoutTag_UseChannelBitmap;
    in.mChannelBitmap = in_bitmap;
    out.mChannelLayoutTag = out_tag;
    EPRET(af_get_prop(kAudioFormatProperty_ChannelMap, sizeof(AudioChannelLayout*) * 2, layouts, &size, chmap));
    return 0;
}

UInt32 ff_appleca_sizeof_acl(AudioChannelLayout* acl)
{
    int num = (int)acl->mNumberChannelDescriptions;
    UInt32 size = (UInt32)(FFMAX(0, (num - 1)) * sizeof(AudioChannelDescription));
    return (size + offsetof(AudioChannelLayout, mChannelDescriptions[1]));
}

/* get a string representation of the error code
if the code is a fourchar code, that is returned,
otherwise the code in hex is returned (with leading '0x') */
static char error_buffer[19];   //0x + 16 hex digits(up to) + NULL terminator
const char* ff_appleca_get_error_string(OSStatus code)
{
    char ch;
    int i, shift;
    for (i = 3, shift = 0; i >= 0; i--, shift += 8)
    {
        ch = ((code >> shift) & 0xFF);
        if (isprint(ch))
        {
            error_buffer[i] = ch;
        }
        else
        {
            //not a fourcc
            snprintf(error_buffer, sizeof(error_buffer), "0x%02X", code);
            break;
        }
    }

    if (i == -1) //fourcc, add null
    {
        error_buffer[4] = 0;
    }

    return error_buffer;
}

void ff_appleca_dump_stream_basic_desc(AVCodecContext* avctx, const char* header, int log_level, AudioStreamBasicDescription* desc)
{
    av_log(avctx, log_level, "%s\n", header);
    av_log(avctx, log_level, "mSampleRate: %F\n", desc->mSampleRate);
    av_log(avctx, log_level, "mFormatID: %u\n", desc->mFormatID);
    av_log(avctx, log_level, "mFormatFlags: %u\n", desc->mFormatFlags);
    av_log(avctx, log_level, "mBytesPerPacket: %u\n", desc->mBytesPerPacket);
    av_log(avctx, log_level, "mFramesPerPacket: %u\n", desc->mFramesPerPacket);
    av_log(avctx, log_level, "mBytesPerFrame: %u\n", desc->mBytesPerFrame);
    av_log(avctx, log_level, "mChannelsPerFrame: %u\n", desc->mChannelsPerFrame);
    av_log(avctx, log_level, "mBitsPerChannel: %u\n", desc->mBitsPerChannel);
    av_log(avctx, log_level, "mReserved: %u\n", desc->mReserved);
}

int ff_appleca_check_bitrate(AcaEncoderContext* cactx, UInt32 bitrate)
{
    OSStatus ca_result;
    UInt32 size;
    Boolean writable;
    AudioValueRange* rates = NULL;

    //get applicable bitrates
    EPRET(ac_get_prop_info(cactx->ac, kAudioConverterApplicableEncodeBitRates, &size, &writable));
    if (size)
    {
        //alloc a buffer
        rates = av_malloc(size);
        if (!rates)
        {
            av_log(cactx->avctx, AV_LOG_ERROR, "failed to allocate AudioValueRange buffer, size %u\n", size);
            return AVERROR(ENOMEM);
        }

        //get bitrates
        FF_APPLECA_EXECP_JMP(cactx->bctx.api->ac_get_prop(cactx->ac, kAudioConverterApplicableEncodeBitRates, &size, rates), ff_appleca_check_bitrate_error);
    }

    ca_result = AVERROR(EINVAL);
    size /= sizeof(AudioValueRange);
    av_log(cactx->avctx, AV_LOG_VERBOSE, "checking bitrate: %u\n", bitrate);
    if (size)
    {
        char buf[512];
        char* pbuf = buf;
        char* pbuf_end = pbuf + sizeof(buf);
        UInt32 ndelim = size - 1;
        UInt32 valid_rate;
        for (int i = 0; i < size; i++)
        {
            valid_rate = (UInt32)rates[i].mMinimum;
            pbuf += snprintf(pbuf, pbuf_end - pbuf, "%uk%c", valid_rate / 1000, ((i == ndelim) ? ' ' : ','));
            if (ca_result && bitrate == valid_rate)
            {
                ca_result = 0;  //matched bitrate
            }
        }

        if (ca_result)
        {
            av_log(cactx->avctx, AV_LOG_ERROR, "bitrate %u not available; applicable bitrates: %s\n", bitrate, buf);
        }
        else
        {
            av_log(cactx->avctx, AV_LOG_VERBOSE, "applicable bitrates: %s\n", buf);
        }
    }
    else
    {
        av_log(cactx->avctx, AV_LOG_ERROR, "no applicable bitrates found, try changing encode mode\n");
    }

ff_appleca_check_bitrate_error:
    if (rates)
    {
        av_free(rates);
    }

    return ca_result;
}