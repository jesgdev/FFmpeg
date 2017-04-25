/*
 * Apple Core Audio AAC encoder
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
#include "get_bits.h"
#include "put_bits.h"
#include "mpeg4audio.h"

//priv_data type
typedef struct AcaAacContext {

    //encoder context, should be 1st in struct
    AcaEncoderContext cactx;

    //options
    int opt_mode;   //see AudioCodecBitRateControlModeConsts
    int opt_vbrq;   //vbr quality(true vbr, cvbr uses bitrate)
    int opt_cq;     //see AudioCodecQualityConsts

} AcaAacContext;


//supported sample rates(TODO - double check these values)
//Not all modes support all rates!
static const int sample_rates[] = {
    96000, 88200, 48000, 44100,
    32000, 24000, 22050, 16000,
    12000, 11025, 8000, 0
};

static const AVProfile profiles[] = {
    { FF_PROFILE_AAC_LOW, "LC" },
    { FF_PROFILE_AAC_HE, "HE-AAC" },
    { FF_PROFILE_UNKNOWN }
};

static const uint64_t channel_layouts[] = {
    AV_CH_LAYOUT_MONO,
    AV_CH_LAYOUT_STEREO,
    AV_CH_LAYOUT_SURROUND,
    AV_CH_LAYOUT_4POINT0,
    AV_CH_LAYOUT_2_2,
    AV_CH_LAYOUT_QUAD,
    AV_CH_LAYOUT_5POINT0,
    AV_CH_LAYOUT_5POINT1,
    AV_CH_LAYOUT_5POINT0_BACK,
    AV_CH_LAYOUT_5POINT1_BACK,
    AV_CH_LAYOUT_6POINT0,
    AV_CH_LAYOUT_HEXAGONAL,
    AV_CH_LAYOUT_6POINT1,
    AV_CH_LAYOUT_6POINT1_BACK,
    AV_CH_LAYOUT_7POINT0,
    AV_CH_LAYOUT_7POINT1,
    AV_CH_LAYOUT_7POINT1_WIDE,
    AV_CH_LAYOUT_7POINT1_WIDE_BACK,
    AV_CH_LAYOUT_OCTAGONAL,
    0
};

#define AV_CH_LAYOUT_6POINT0_MAP (0)
#define AV_CH_LAYOUT_6POINT1_MAP (AV_CH_LAYOUT_6POINT0_MAP + 1)
#define AV_CH_LAYOUT_7POINT0_MAP (AV_CH_LAYOUT_6POINT1_MAP + 1)
#define AV_CH_LAYOUT_7POINT1_MAP (AV_CH_LAYOUT_7POINT0_MAP + 1)
#define AV_CH_LAYOUT_7POINT1_WIDE_MAP (AV_CH_LAYOUT_7POINT1_MAP + 1)
static const SInt32 channel_maps[][8] = {
    { 2, 0, 1, 4, 5, 3, -1, -1 },       //AV_CH_LAYOUT_6POINT0
    { 2, 0, 1, 5, 6, 4, 3, -1 },        //AV_CH_LAYOUT_6POINT1
    { 2, 0, 1, 5, 6, 3, 4, -1 },        //AV_CH_LAYOUT_7POINT0
    { 2, 0, 1, 6, 7, 4, 5, 3 },         //AV_CH_LAYOUT_7POINT1
    { 2, 4, 5, 0, 1, 6, 7, 3 },         //AV_CH_LAYOUT_7POINT1_WIDE
};

static av_cold int get_channel_map_info(AcaEncoderContext* cactx, const SInt32** chmap, UInt32* in_bitmap, UInt32* out_tag)
{
    switch (*in_bitmap)
    {
    case AV_CH_LAYOUT_MONO: *out_tag = kAudioChannelLayoutTag_Mono; break;
    case AV_CH_LAYOUT_STEREO: *out_tag = kAudioChannelLayoutTag_Stereo; break;
    case AV_CH_LAYOUT_SURROUND: *out_tag = kAudioChannelLayoutTag_AAC_3_0; break;
    case AV_CH_LAYOUT_4POINT0: *out_tag = kAudioChannelLayoutTag_AAC_4_0; break;

    case AV_CH_LAYOUT_2_2:
        *in_bitmap = AV_CH_LAYOUT_QUAD;
    case AV_CH_LAYOUT_QUAD:
        *out_tag = kAudioChannelLayoutTag_AAC_Quadraphonic;
        break;

    case AV_CH_LAYOUT_5POINT0:
        *in_bitmap = AV_CH_LAYOUT_5POINT0_BACK;
    case AV_CH_LAYOUT_5POINT0_BACK:
        *out_tag = kAudioChannelLayoutTag_AAC_5_0;
        break;

    case AV_CH_LAYOUT_5POINT1:
        *in_bitmap = AV_CH_LAYOUT_5POINT1_BACK;
    case AV_CH_LAYOUT_5POINT1_BACK:
        *out_tag = kAudioChannelLayoutTag_AAC_5_1;
        break;

    case AV_CH_LAYOUT_6POINT0:
        *chmap = channel_maps[AV_CH_LAYOUT_6POINT0_MAP];
    case AV_CH_LAYOUT_HEXAGONAL:
        *out_tag = kAudioChannelLayoutTag_AAC_6_0;
        break;

    case AV_CH_LAYOUT_6POINT1:
        *chmap = channel_maps[AV_CH_LAYOUT_6POINT1_MAP];
    case AV_CH_LAYOUT_6POINT1_BACK:
        *out_tag = kAudioChannelLayoutTag_AAC_6_1;
        break;

    case AV_CH_LAYOUT_7POINT0:
        *chmap = channel_maps[AV_CH_LAYOUT_7POINT0_MAP];
        *out_tag = kAudioChannelLayoutTag_AAC_7_0;
        break;

    case AV_CH_LAYOUT_7POINT1:
        *chmap = channel_maps[AV_CH_LAYOUT_7POINT1_MAP];
        *out_tag = kAudioChannelLayoutTag_AAC_7_1;
        break;

    case AV_CH_LAYOUT_7POINT1_WIDE:
        *chmap = channel_maps[AV_CH_LAYOUT_7POINT1_WIDE_MAP];
    case AV_CH_LAYOUT_7POINT1_WIDE_BACK:
        *out_tag = kAudioChannelLayoutTag_AAC_7_1;
        break;

    case AV_CH_LAYOUT_OCTAGONAL:
        *out_tag = kAudioChannelLayoutTag_AAC_Octagonal;
        break;
    }

    return 0;
}

static av_cold int read_descr(uint8_t** buffer, int* tag)
{
    int len = 0;
    int count = 4;
    *tag = *(*buffer)++;
    while (count--)
    {
        int c = *(*buffer)++;
        len = (len << 7) | (c & 0x7f);
        if (!(c & 0x80))
        {
            break;
        }
    }

    return len;
}

//TODO - AOTSpecifcConfig for other channel formats
//TODO - should just rewrite the ASC myself?
//avctx->extradata_size = 0 if not found
//cookie (kAudioConverterCompressionMagicCookie)
#define MP4ESDescrTag 0x03
#define MP4DecConfigDescrTag 0x04
#define MP4DecSpecificDescrTag 0x05
static av_cold int set_audio_specific_config(AVCodecContext* avctx, uint8_t* cookie)
{
    int tag, len;
    int ret = AVERROR_BUG;
    avctx->extradata_size = 0;
    read_descr(&cookie, &tag);
    cookie += 2;    //ID
    if (tag == MP4ESDescrTag)
    {
        cookie++;   //priority
    }

    read_descr(&cookie, &tag);
    if (tag == MP4DecConfigDescrTag)
    {
        cookie++;         // object type id
        cookie++;         // stream type
        cookie += 3;      // buffer size db
        cookie += 4;      // max bitrate
        cookie += 4;      // average bitrate
        len = read_descr(&cookie, &tag);
        if (tag == MP4DecSpecificDescrTag)
        {
            avctx->extradata = av_malloc(len + AV_INPUT_BUFFER_PADDING_SIZE);
            if (!avctx->extradata)
            {
                ret = AVERROR(ENOMEM);
            }
            else
            {
                memcpy(avctx->extradata, cookie, len);
                avctx->extradata_size = len;
                ret = 0;
            }
        }
    }

    return ret;
}

//TODO - something already in libav for these?
static unsigned int copy_bit(PutBitContext* pb, GetBitContext* gb)
{
    unsigned int bit = get_bits1(gb);
    put_bits(pb, 1, bit);
    return bit;
}

static void copy_bits(PutBitContext* pb, GetBitContext* gb, unsigned int num_bits)
{
    while (num_bits >= 32)
    {
        put_bits32(pb, get_bits_long(gb, 32));
        num_bits -= 32;
    }

    if (num_bits)
    {
        put_bits(pb, num_bits, get_bits_long(gb, num_bits));
    }
}

//used in rewrite_asc_with_pce
//BUGBUG: sce_tag and/or cpe_tag will overflow on invalid params
#define PUT_PCE_CHANNEL_ELEMS(x) \
    for (i = 0; i < (x); i++) \
    { \
        tmp = va_arg(argp, unsigned int); \
        tmp = (tmp == 0) ? (sce_tag++) : (0x10 | cpe_tag++); \
        put_bits(pb, 5, tmp); \
    }

/**
* Rewrite ASC with a custom PCE
* On enter, ASC from appleca is in avctx->extradata(with AV_INPUT_BUFFER_PADDING_SIZE)
* On exit, new ASC is written to avctx->extradata(with AV_INPUT_BUFFER_PADDING_SIZE)
*
* ASC = Audio Specific Config
* GASC = General Audio Specific Config(contained within ASC)
* PCE = Program Config Element(contained within GASC)
*
* NOTE: this only handles basic channel elements. assoc data, CC(coupling channel), and mixdowns not handled
* NOTE: args are NOT validated, make sure they are a valid combo when calling!
*/
static av_cold int rewrite_asc_with_pce(AVCodecContext* avctx, int front_elems, int side_elems, int back_elems, int lfe_elems, ...)
{
    va_list argp;
    uint8_t* ascbuf;
    GetBitContext get_bits_ctx;
    PutBitContext put_bits_ctx;
    GetBitContext* gb = &get_bits_ctx;
    PutBitContext* pb = &put_bits_ctx;
    MPEG4AudioConfig m4ac = { 0 };
    int i;
    unsigned int tmp, comment_len;
    unsigned int sce_tag = 0, cpe_tag = 0;
    char comment_buf[250];

    //get channel layout string(comment)
    av_get_channel_layout_string(comment_buf, 250, avctx->channels, avctx->channel_layout);
    av_log(avctx, AV_LOG_VERBOSE, "setting a PCE for channel layout: %s\n", comment_buf);

    //alloc buffer for new asc and init PB
    comment_len = strlen(comment_buf) + 1;  //include null terminator
    i = 64 + comment_len + AV_INPUT_BUFFER_PADDING_SIZE;   //64 is arbitrarily large
    ascbuf = av_malloc(i);
    if (!ascbuf)
    {
        return AVERROR(ENOMEM);
    }

    init_put_bits(pb, ascbuf, i);

    //read and init GB(existing ASC)
    i = avpriv_mpeg4audio_get_config(&m4ac, avctx->extradata, avctx->extradata_size * 8, 1);
    if (i < 0)
    {
        return AVERROR_INVALIDDATA;
    }

    init_get_bits8(gb, avctx->extradata, avctx->extradata_size);
    skip_bits_long(gb, i); //skip to GASC

    //write new ASC...

    //object type
    if (m4ac.object_type > AOT_ESCAPE)
    {
        put_bits(pb, 5, AOT_ESCAPE);
        put_bits(pb, 6, m4ac.object_type - 32);
    }
    else
    {
        put_bits(pb, 5, m4ac.object_type);
    }

    //sampling rate
    put_bits(pb, 4, m4ac.sampling_index);
    if (m4ac.sampling_index == 0x0F)
    {
        put_bits(pb, 24, m4ac.sample_rate);
    }

    //chan_config(zero indicates PCE to follow in GASC)
    put_bits(pb, 4, 0);

    //ext_object_type && ext_sample_rate
    //TODO - handle this and figure out 'sync extension'(see avpriv_mpeg4audio_get_config)
    if (m4ac.ext_object_type != AOT_NULL || m4ac.ext_sample_rate != 0)
    {
        av_log(avctx, AV_LOG_ERROR, "EXT object type and EXT sample index/rate not supported\n");
        return AVERROR_PATCHWELCOME;
    }

    //start GASC, frame length flag
    copy_bit(pb, gb);

    //depends on core coder flag
    if (copy_bit(pb, gb))
    {
        copy_bits(pb, gb, 14);
    }

    //extension flag
    copy_bit(pb, gb);

    //layerNr
    //TODO: WTF is this?  see aacdec_template.c
    if (m4ac.object_type == AOT_AAC_SCALABLE || m4ac.object_type == AOT_ER_AAC_SCALABLE)
    {
        copy_bits(pb, gb, 3);
    }

    //we should now be at the PCE
    if (m4ac.chan_config == 0)
    {
        //need to skip the PCE output from apple converter in GB
        char skipPceBuf[MAX_PCE_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];
        PutBitContext skipPcePb;
        init_put_bits(&skipPcePb, skipPceBuf, 64 + AV_INPUT_BUFFER_PADDING_SIZE);
        avpriv_copy_pce_data(&skipPcePb, gb);
    }

    //PB is ready to start writing new PCE
    //GB is past any existing PCE

    //write pce...
    put_bits(pb, 4, 0); //element instance tag
    put_bits(pb, 2, m4ac.object_type - 1); //TODO - correct?
    put_bits(pb, 4, m4ac.sampling_index);  //TODO - need to handle index 0x0F?
    put_bits(pb, 4, front_elems);
    put_bits(pb, 4, side_elems);
    put_bits(pb, 4, back_elems);
    put_bits(pb, 2, lfe_elems);
    put_bits(pb, 3, 0); //num assoc data
    put_bits(pb, 4, 0); //num cc
    put_bits(pb, 3, 0); //flags(mono/stero/matrix mixdown)

    //channel elements
    va_start(argp, lfe_elems);
    PUT_PCE_CHANNEL_ELEMS(front_elems);
    PUT_PCE_CHANNEL_ELEMS(side_elems);
    PUT_PCE_CHANNEL_ELEMS(back_elems);
    va_end(argp);

    //lfe elements
    for (i = 0; i < lfe_elems; i++)
    {
        put_bits(pb, 4, i);
    }

    //end of PCE, align and put comment
    avpriv_align_put_bits(pb);

    //TODO - output channel layout string as comment??
    put_bits(pb, 8, 0); //comment length
    //put_bits(pb, 8, comment_len); //comment length
    //avpriv_put_string(pb, comment_buf, 1); //comment(with null)
    
    //copy any remaining bits
    copy_bits(pb, gb, get_bits_left(gb));

    //set new ASC
    flush_put_bits(pb);
    avctx->extradata_size = put_bits_count(pb) / 8;
    av_free(avctx->extradata);
    avctx->extradata = ascbuf;

    //dump the new ASC
    av_log(avctx, AV_LOG_DEBUG, "NEW ASC: avctx->extradata_size: %d\n", avctx->extradata_size);
    for (i = 0; i < avctx->extradata_size; i++)
    {
        av_log(avctx, AV_LOG_DEBUG, "NEW ASC: avctx->extradata[%u] = %u\n", i, avctx->extradata[i]);
    }

    return 0;
}

#define EPJMP(expr) FF_APPLECA_EXECP_JMP(cactx->bctx.api->expr, appleca_aac_init_error)
static av_cold int appleca_aac_init(AVCodecContext* avctx)
{
    OSStatus ca_result;
    UInt32 val, size, output_format_id;
    int ret = 0;
    uint8_t* magic_cookie = NULL;
    AcaAacContext* aacctx = avctx->priv_data;
    AcaEncoderContext* cactx = &aacctx->cactx;

    //determine output format
    if (avctx->profile == FF_PROFILE_AAC_HE)
    {
        output_format_id = kAudioFormatMPEG4AAC_HE;
    }
    else
    {
        avctx->profile = FF_PROFILE_AAC_LOW;
        output_format_id = kAudioFormatMPEG4AAC;
    }

    //init encoder
    if ((ret = ff_appleca_enc_init(avctx, output_format_id)) != 0)
    {
        goto appleca_aac_init_error;
    }

    if ((ret = ff_appleca_setup_output_channels(cactx, get_channel_map_info)) != 0)
    {
        goto appleca_aac_init_error;
    }

    //set mode
    val = aacctx->opt_mode;
    av_log(avctx, AV_LOG_VERBOSE, "setting mode: %u\n", val);
    EPJMP(ac_set_prop(cactx->ac, kAudioCodecPropertyBitRateControlMode, sizeof(val), &val));

    //cq(codec quality)
    val = (UInt32)FFMIN(aacctx->opt_cq << 5, kAudioCodecQuality_Max);
    av_log(avctx, AV_LOG_VERBOSE, "setting codec quality: %u(cq = %d)\n", val, aacctx->opt_cq);
    EPJMP(ac_set_prop(cactx->ac, kAudioConverterCodecQuality, sizeof(val), &val));

    //quality/bitrate
    if (aacctx->opt_mode == kAudioCodecBitRateControlMode_Variable)
    {
        val = aacctx->opt_vbrq;
        av_log(avctx, AV_LOG_VERBOSE, "setting VBR quality: %u\n", val);
        EPJMP(ac_set_prop(cactx->ac, kAudioCodecPropertySoundQualityForVBR, sizeof(val), &val));
    }
    else if (avctx->bit_rate <= 0)
    {
        //use whatever the codec defaulted to
        size = sizeof(val);
        EPJMP(ac_get_prop(cactx->ac, kAudioCodecPropertyCurrentTargetBitRate, &size, &val));
        av_log(avctx, AV_LOG_INFO, "bitrate not provided, using codec target: %u\n", val);
        avctx->bit_rate = val;
    }
    else
    {
        //attempt to use user provided bitrate
        val = (UInt32)avctx->bit_rate;
        ca_result = ff_appleca_check_bitrate(cactx, val);
        if (ca_result)
        {
            goto appleca_aac_init_error;
        }

        av_log(avctx, AV_LOG_VERBOSE, "setting bitrate: %u\n", val);
        EPJMP(ac_set_prop(cactx->ac, kAudioConverterEncodeBitRate, sizeof(val), &val));
    }

    //set min delay mode(no latency on encoder, he-aac normally has 1 packet)
    //TODO - not sure how this affects he-aac, implementing the delay is a bit of a PIA!
    val = 1;
    EPJMP(ac_set_prop(cactx->ac, kAudioCodecPropertyMinimumDelayMode, sizeof(val), &val));

    //post init
    if ((ret = ff_appleca_enc_postinit(avctx)) != 0)
    {
        goto appleca_aac_init_error;
    }

    //extradata (audio specific config)
    avctx->extradata_size = 0;
    if (avctx->flags & AV_CODEC_FLAG_GLOBAL_HEADER)
    {
        Boolean writable;

        av_log(avctx, AV_LOG_DEBUG, "setting audio specific config in avctx->extradata\n");

        EPJMP(ac_get_prop_info(cactx->ac, kAudioConverterCompressionMagicCookie, &size, &writable));
        magic_cookie = av_malloc(size);
        if (!magic_cookie)
        {
            ret = AVERROR(ENOMEM);
            goto appleca_aac_init_error;
        }

        EPJMP(ac_get_prop(cactx->ac, kAudioConverterCompressionMagicCookie, &size, magic_cookie));
        if ((ret = set_audio_specific_config(avctx, magic_cookie)) != 0)
        {
            av_log(avctx, AV_LOG_ERROR, "failed to get audio specific config\n");
            goto appleca_aac_init_error;
        }

        //free magic cookie & dump the ASC
        av_freep(&magic_cookie);
        av_log(avctx, AV_LOG_DEBUG, "ASC: avctx->extradata_size: %d\n", avctx->extradata_size);
        for (int i = 0; i < avctx->extradata_size; i++)
        {
            av_log(avctx, AV_LOG_DEBUG, "ASC: avctx->extradata[%u] = %u\n", i, avctx->extradata[i]);
        }

        //check channel layout
        switch (avctx->channel_layout)
        {
        case AV_CH_LAYOUT_2_2: ret = rewrite_asc_with_pce(avctx, 1, 1, 0, 0, 1, 1); break;
        case AV_CH_LAYOUT_5POINT0: ret = rewrite_asc_with_pce(avctx, 2, 1, 0, 0, 0, 1, 1); break;
        case AV_CH_LAYOUT_5POINT1: ret = rewrite_asc_with_pce(avctx, 2, 1, 0, 1, 0, 1, 1); break;
        case AV_CH_LAYOUT_6POINT0: ret = rewrite_asc_with_pce(avctx, 2, 1, 1, 0, 0, 1, 1, 0); break;
        case AV_CH_LAYOUT_6POINT1: ret = rewrite_asc_with_pce(avctx, 2, 1, 1, 1, 0, 1, 1, 0); break;
        case AV_CH_LAYOUT_7POINT0: ret = rewrite_asc_with_pce(avctx, 2, 1, 1, 0, 0, 1, 1, 1); break;
        case AV_CH_LAYOUT_7POINT1: ret = rewrite_asc_with_pce(avctx, 2, 1, 1, 1, 0, 1, 1, 1); break;
        case AV_CH_LAYOUT_7POINT1_WIDE: ret = rewrite_asc_with_pce(avctx, 3, 1, 0, 1, 0, 1, 1, 1); break;
        }

        //check error
        if (ret)
        {
            av_log(avctx, AV_LOG_ERROR, "failed to set audio specific config\n");
            goto appleca_aac_init_error;
        }
    }

    return 0;

appleca_aac_init_error:
    if (magic_cookie)
    {
        av_free(magic_cookie);
    }

    ff_appleca_enc_close(avctx);
    ret = (ret == 0) ? AVERROR_EXTERNAL : ret;
    av_log(avctx, AV_LOG_ERROR, "appleca_aac_init_error return code: %d\n", ret);
    return ret;
}

#define OFFSET(x) offsetof(AcaAacContext, x)
#define OPT_FLAGS (AV_OPT_FLAG_AUDIO_PARAM | AV_OPT_FLAG_ENCODING_PARAM)
static const AVOption appleca_aacenc_options[] = {
    { "mode", "Encoding mode", OFFSET(opt_mode), AV_OPT_TYPE_INT, { .i64 = kAudioCodecBitRateControlMode_Variable }, 0, 3, OPT_FLAGS, "mode" },
    { "abr", "Average bitrate", 0, AV_OPT_TYPE_CONST,{ .i64 = kAudioCodecBitRateControlMode_LongTermAverage }, 0, 0, OPT_FLAGS, "mode" },
    { "cbr", "Constaint bitrate", 0, AV_OPT_TYPE_CONST, { .i64 = kAudioCodecBitRateControlMode_Constant }, 0, 0, OPT_FLAGS, "mode" },
    { "cvbr", "Constrained variable bitrate", 0, AV_OPT_TYPE_CONST, { .i64 = kAudioCodecBitRateControlMode_VariableConstrained }, 0, 0, OPT_FLAGS, "mode" },
    { "tvbr", "True variable bitrate", 0, AV_OPT_TYPE_CONST, { .i64 = kAudioCodecBitRateControlMode_Variable }, 0, 0, OPT_FLAGS, "mode" },
    { "cq", "Codec quality", OFFSET(opt_cq), AV_OPT_TYPE_INT,{ .i64 = 4 }, 0, 4, OPT_FLAGS },
    { "vbrq", "TVBR quality", OFFSET(opt_vbrq), AV_OPT_TYPE_INT, { .i64 = 96 }, 0, 127, OPT_FLAGS },
    { NULL }
};

static const AVCodecDefault appleca_aacenc_defaults[] = {
    { "b", "0" },
    { NULL }
};

static const AVClass appleca_aacenc_class = {
    .class_name = "appleca_aac",
    .item_name = av_default_item_name,
    .option = appleca_aacenc_options,
    .version = LIBAVUTIL_VERSION_INT,
};

AVCodec ff_appleca_aac_encoder = {
    .name = "appleca_aac",
    .long_name = NULL_IF_CONFIG_SMALL("Apple Core Audio AAC"),
    .type = AVMEDIA_TYPE_AUDIO,
    .id = AV_CODEC_ID_AAC,
    .priv_data_size = sizeof(AcaAacContext),
    .init_static_data = ff_appleca_static_init,
    .init = appleca_aac_init,
    .encode2 = ff_appleca_encode,
    .close = ff_appleca_enc_close,
    .capabilities = AV_CODEC_CAP_SMALL_LAST_FRAME | AV_CODEC_CAP_EXPERIMENTAL,
    .sample_fmts = (const enum AVSampleFormat[]) {
        AV_SAMPLE_FMT_S16,
        AV_SAMPLE_FMT_S32,
        AV_SAMPLE_FMT_FLT,
        AV_SAMPLE_FMT_NONE
    },

    .priv_class = &appleca_aacenc_class,
    .defaults = appleca_aacenc_defaults,
    .profiles = profiles,
    .supported_samplerates = sample_rates,
    .channel_layouts = channel_layouts,
};