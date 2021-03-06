#include "prelu_layer.h"
#include "arm/generic_kernels.h"

namespace feather
{
int PReluLayer::Init(float *ginput, float *goutput, float *ginputMuti)
{
    if ((NULL != ginput) && (NULL != ginput))
    {
        ((Blob<float> *)_bottom_blobs[_bottom[0]])->setData(ginput);
        ((Blob<float> *)_top_blobs[_top[0]])->setData(goutput);
    }

    input = _bottom_blobs[_bottom[0]]->data();
    output = _top_blobs[_top[0]]->data();
    n = _bottom_blobs[_bottom[0]]->num();
    c = _bottom_blobs[_bottom[0]]->channels();
    h = _bottom_blobs[_bottom[0]]->height();
    w = _bottom_blobs[_bottom[0]]->width();

    return 0;
}

int PReluLayer::Forward()
{
    if ((0 == c) && (0 == h) && (0 != w))
    {
        int i = 0;
        if (shared)
        {
            float slope = slope_data[0];
#if 0//def __ARM_NEON
            float32x4_t vzerof32x4 = vdupq_n_f32(0.f);
            float32x4_t vslopef32x4 = vdupq_n_f32(slope);
            for (; i < w; i += 4)
            {
                float32x4_t vsrcf32x4 = vld1q_f32(&input[i]);
                uint32x4_t vmasku32x4 = vcleq_f32(vsrcf32x4, vzerof32x4);
                float32x4_t vmulf32x4 = vmulq_f32(vsrcf32x4, vslopef32x4);
                vmulf32x4 = vbslq_f32(vmasku32x4, vmulf32x4, vsrcf32x4);
                vst1q_f32(&output[i], vmulf32x4);
            }
            if (i > w) i -= 4;
#endif
            for (; i < w; i++)
            {
                if (input[i] < 0)
                    output[i] = input[i]*slope;
                else
                    output[i] = input[i];
            }
        }
        else
        {
#if 0//def __ARM_NEON
            float32x4_t vzerof32x4 = vdupq_n_f32(0.f);
            for (; i < w; i += 4)
            {
                float32x4_t vslopef32x4 = vld1q_f32(&slope_data[i]);
                float32x4_t vsrcf32x4 = vld1q_f32(&input[i]);
                uint32x4_t vmasku32x4 = vcleq_f32(vsrcf32x4, vzerof32x4);
                float32x4_t vmulf32x4 = vmulq_f32(vsrcf32x4, vslopef32x4);
                vmulf32x4 = vbslq_f32(vmasku32x4, vmulf32x4, vsrcf32x4);
                vst1q_f32(&output[i], vmulf32x4);
            }
            if (i > w) i -= 4;
#endif
            for (; i < w; i++)
            {
                if (input[i] < 0)
                    output[i] = input[i]*slope_data[i];
                else
                    output[i] = input[i];
            }
        }
    }
    else if ((0 == c) && (0 != h) && (0 != w))
    {
        for (int i=0; i<h; i++)
        {
            const float* inPtr = input + i*w;
            float* outPtr = output + i*w;
            float slope = shared ? slope_data[0]:slope_data[i];
            int j = 0;
#if 0//def __ARM_NEON
            float32x4_t vzerof32x4 = vdupq_n_f32(0.f);
            float32x4_t vslopef32x4 = vdupq_n_f32(slope);
            for (; j < w; j += 4)
            {
                float32x4_t vsrcf32x4 = vld1q_f32(&inPtr[j]);
                uint32x4_t vmasku32x4 = vcleq_f32(vsrcf32x4, vzerof32x4);
                float32x4_t vmulf32x4 = vmulq_f32(vsrcf32x4, vslopef32x4);
                vmulf32x4 = vbslq_f32(vmasku32x4, vmulf32x4, vsrcf32x4);
                vst1q_f32(&outPtr[j], vmulf32x4);
            }
            if (j > w) j -= 4;
#endif
            for (; j < w; j++)
            {
                if (inPtr[j] < 0)
                    outPtr[j] = inPtr[j]*slope;
                else
                    outPtr[j] = inPtr[j];
            }
        }
    }
    else if ((0 != c) && (0 != h) && (0 != w))
    {
        int size = w * h;
        float32x4_t vzerof32x4 = vdupq_n_f32(0.f);

        #pragma omp parallel for num_threads(num_threads)
        for (int q=0; q<c; q++)
        {
            const float* inPtr = input + q*size;
            float* outPtr = output + q*size;
            float slope = shared ? slope_data[0]:slope_data[q];
            int i = 0;
#if 0//def __ARM_NEON
            float32x4_t vslopef32x4 = vdupq_n_f32(slope);
            for (; i < size - 4; i += 4)
            {
                float32x4_t vsrcf32x4 = vld1q_f32(&inPtr[i]);
                uint32x4_t vmasku32x4 = vcleq_f32(vsrcf32x4, vzerof32x4);
                float32x4_t vmulf32x4 = vmulq_f32(vsrcf32x4, vslopef32x4);
                vmulf32x4 = vbslq_f32(vmasku32x4, vmulf32x4, vsrcf32x4);
                vst1q_f32(&outPtr[i], vmulf32x4);
            }
#endif
            for (; i<size; i++)
            {
                if (inPtr[i] < 0)
                    outPtr[i] = inPtr[i]*slope;
                else
                    outPtr[i] = inPtr[i];
            }
        }
    }
    return 0;
}
};
