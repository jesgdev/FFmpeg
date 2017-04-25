/*
 * DVD (libdvdnav) protocol
 *
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

#include <dvdnav/dvdnav.h>

#include "libavutil/avstring.h"
#include "libavformat/avformat.h"
#include "libavformat/url.h"
#include "libavutil/opt.h"

#define DVD_MIN_TITLE_DURATION 90000 * 15 //15 seconds
#define DVD_PROTO_PREFIX "dvd:"

typedef struct
{
    const AVClass *class;

    dvdnav_t *nav;

    uint8_t block_buf[DVD_VIDEO_LB_LEN];
    int block_len;
    int title_end;

    int32_t title;  //selected title

} DvdContext;

#define OFFSET(x) offsetof(DvdContext, x)
static const AVOption options[] = {
    { "title", "", OFFSET(title), AV_OPT_TYPE_INT, {.i64 = 0 }, 0, 99, AV_OPT_FLAG_DECODING_PARAM },
    { NULL }
};

static const AVClass dvd_context_class = {
    .class_name     = "dvd",
    .item_name      = av_default_item_name,
    .option         = options,
    .version        = LIBAVUTIL_VERSION_INT,
};

static void print_title_info(URLContext *h, int32_t title, int32_t chapter_count, uint64_t duration, uint64_t* chapter_times)
{
    av_log(h, AV_LOG_INFO, "title %d: %d chapters (%d:%02d:%02d)\n",
        title,
        chapter_count,
        ((int)(duration / 90000) / 3600),
        (((int)(duration / 90000) % 3600) / 60),
        ((int)(duration / 90000) % 60));

    if (chapter_times)
    {
        //TODO: print chapter times...
    }
}

static int check_title(URLContext *h, int32_t title)
{
    DvdContext* dvd = h->priv_data;
    uint64_t* chapter_times = NULL;
    uint64_t duration;
    int32_t chapter_count = dvdnav_describe_title_chapters(dvd->nav, title, &chapter_times, &duration);
    if (!chapter_times)
    {
        av_log(h, AV_LOG_ERROR, "title %d is invalid\n", title);
        return AVERROR(EINVAL);
    }

    print_title_info(h, title, chapter_count, duration, chapter_times);

    //TODO: print chapter times?

    free(chapter_times);

    return 0;
}

static int dvd_close(URLContext *h)
{
    DvdContext *dvd = h->priv_data;
    if (dvd->nav)
    {
        dvdnav_close(dvd->nav);
        dvd->nav = NULL;
    }

    return 0;
}

#define ON_DVD_OPEN_ERROR(result) dvd_close(h); return (result)
static int dvd_open(URLContext *h, const char *path, int flags)
{
    DvdContext *dvd = h->priv_data;
    const char *title = NULL, *serial = NULL;
    int32_t num_titles;
    int result;
    const char *diskpath = path;

    h->max_packet_size = DVD_VIDEO_LB_LEN;
    av_strstart(path, DVD_PROTO_PREFIX, &diskpath);

    //open the disc
    if (dvdnav_open(&dvd->nav, diskpath) != DVDNAV_STATUS_OK)
    {
        av_log(h, AV_LOG_ERROR, "Error on dvdnav_open: %s\n", dvdnav_err_to_string(dvd->nav));
        ON_DVD_OPEN_ERROR(AVERROR(EIO));
    }

    //set cache usage(TODO: turning off but maybe should be on, test on actual DVD disk in drive!)
    if (dvdnav_set_readahead_flag(dvd->nav, 0) != DVDNAV_STATUS_OK)
    {
        av_log(h, AV_LOG_ERROR, "Error on dvdnav_set_readahead_flag: %s\n", dvdnav_err_to_string(dvd->nav));
        ON_DVD_OPEN_ERROR(AVERROR(EIO));
    }

    //set the PGC positioning flag to have position information relatively to the
    //whole feature instead of just relatively to the current chapter
    if (dvdnav_set_PGC_positioning_flag(dvd->nav, 1) != DVDNAV_STATUS_OK)
    {
        av_log(h, AV_LOG_ERROR, "Error on dvdnav_set_PGC_positioning_flag: %s\n", dvdnav_err_to_string(dvd->nav));
        ON_DVD_OPEN_ERROR(AVERROR(EIO));
    }

    //get title
    if (dvdnav_get_title_string(dvd->nav, &title) != DVDNAV_STATUS_OK)
    {
        av_log(h, AV_LOG_ERROR, "Error on dvdnav_get_title_string: %s\n", dvdnav_err_to_string(dvd->nav));
        ON_DVD_OPEN_ERROR(AVERROR(EIO));
    }

    //get serial
    if (dvdnav_get_serial_string(dvd->nav, &serial) != DVDNAV_STATUS_OK)
    {
        av_log(h, AV_LOG_ERROR, "Error on dvdnav_get_serial_string: %s\n", dvdnav_err_to_string(dvd->nav));
        ON_DVD_OPEN_ERROR(AVERROR(EIO));
    }

    //get number of titles
    if (dvdnav_get_number_of_titles(dvd->nav, &num_titles) != DVDNAV_STATUS_OK)
    {
        av_log(h, AV_LOG_ERROR, "Error on dvdnav_get_number_of_titles: %s\n", dvdnav_err_to_string(dvd->nav));
        ON_DVD_OPEN_ERROR(AVERROR(EIO));
    }

    //print disc info
    av_log(h, AV_LOG_INFO, "DVD disc title: %s, serial: %s, titles %d\n",
        (!title || !*title) ? "<null>" : title,
        (!serial || !*serial) ? "<null>" : serial,
        num_titles);

    //select title if needed
    if (dvd->title < 1)
    {
        uint64_t* chapter_times = NULL;
        uint64_t title_duration;
        uint64_t longest_title_duration = 0;

        av_log(h, AV_LOG_INFO, "title not given, finding longest...\n");

        for (int32_t i = 1; i <= num_titles; i++)
        {
            int32_t chapter_count = dvdnav_describe_title_chapters(dvd->nav, i, &chapter_times, &title_duration);
            if (chapter_times)
            {
                if (title_duration >= DVD_MIN_TITLE_DURATION)
                {
                    print_title_info(h, i, chapter_count, title_duration, NULL);
                    if (title_duration > longest_title_duration)
                    {
                        dvd->title = i;
                        longest_title_duration = title_duration;
                    }
                }

                free(chapter_times);
                chapter_times = NULL;
            }
        }

        if (dvd->title < 1)
        {
            av_log(h, AV_LOG_ERROR, "failed to find a suitable title\n");
            ON_DVD_OPEN_ERROR(AVERROR(EINVAL));
        }
    }

    //check title
    av_log(h, AV_LOG_INFO, "selecting title %d\n", dvd->title);
    if ((result = check_title(h, dvd->title)) != 0)
    {
        ON_DVD_OPEN_ERROR(result);
    }

    //start the title
    if (dvdnav_title_play(dvd->nav, dvd->title) != DVDNAV_STATUS_OK)
    {
        av_log(h, AV_LOG_ERROR, "Error on dvdnav_title_play: %s\n", dvdnav_err_to_string(dvd->nav));
        ON_DVD_OPEN_ERROR(AVERROR(EIO));
    }

    return 0;
}

static int read_block(URLContext* h, uint8_t* buf, int is_buf_cache)
{
    DvdContext *dvd = h->priv_data;
    int result, event, len;

    //if we have a cached block(from a seek) use that
    if (!is_buf_cache && dvd->block_len)
    {
        len = dvd->block_len;
        dvd->block_len = 0;
        memcpy(buf, dvd->block_buf, len);
        return len;
    }

    //no cached block or we are reading into cache
    dvd->block_len = 0;
    result = dvdnav_get_next_block(dvd->nav, buf, &event, &len);
    if (result != DVDNAV_STATUS_OK)
    {
        av_log(h, AV_LOG_ERROR, "Error on dvdnav_get_next_block: %s\n", dvdnav_err_to_string(dvd->nav));
        return AVERROR(EIO);
    }

    switch (event)
    {
    //block of mpeg data
    case DVDNAV_BLOCK_OK:
    {
        if (is_buf_cache)
        {
            dvd->block_len = len;
        }

        return len;
    }
        
    //EOF
    case DVDNAV_STOP:
        dvd->title_end = 1;
        return 0;

    case DVDNAV_CELL_CHANGE:
    {
        int32_t title = 0, part = 0;
        dvdnav_current_title_info(dvd->nav, &title, &part);
        if (title != dvd->title)
        {
            dvd->title_end = 1;
            return 0;
        }

        return AVERROR(EAGAIN);
    } break;

    //TODO: can pass these along somehow?  need to?
    case DVDNAV_SPU_CLUT_CHANGE: //Player applications should pass the new colour lookup table to their SPU decoder
    case DVDNAV_SPU_STREAM_CHANGE: //Player applications should inform their SPU decoder to switch channels
    case DVDNAV_AUDIO_STREAM_CHANGE: //Player applications should inform their audio decoder to switch channels
    case DVDNAV_HIGHLIGHT:  //Player applications should inform their overlay engine to highlight the given button
    case DVDNAV_VTS_CHANGE: //?
    case DVDNAV_NAV_PACKET: //has buttons
    case DVDNAV_HOP_CHANNEL: //?
    case DVDNAV_NOP:
        return AVERROR(EAGAIN);

    case DVDNAV_STILL_FRAME:
        //dvdnav_still_skip(dvd->nav);
        av_log(h, AV_LOG_ERROR, "Unexpected event DVDNAV_STILL_FRAME\n");
        return AVERROR(EIO);

    case DVDNAV_WAIT:
        //dvdnav_wait_skip(dvd->nav);
        av_log(h, AV_LOG_ERROR, "Unexpected event DVDNAV_WAIT\n");
        return AVERROR(EIO);
    }

    av_log(h, AV_LOG_ERROR, "read_block unhandled event %d", event);
    return AVERROR_BUG;
}

static int dvd_read(URLContext *h, unsigned char *buf, int size)
{
    DvdContext *dvd = h->priv_data;
    if (size < DVD_VIDEO_LB_LEN)
    {
        av_log(h, AV_LOG_ERROR, "dvd_read size %d to small, expected at least %d", size, DVD_VIDEO_LB_LEN);
        return AVERROR(EINVAL);
    }

    return (dvd->title_end) ? 0 : read_block(h, buf, 0);
}


//TODO - this really doesn't seem correct(seeking)
//TODO - not sure byte size is correct either
//TODO - expected behavior at end?  currently ffplay just sits there and you can't seek since title_end is set
static int64_t dvd_seek(URLContext *h, int64_t pos, int whence)
{
    uint32_t position, len;
    DvdContext *dvd = h->priv_data;
    if (!dvd || !dvd->nav)
    {
        return AVERROR(EFAULT);
    }

    switch (whence)
    {
    case SEEK_SET:
    case SEEK_CUR:
    case SEEK_END:
    {
        av_log(h, AV_LOG_DEBUG, "SEEK: pos %"PRId64", whence %d\n", pos, whence);

        //seek
        //TODO: is a sector same as a block?
        if (dvdnav_sector_search(dvd->nav, pos / DVD_VIDEO_LB_LEN, whence) != DVDNAV_STATUS_OK)
        {
            av_log(h, AV_LOG_ERROR, "Error on dvdnav_sector_search: %s\n", dvdnav_err_to_string(dvd->nav));
            return AVERROR(EIO);
        }

        //read next mpeg block(or EOF), avoids 'New position not yet determined'
        //errors on back to back seeks
        while (!dvd->title_end && !dvd->block_len)
        {
            read_block(h, dvd->block_buf, 1);
        }

        //get updated position
        if (dvdnav_get_position(dvd->nav, &position, &len) != DVDNAV_STATUS_OK)
        {
            av_log(h, AV_LOG_ERROR, "Error on dvdnav_get_position: %s\n", dvdnav_err_to_string(dvd->nav));
            return AVERROR(EIO);
        }

        //the above read will move us ahead by 1, take it back!
        if (position != 0)  //TODO: this can probably be an assert as it should never be zero?
        {
            position--;
        }

        av_log(h, AV_LOG_DEBUG, "SEEK: Got position %u, length %u\n", position, len);
        return position * DVD_VIDEO_LB_LEN;
    }

    case AVSEEK_SIZE:
    {
        if (dvdnav_get_position(dvd->nav, &position, &len) != DVDNAV_STATUS_OK)
        {
            av_log(h, AV_LOG_ERROR, "Error on dvdnav_get_position: %s\n", dvdnav_err_to_string(dvd->nav));
            return AVERROR(EIO);
        }

        av_log(h, AV_LOG_DEBUG, "AVSEEK_SIZE: Got position %u, length %u\n", position, len);
        return len * DVD_VIDEO_LB_LEN;
    }

    }

    av_log(h, AV_LOG_ERROR, "Unsupported whence operation %d\n", whence);
    return AVERROR(EINVAL);
}

const URLProtocol ff_dvd_protocol = {
    .name = "dvd",
    .url_close = dvd_close,
    .url_open = dvd_open,
    .url_read = dvd_read,
    .url_seek = dvd_seek,
    .priv_data_size = sizeof(DvdContext),
    .priv_data_class = &dvd_context_class
};