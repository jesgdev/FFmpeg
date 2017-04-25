/*
 * Apple Core Audio ALAC encoder
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
#include "put_bits.h"

//supported sample rates(TODO - double check these values)
static const int sample_rates[] = {
    96000, 88200, 48000, 44100,
    32000, 24000, 22050, 16000,
    12000, 11025, 8000, 0
};

static const uint64_t channel_layouts[] = {
    AV_CH_LAYOUT_MONO,
    AV_CH_LAYOUT_STEREO,
    AV_CH_LAYOUT_SURROUND,
    AV_CH_LAYOUT_4POINT0,
    AV_CH_LAYOUT_5POINT0_BACK,
    AV_CH_LAYOUT_5POINT1_BACK,
    AV_CH_LAYOUT_6POINT1_BACK,
    AV_CH_LAYOUT_7POINT1_WIDE_BACK,
    0
};

static av_cold int get_channel_map_info(AcaEncoderContext* cactx, const SInt32** chmap, UInt32* in_bitmap, UInt32* out_tag)
{
    switch (*in_bitmap)
    {
    case AV_CH_LAYOUT_MONO: *out_tag = kALACChannelLayoutTag_Mono; break;
    case AV_CH_LAYOUT_STEREO: *out_tag = kALACChannelLayoutTag_Stereo; break;
    case AV_CH_LAYOUT_SURROUND: *out_tag = kALACChannelLayoutTag_MPEG_3_0_B; break;
    case AV_CH_LAYOUT_4POINT0: *out_tag = kALACChannelLayoutTag_MPEG_4_0_B; break;
    case AV_CH_LAYOUT_5POINT0_BACK: *out_tag = kALACChannelLayoutTag_MPEG_5_0_D; break;
    case AV_CH_LAYOUT_5POINT1_BACK: *out_tag = kALACChannelLayoutTag_MPEG_5_1_D; break;
    case AV_CH_LAYOUT_6POINT1_BACK: *out_tag = kALACChannelLayoutTag_AAC_6_1; break;
    case AV_CH_LAYOUT_7POINT1_WIDE_BACK: *out_tag = kALACChannelLayoutTag_MPEG_7_1_B; break;
    }

    return 0;
}

#define EPJMP(expr) FF_APPLECA_EXECP_JMP(cactx->bctx.api->expr, appleca_alac_init_error)
static av_cold int appleca_alac_init(AVCodecContext* avctx)
{
    OSStatus ca_result;
    UInt32 val, size;
    int ret = 0;
    uint8_t* magic_cookie = NULL;
    AudioStreamBasicDescription in_desc = { 0 };
    AcaEncoderContext* cactx = avctx->priv_data;

    //init encoder
    if ((ret = ff_appleca_enc_init(avctx, kAudioFormatAppleLossless)) != 0)
    {
        goto appleca_alac_init_error;
    }

    if ((ret = ff_appleca_setup_output_channels(cactx, get_channel_map_info)) != 0)
    {
        goto appleca_alac_init_error;
    }

    //codec quality
    val = kAudioCodecQuality_Max;
    EPJMP(ac_set_prop(cactx->ac, kAudioConverterCodecQuality, sizeof(val), &val));

    //kAudioConverterPropertyBitDepthHint - TODO: is this really needed??
    size = sizeof(in_desc);
    EPJMP(ac_get_prop(cactx->ac, kAudioConverterCurrentInputStreamDescription, &size, &in_desc));
    EPJMP(ac_set_prop(cactx->ac, kAudioConverterPropertyBitDepthHint, sizeof(in_desc.mBitsPerChannel), &in_desc.mBitsPerChannel));

    //post init
    if ((ret = ff_appleca_enc_postinit(avctx)) != 0)
    {
        goto appleca_alac_init_error;
    }

    //extradata (alac magic cookie)
    //https://llvm.org/svn/llvm-project/test-suite/trunk/MultiSource/Applications/ALAC/decode/ALACMagicCookieDescription.txt
    avctx->extradata_size = 0;
    if (avctx->flags & AV_CODEC_FLAG_GLOBAL_HEADER)
    {
        Boolean writable;

        //get max size of cookie and alloc space
        EPJMP(ac_get_prop_info(cactx->ac, kAudioConverterCompressionMagicCookie, &size, &writable));
        magic_cookie = av_malloc(size);
        if (!magic_cookie)
        {
            ret = AVERROR(ENOMEM);
            goto appleca_alac_init_error;
        }

        //alloc extra data
        avctx->extradata = av_mallocz(64 + AV_INPUT_BUFFER_PADDING_SIZE);
        if (!avctx->extradata)
        {
            ret = AVERROR(ENOMEM);
            goto appleca_alac_init_error;
        }

        //get the magic cookie
        EPJMP(ac_get_prop(cactx->ac, kAudioConverterCompressionMagicCookie, &size, magic_cookie));
        if (size >= 24)
        {
            uint8_t* alacp = magic_cookie; //alac cookie within magic_cookie
            uint8_t* alacp_end = alacp + size;
            if (memcmp(alacp + 4, "frmaalac", 8) == 0)
            {
                alacp += 24;
            }

            if ((alacp_end - alacp) >= 24)
            {
                memcpy(avctx->extradata + 12, alacp, 24);
                alacp += 24;
                avctx->extradata_size = 36;
                if ((alacp_end - alacp) >= 24 && memcmp(alacp + 4, "chan", 4) == 0)
                {
                    memcpy(avctx->extradata + 36, alacp, 24);
                    avctx->extradata_size += 24;
                }
            }
        }

        //free the magic_cookie and check result
        av_freep(&magic_cookie);
        if (!avctx->extradata_size)
        {
            av_log(avctx, AV_LOG_ERROR, "failed to get extradata\n");
            goto appleca_alac_init_error;
        }

        //put the size and id(see alacenc.c)
        AV_WB32(avctx->extradata, 36);  //yes always 36, see the link above
        AV_WB32(avctx->extradata + 4, MKBETAG('a', 'l', 'a', 'c'));
    }

    return 0;

appleca_alac_init_error:
    if (magic_cookie)
    {
        av_free(magic_cookie);
    }

    ff_appleca_enc_close(avctx);
    ret = (ret == 0) ? AVERROR_EXTERNAL : ret;
    av_log(avctx, AV_LOG_ERROR, "appleca_alac_init_error return code: %d\n", ret);
    return ret;
}

static const AVClass appleca_alacenc_class = {
    .class_name = "appleca_alac",
    .item_name = av_default_item_name,
    .option = NULL,
    .version = LIBAVUTIL_VERSION_INT,
};

AVCodec ff_appleca_alac_encoder = {
    .name = "appleca_alac",
    .long_name = NULL_IF_CONFIG_SMALL("Apple Core Audio ALAC (Apple Lossless Audio Codec)"),
    .type = AVMEDIA_TYPE_AUDIO,
    .id = AV_CODEC_ID_ALAC,
    .priv_data_size = sizeof(AcaEncoderContext),
    .init_static_data = ff_appleca_static_init,
    .init = appleca_alac_init,
    .encode2 = ff_appleca_encode,
    .close = ff_appleca_enc_close,
    .capabilities = AV_CODEC_CAP_SMALL_LAST_FRAME | AV_CODEC_CAP_LOSSLESS | AV_CODEC_CAP_EXPERIMENTAL,
    .sample_fmts = (const enum AVSampleFormat[]) {
        AV_SAMPLE_FMT_S16,
        AV_SAMPLE_FMT_S32,
        AV_SAMPLE_FMT_FLT,
        AV_SAMPLE_FMT_NONE
    },

    .priv_class = &appleca_alacenc_class,
    .supported_samplerates = sample_rates,
    .channel_layouts = channel_layouts,
};