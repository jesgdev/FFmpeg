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

#ifndef AVCODEC_APPLECA_H
#define AVCODEC_APPLECA_H

#include "appleca_defs.h"
#include "avcodec.h"
#include "libavutil/channel_layout.h"
#include "libavutil/common.h"
#include "libavutil/opt.h"
#include "internal.h"

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct AcaApi
    {
        //dll handle
        void* handle;

        //audio converter
        AudioConverterNewProc ac_new;
        AudioConverterGetPropertyProc ac_get_prop;
        AudioConverterGetPropertyInfoProc ac_get_prop_info;
        AudioConverterSetPropertyProc ac_set_prop;
        AudioConverterFillComplexBufferProc ac_fcb;
        AudioConverterDisposeProc ac_dispose;

        //audio format
        AudioFormatGetPropertyProc af_get_prop;

    } AcaApi;

    //base context
    typedef struct AcaContext
    {
        const AVClass* class;
        AcaApi* api;            //imported core audio api
    } AcaContext;

    //encoder context
    typedef struct AcaEncoderContext
    {
        AcaContext bctx;                //base context, must be 1st in struct!
        AVCodecContext* avctx;          //convience ref
        AudioConverterRef ac;           //core audio converter
        int in_bytes_per_sample;        //input bytes per sample
        int in_bytes_per_packet;        //input bytes per packet
        Boolean use_pkt_desc;           //use packet_desc for output
        UInt32 output_buffer_bytes;     //max size needed for output buffer
    } AcaEncoderContext;

    //api
    av_cold AcaApi* ff_appleca_get_api(AVCodecContext* avctx);
    av_cold void ff_appleca_release_api(AcaApi** api);

    //static
    av_cold void ff_appleca_static_init(AVCodec* codec);

    //encoder
    av_cold int ff_appleca_enc_close(AVCodecContext* avctx);
    av_cold int ff_appleca_enc_init(AVCodecContext* avctx, UInt32 outputFormatId);
    av_cold int ff_appleca_enc_postinit(AVCodecContext* avctx);
    int ff_appleca_encode(AVCodecContext* avctx, AVPacket* avpkt, const AVFrame* frame, int* got_packet_ptr);

    //decoder

    //output channel setup
    typedef int(*ff_appleca_get_channel_map_info) (AcaEncoderContext* cactx, const SInt32** chmap, UInt32* in_bitmap, UInt32* out_tag);
    int ff_appleca_setup_output_channels(AcaEncoderContext* cactx, ff_appleca_get_channel_map_info get_channel_map_info);

    //helpers
    int ff_appleca_get_chanmap(AcaEncoderContext* cactx, SInt32* chmap, UInt32 in_bitmap, UInt32 out_tag, int num_channels);
    UInt32 ff_appleca_sizeof_acl(AudioChannelLayout* acl);
    const char* ff_appleca_get_error_string(OSStatus code);
    void ff_appleca_dump_stream_basic_desc(AVCodecContext* avctx, const char* header, int log_level, AudioStreamBasicDescription* desc);
    int ff_appleca_check_bitrate(AcaEncoderContext* cactx, UInt32 bitrate);

#define FF_APPLECA_EXECP_RET(expr) \
    do { \
        ca_result = expr; \
        if (ca_result) \
        { \
            av_log(cactx->avctx, AV_LOG_ERROR, "CoreAudio proc error: %s\n", ff_appleca_get_error_string(ca_result)); \
            av_log(cactx->avctx, AV_LOG_ERROR, "CoreAudio proc call: %s\n", #expr); \
            return AVERROR_EXTERNAL; \
        } \
    } while (0)

#define FF_APPLECA_EXECP_JMP(expr,errlbl) \
    do { \
        ca_result = expr; \
        if (ca_result) { \
            av_log(cactx->avctx, AV_LOG_ERROR, "CoreAudio proc error: %s\n", ff_appleca_get_error_string(ca_result)); \
            av_log(cactx->avctx, AV_LOG_ERROR, "CoreAudio proc call: %s\n", #expr); \
            goto errlbl; \
        } \
    } while (0)

#ifdef __cplusplus
}
#endif

#endif //#ifndef AVCODEC_APPLECA_H