/*
 * Copyright (c) 2019 kewenyu

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <stdexcept>

#include <VSHelper.h>
#include <VapourSynth.h>

#include "draw.h"

struct DrawData {
    VSNodeRef *node;
    const VSVideoInfo *vi;

    bool process[3];
    std::unique_ptr<float[]> lut[3];
};

template<typename T>
static void process(const VSFrameRef *src, VSFrameRef *dst, const VSFormat *fi, DrawData *d, const VSAPI *vsapi) {
    for (int plane = 0; plane < fi->numPlanes; ++plane) {
        if (!d->process[plane]) {
            continue;
        }

        const T *srcp = reinterpret_cast<const T *>(vsapi->getReadPtr(src, plane));
        int srcStride = vsapi->getStride(src, plane) / sizeof(T);

        T *dstp = reinterpret_cast<T *>(vsapi->getWritePtr(dst, plane));
        int dstStride = vsapi->getStride(dst, plane) / sizeof(T);

        int w = vsapi->getFrameWidth(src, plane);
        int h = vsapi->getFrameHeight(src, plane);

        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                dstp[x] = d->lut[plane][y * w + x];
            }

            dstp += dstStride;
            srcp += srcStride;
        }
    }
}

static void VS_CC drawInit(VSMap *in, VSMap *out, void **instanceData, VSNode *node, VSCore *core, const VSAPI *vsapi) {
    auto *d = static_cast<DrawData *>(*instanceData);
    vsapi->setVideoInfo(d->vi, 1, node);
}

static const VSFrameRef * VS_CC drawGetFrame(int n, int activationReason, void **instanceData, void **frameData, VSFrameContext *frameCtx, VSCore *core, const VSAPI *vsapi) {
    auto *d = static_cast<DrawData *>(*instanceData);

    if (activationReason == arInitial) {
        vsapi->requestFrameFilter(n, d->node, frameCtx);
    } else if (activationReason == arAllFramesReady) {
        const VSFrameRef *src = vsapi->getFrameFilter(n, d->node, frameCtx);
        const VSFormat *fi = d->vi->format;

        int width = d->vi->width;
        int height = d->vi->height;
        int bytesPerSample = d->vi->format->bytesPerSample;

        const VSFrameRef *frames[] = {
                d->process[0] ? nullptr : src,
                d->process[1] ? nullptr : src,
                d->process[2] ? nullptr : src,
        };

        const int processPlane[] = {0, 1, 2};

        VSFrameRef *dst = vsapi->newVideoFrame2(d->vi->format, width, height, frames, processPlane, src, core);

        if (bytesPerSample == 1) {
            process<uint8_t>(src, dst, fi, d, vsapi);
        } else if (bytesPerSample == 2) {
            process<uint16_t>(src, dst, fi, d, vsapi);
        }

        vsapi->freeFrame(src);
        return dst;
    }

    return nullptr;
}

static void VS_CC drawFree(void *instanceData, VSCore *core, const VSAPI *vsapi) {
    auto *d = static_cast<DrawData *>(instanceData);
    vsapi->freeNode(d->node);
    delete d;
}

static void VS_CC drawCreate(const VSMap *in, VSMap *out, void *userData, VSCore *core, const VSAPI *vsapi) {
    std::unique_ptr<DrawData> d(new DrawData);

    try {
        d->node = vsapi->propGetNode(in, "clip", 0, nullptr);
        d->vi = vsapi->getVideoInfo(d->node);

        if (!isConstantFormat(d->vi) || d->vi->format->sampleType != stInteger || d->vi->format->bitsPerSample < 8 ||
            d->vi->format->bitsPerSample > 16) {
            throw std::runtime_error("only constant format of 8bit or 16bit integer input is supported !");
        }

        int numExpr = vsapi->propNumElements(in, "expr");

        if (numExpr > d->vi->format->numPlanes) {
            throw std::runtime_error("too many expression was given !");
        }

        std::string expr[3];
        std::vector<Operator> token[3];

        for (int i = 0; i < numExpr; ++i) {
            expr[i] = vsapi->propGetData(in, "expr", i, nullptr);
            stripRedundantSpace(expr[i]);
            tokenize(expr[i], token[i]);
        }

        for (int i = numExpr; i < 3; ++i) {
            token[i] = token[numExpr - 1];
        }

        for (int i = 0; i < 3; ++i) {
            if (!token[i].empty()) {
                d->process[i] = true;

                int w = d->vi->width;
                int h = d->vi->height;

                if (i != 0) {
                    w = d->vi->format->subSamplingW == 0 ? w : w / (d->vi->format->subSamplingW + 1);
                    h = d->vi->format->subSamplingH == 0 ? h : h / (d->vi->format->subSamplingH + 1);
                }

                d->lut[i].reset(new float[w * h]);

#pragma omp parallel for
                for (int j = 0; j < h; ++j) {
                    for (int k = 0; k < w; ++k) {
                        d->lut[i][j * w + k] = parseExpression(token[i], k, j);
                    }
                }
            }
        }
    } catch (std::runtime_error &e) {
        vsapi->freeNode(d->node);

        std::string errMsg = "Draw: ";
        errMsg += e.what();
        vsapi->setError(out, errMsg.c_str());

        return;
    }

    vsapi->createFilter(in, out, "Draw", drawInit, drawGetFrame, drawFree, fmParallel, 0, d.release(), core);
}

VS_EXTERNAL_API(void) VapourSynthPluginInit(VSConfigPlugin configFunc, VSRegisterFunction registerFunc,
                                            VSPlugin *plugin) {
    configFunc(
            "com.kewenyu.draw",
            "draw",
            "a simple alternative to Masktools2's mt_lutspa",
            VAPOURSYNTH_API_VERSION, 1, plugin);

    registerFunc(
            "Draw",
            "clip:clip;"
            "expr:data[];",
            drawCreate, nullptr, plugin);
}
