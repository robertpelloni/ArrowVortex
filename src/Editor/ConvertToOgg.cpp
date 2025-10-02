/**
 * @file demuxing, decoding, filtering, encoding and muxing API usage example
 * @example transcode.c
 *
 * Convert input to output file, applying some hard-coded filter-graph on both
 * audio and video streams.
 *
 * Adapted from
 * https://github.com/FFmpeg/FFmpeg/blob/master/doc/examples/transcode.c
 */

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/channel_layout.h>
#include <libavutil/mem.h>
#include <libavutil/opt.h>
#include <libavutil/pixdesc.h>
}

#include <Editor/ConvertToOgg.h>
#include <Editor/Music.h>

#include <Core/Utils.h>
#include <Core/StringUtils.h>

#include <System/System.h>

#include <stdint.h>
#include <string.h>

namespace Vortex {

static AVFormatContext *ifmt_ctx;
static AVFormatContext *ofmt_ctx;
// typedef struct FilteringContext {
//     AVFilterContext *buffersink_ctx;
//     AVFilterContext *buffersrc_ctx;
//     AVFilterGraph *filter_graph;
//
//     AVPacket *enc_pkt;
//     AVFrame *filtered_frame;
// } FilteringContext;
// static FilteringContext *filter_ctx;

typedef struct StreamContext {
    AVCodecContext *dec_ctx;
    AVCodecContext *enc_ctx;

    AVFrame *dec_frame;
} StreamContext;
static StreamContext *stream_ctx;

static int open_input_file(const char *filename) {
    int ret;
    unsigned int i;

    ifmt_ctx = nullptr;
    if ((ret = avformat_open_input(&ifmt_ctx, filename, nullptr, nullptr)) <
        0) {
        HudError("Cannot open input file");
        return ret;
    }

    if ((ret = avformat_find_stream_info(ifmt_ctx, nullptr)) < 0) {
        HudError("Cannot find stream information");
        return ret;
    }

    stream_ctx = reinterpret_cast<StreamContext *>(
        av_calloc(ifmt_ctx->nb_streams, sizeof(*stream_ctx)));
    if (!stream_ctx) return AVERROR(ENOMEM);

    for (i = 0; i < ifmt_ctx->nb_streams; i++) {
        AVStream *stream = ifmt_ctx->streams[i];
        const AVCodec *dec = avcodec_find_decoder(stream->codecpar->codec_id);
        AVCodecContext *codec_ctx;
        if (!dec) {
            HudError("Failed to find decoder for stream #%u\n", i);
            return AVERROR_DECODER_NOT_FOUND;
        }
        codec_ctx = avcodec_alloc_context3(dec);
        if (!codec_ctx) {
            HudError("Failed to allocate the decoder context for stream #%u\n",
                     i);
            return AVERROR(ENOMEM);
        }
        ret = avcodec_parameters_to_context(codec_ctx, stream->codecpar);
        if (ret < 0) {
            HudError(
                "Failed to copy decoder parameters to input decoder context "
                "for stream #%u\n",
                i);
            return ret;
        }

        /* Inform the decoder about the timebase for the packet timestamps.
         * This is highly recommended, but not mandatory. */
        codec_ctx->pkt_timebase = stream->time_base;

        /* Reencode video & audio and remux subtitles etc. */
        if (codec_ctx->codec_type == AVMEDIA_TYPE_VIDEO ||
            codec_ctx->codec_type == AVMEDIA_TYPE_AUDIO) {
            if (codec_ctx->codec_type == AVMEDIA_TYPE_VIDEO)
                codec_ctx->framerate =
                    av_guess_frame_rate(ifmt_ctx, stream, nullptr);
            /* Open decoder */
            ret = avcodec_open2(codec_ctx, dec, nullptr);
            if (ret < 0) {
                HudError("Failed to open decoder for stream #%u\n", i);
                return ret;
            }
        }
        stream_ctx[i].dec_ctx = codec_ctx;

        stream_ctx[i].dec_frame = av_frame_alloc();
        if (!stream_ctx[i].dec_frame) return AVERROR(ENOMEM);
    }

    av_dump_format(ifmt_ctx, 0, filename, 0);
    return 0;
}

static int open_output_file(const char *filename) {
    AVStream *out_stream;
    AVStream *in_stream;
    AVCodecContext *dec_ctx, *enc_ctx;
    const AVCodec *encoder;
    int ret;
    unsigned int i;

    ofmt_ctx = nullptr;
    avformat_alloc_output_context2(&ofmt_ctx, nullptr, nullptr, filename);
    if (!ofmt_ctx) {
        HudError("Could not create output context");
        return AVERROR_UNKNOWN;
    }

    for (i = 0; i < ifmt_ctx->nb_streams; i++) {
        out_stream = avformat_new_stream(ofmt_ctx, nullptr);
        if (!out_stream) {
            HudError("Failed allocating output stream");
            return AVERROR_UNKNOWN;
        }

        in_stream = ifmt_ctx->streams[i];
        dec_ctx = stream_ctx[i].dec_ctx;

        if (dec_ctx->codec_type == AVMEDIA_TYPE_VIDEO ||
            dec_ctx->codec_type == AVMEDIA_TYPE_AUDIO) {
            encoder = avcodec_find_encoder(AV_CODEC_ID_VORBIS);
            if (!encoder) {
                HudError("Bug: Vorbis encoder not found");
                return AVERROR_INVALIDDATA;
            }
            enc_ctx = avcodec_alloc_context3(encoder);
            if (!enc_ctx) {
                HudError("Failed to allocate the encoder context");
                return AVERROR(ENOMEM);
            }

            const enum AVSampleFormat *sample_fmts = nullptr;

            enc_ctx->sample_rate = dec_ctx->sample_rate;
            ret = av_channel_layout_copy(&enc_ctx->ch_layout,
                                         &dec_ctx->ch_layout);
            if (ret < 0) return ret;

            ret = avcodec_get_supported_config(
                dec_ctx, nullptr, AV_CODEC_CONFIG_SAMPLE_FORMAT, 0,
                reinterpret_cast<const void **>(&sample_fmts), nullptr);

            enc_ctx->sample_fmt = AV_SAMPLE_FMT_FLTP;

            enc_ctx->time_base.num = 1;
            enc_ctx->time_base.den = enc_ctx->sample_rate;
            // enc_ctx->flags |= AV_CODEC_FLAG_QSCALE;
            // enc_ctx->global_quality = 7;

            if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
                enc_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

            /* Third parameter can be used to pass settings to encoder */
            ret = avcodec_open2(enc_ctx, encoder, nullptr);
            if (ret < 0) {
                HudError("Cannot open %s encoder for stream #%u", encoder->name,
                         i);
                return ret;
            }
            ret =
                avcodec_parameters_from_context(out_stream->codecpar, enc_ctx);
            if (ret < 0) {
                HudError(
                    "Failed to copy encoder parameters to output stream #%u",
                    i);
                return ret;
            }

            out_stream->time_base = enc_ctx->time_base;
            stream_ctx[i].enc_ctx = enc_ctx;
        } else if (dec_ctx->codec_type == AVMEDIA_TYPE_UNKNOWN) {
            HudError("Elementary stream #%d is of unknown type, cannot proceed",
                     i);
            return AVERROR_INVALIDDATA;
        } else {
            /* if this stream must be remuxed */
            ret = avcodec_parameters_copy(out_stream->codecpar,
                                          in_stream->codecpar);
            if (ret < 0) {
                HudError("Copying parameters for stream #%u failed", i);
                return ret;
            }
            out_stream->time_base = in_stream->time_base;
        }
    }
    av_dump_format(ofmt_ctx, 0, filename, 1);

    if (!(ofmt_ctx->oformat->flags & AVFMT_NOFILE)) {
        ret = avio_open(&ofmt_ctx->pb, filename, AVIO_FLAG_WRITE);
        if (ret < 0) {
            HudError("Could not open output file '%s'", filename);
            return ret;
        }
    }

    /* init muxer, write output file header */
    ret = avformat_write_header(ofmt_ctx, nullptr);
    if (ret < 0) {
        HudError("Error occurred when opening output file");
        return ret;
    }

    return 0;
}

static int encode_write_frame(unsigned int stream_index, AVFrame *frame,
                              int flush) {
    StreamContext *stream = &stream_ctx[stream_index];
    AVPacket *enc_pkt = av_packet_alloc();
    int ret;

    // HudNote("Encoding frame");
    /* encode filtered frame */
    av_packet_unref(enc_pkt);

    if (frame && frame->pts != AV_NOPTS_VALUE)
        frame->pts = av_rescale_q(frame->pts, frame->time_base,
                                  stream->enc_ctx->time_base);

    ret = avcodec_send_frame(stream->enc_ctx, frame);

    if (ret < 0) return ret;

    while (ret >= 0) {
        ret = avcodec_receive_packet(stream->enc_ctx, enc_pkt);

        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) return 0;

        /* prepare packet for muxing */
        enc_pkt->stream_index = stream_index;
        av_packet_rescale_ts(enc_pkt, stream->enc_ctx->time_base,
                             ofmt_ctx->streams[stream_index]->time_base);

        // HudNote("Muxing frame");
        /* mux encoded frame */
        ret = av_interleaved_write_frame(ofmt_ctx, enc_pkt);
    }

    return ret;
}

static int filter_encode_write_frame(AVFrame *frame,
                                     unsigned int stream_index) {
    int ret = 0;

    /* pull filtered frames from the filtergraph */
    while (ret >= 0) {
        frame->pict_type = AV_PICTURE_TYPE_NONE;
        ret = encode_write_frame(stream_index, frame, 0);
        av_frame_unref(frame);
    }

    return ret;
}

static int flush_encoder(unsigned int stream_index) {
    if (!(stream_ctx[stream_index].enc_ctx->codec->capabilities &
          AV_CODEC_CAP_DELAY))
        return 0;

    // HudNote("Flushing stream #%u encoder\n", stream_index);
    return encode_write_frame(stream_index, stream_ctx[stream_index].dec_frame,
                              1);
}

OggConversionThread::OggConversionThread() { progress = 0; }

void OggConversionThread::exec() {
    int ret = 0;
    AVPacket *packet = nullptr;
    unsigned int stream_index;
    unsigned int i;

    if (!outPath.ends_with(".ogg")) {
        HudError("Bug: tried to save to a file without an .ogg extension");
        return;
    }
    if ((ret = open_input_file(inPath.c_str()) >= 0) &&
        (ret = open_output_file(outPath.c_str()) >= 0) &&
        (packet = av_packet_alloc())) {
        /* read all packets */
        while (true) {
            if ((ret = av_read_frame(ifmt_ctx, packet)) < 0) break;
            stream_index = packet->stream_index;
            // HudNote("Demuxer gave frame of stream_index %u", stream_index);
            StreamContext *stream = &stream_ctx[stream_index];

            ret = avcodec_send_packet(stream->dec_ctx, packet);
            if (ret < 0) {
                HudError("Decoding failed");
                break;
            }

            while (ret >= 0) {
                ret = avcodec_receive_frame(stream->dec_ctx, stream->dec_frame);
                if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN) || ret < 0) {
                    break;
                }

                stream->dec_frame->pts =
                    stream->dec_frame->best_effort_timestamp;
                ret =
                    filter_encode_write_frame(stream->dec_frame, stream_index);
                if (ret < 0) break;
            }
            av_packet_unref(packet);
        }

        /* flush decoders, filters and encoders */
        if (ret >= 0) {
            for (i = 0; i < ifmt_ctx->nb_streams; i++) {
                StreamContext *stream;

                stream = &stream_ctx[i];

                // HudNote("Flushing stream %u decoder", i);

                /* flush decoder */
                ret = avcodec_send_packet(stream->dec_ctx, nullptr);
                if (ret < 0) {
                    HudError("Flushing decoding failed");
                    break;
                }

                while (ret >= 0) {
                    ret = avcodec_receive_frame(stream->dec_ctx,
                                                stream->dec_frame);
                    if (ret == AVERROR_EOF || ret < 0) {
                        break;
                    }

                    stream->dec_frame->pts =
                        stream->dec_frame->best_effort_timestamp;
                    ret = filter_encode_write_frame(stream->dec_frame, i);
                    if (ret < 0) break;
                }

                /* flush filter */
                ret = filter_encode_write_frame(nullptr, i);
                if (ret < 0) {
                    HudError("Flushing filter failed");
                    break;
                }

                /* flush encoder */
                ret = flush_encoder(i);
                if (ret < 0) {
                    HudError("Flushing encoder failed");
                    break;
                }
            }
        }
        if (ret >= 0) {
            av_write_trailer(ofmt_ctx);
        }
    }
    av_packet_free(&packet);
    for (i = 0; i < ifmt_ctx->nb_streams; i++) {
        avcodec_free_context(&stream_ctx[i].dec_ctx);
        if (ofmt_ctx && ofmt_ctx->nb_streams > i && ofmt_ctx->streams[i] &&
            stream_ctx[i].enc_ctx)
            avcodec_free_context(&stream_ctx[i].enc_ctx);

        av_frame_free(&stream_ctx[i].dec_frame);
    }
    av_free(stream_ctx);
    avformat_close_input(&ifmt_ctx);
    if (ofmt_ctx && !(ofmt_ctx->oformat->flags & AVFMT_NOFILE))
        avio_closep(&ofmt_ctx->pb);
    avformat_free_context(ofmt_ctx);

    if (ret < 0) {
        char error_buffer[AV_ERROR_MAX_STRING_SIZE];
        av_make_error_string(error_buffer, AV_ERROR_MAX_STRING_SIZE, ret);
        HudError("Error occurred: %s", error_buffer);
    }
}

};  // namespace Vortex