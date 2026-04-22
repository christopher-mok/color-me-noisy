#include "image_utils.h"
#include <QImage>
#include <vector>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

Image ImageUtils::readImage(const QString& path, bool isFrame) {
    QImage qimg(path);
    if (qimg.isNull()) {
        return Image();  // Return empty image on failure
    }

    Image img;
    img.width = qimg.width();
    img.height = qimg.height();
    img.isFrame = isFrame;
    img.pixels.resize(img.width * img.height);

    // Convert QImage pixels to RGB floats (0.0-1.0 range)
    for (int y = 0; y < img.height; ++y) {
        for (int x = 0; x < img.width; ++x) {
            QRgb qrbg = qimg.pixel(x, y);
            int index = y * img.width + x;
            img.pixels[index].r = qRed(qrbg) / 255.0f;
            img.pixels[index].g = qGreen(qrbg) / 255.0f;
            img.pixels[index].b = qBlue(qrbg) / 255.0f;
            img.pixels[index].a = qAlpha(qrbg) / 255.0f;
        }
    }

    return img;
}

void ImageUtils::writeImage(Image& img, const QString& path) {
    QImage qimg(img.width, img.height, QImage::Format_ARGB32);
    for (int y = 0; y < img.height; ++y) {
        for (int x = 0; x < img.width; ++x) {
            int index = y * img.width + x;
            RGB pixel = img.pixels[index];
            QRgb qrbg = qRgba(
                static_cast<int>(pixel.r * 255),
                static_cast<int>(pixel.g * 255),
                static_cast<int>(pixel.b * 255),
                //static_cast<int>(pixel.a * 255)
                255
            );
            qimg.setPixel(x, y, qrbg);
        }
    }
    qimg.save(path);
}

std::vector<Image> ImageUtils::readVideo(const QString& path) {
    std::vector<Image> output;

    AVFormatContext* fmtCtx = nullptr;
    if (avformat_open_input(&fmtCtx, path.toUtf8().constData(), nullptr, nullptr) < 0)
        return output;

    if (avformat_find_stream_info(fmtCtx, nullptr) < 0) {
        avformat_close_input(&fmtCtx);
        return output;
    }

    int videoStream = -1;
    for (unsigned i = 0; i < fmtCtx->nb_streams; ++i) {
        if (fmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStream = (int)i;
            break;
        }
    }
    if (videoStream < 0) {
        avformat_close_input(&fmtCtx);
        return output;
    }

    AVCodecParameters* codecPar = fmtCtx->streams[videoStream]->codecpar;
    const AVCodec* codec = avcodec_find_decoder(codecPar->codec_id);
    AVCodecContext* codecCtx = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(codecCtx, codecPar);
    avcodec_open2(codecCtx, codec, nullptr);

    int w = codecCtx->width, h = codecCtx->height;
    SwsContext* swsCtx = sws_getContext(
        w, h, codecCtx->pix_fmt,
        w, h, AV_PIX_FMT_RGBA,
        SWS_BILINEAR, nullptr, nullptr, nullptr);

    AVFrame* frame    = av_frame_alloc();
    AVFrame* rgbaFrame = av_frame_alloc();
    std::vector<uint8_t> rgbaBuf(av_image_get_buffer_size(AV_PIX_FMT_RGBA, w, h, 1));
    av_image_fill_arrays(rgbaFrame->data, rgbaFrame->linesize,
                         rgbaBuf.data(), AV_PIX_FMT_RGBA, w, h, 1);

    AVPacket* pkt = av_packet_alloc();
    int frameNumber = 0;
    while (av_read_frame(fmtCtx, pkt) >= 0) {
        if (pkt->stream_index != videoStream) {
            av_packet_unref(pkt);
            continue;
        }
        if (avcodec_send_packet(codecCtx, pkt) < 0) {
            av_packet_unref(pkt);
            continue;
        }
        while (avcodec_receive_frame(codecCtx, frame) == 0) {
            sws_scale(swsCtx, frame->data, frame->linesize, 0, h,
                      rgbaFrame->data, rgbaFrame->linesize);

            Image img;
            img.width = w;
            img.height = h;
            img.isFrame = true;
            img.frameNumber = frameNumber++;
            img.pixels.resize(w * h);

            for (int y = 0; y < h; ++y) {
                const uint8_t* row = rgbaFrame->data[0] + y * rgbaFrame->linesize[0];
                for (int x = 0; x < w; ++x) {
                    int idx = y * w + x;
                    img.pixels[idx].r = row[x * 4 + 0] / 255.0f;
                    img.pixels[idx].g = row[x * 4 + 1] / 255.0f;
                    img.pixels[idx].b = row[x * 4 + 2] / 255.0f;
                    img.pixels[idx].a = row[x * 4 + 3] / 255.0f;
                }
            }
            output.push_back(std::move(img));
        }
        av_packet_unref(pkt);
    }

    av_packet_free(&pkt);
    av_frame_free(&frame);
    av_frame_free(&rgbaFrame);
    sws_freeContext(swsCtx);
    avcodec_free_context(&codecCtx);
    avformat_close_input(&fmtCtx);

    return output;
}

RGB ImageUtils::rgbAt(const Image& img, int x, int y){
    int ind = img.width * y + x;
    RGB rgbVal = img.pixels[ind];
    return rgbVal;
}
