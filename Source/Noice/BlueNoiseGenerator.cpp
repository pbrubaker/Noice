#include "BlueNoiseGenerator.h"
#include "Image.h"
#include <Noice/DistanceKernel.ispc.h>
#include <vector>
#include <iostream>
#include <chrono>
#include "KernelRunner.h"
#include "SimdSearcher.h"

namespace noice
{

class DistanceKernel
{
public:
    inline void operator()(
        int x0, int y0,
        int x1, int y1, int z, ispc::Image& image)
    {
        ispc::DistanceKernel(
            m_pX, m_pY, m_pZ,
            x0, y0, x1, y1,
            z, m_w, m_h, m_d, m_rho2, image);
    }

    inline void init(float rho2, int w, int h, int d)
    {
        m_w = w;
        m_h = h;
        m_d = d;
        m_rho2 = rho2;
    }

    inline void args(int px, int py, int pz)
    {
        m_pX = px;
        m_pY = py;
        m_pZ = pz;
    }

private:
    int m_pX = 0;
    int m_pY = 0;
    int m_pZ = 0;
    int m_w = 0;
    int m_h = 0;
    int m_d = 0;
    float m_rho2 = 2.1f;
};


template<bool useEventCallback>
static Error blueNoiseGeneratorTemplate(
    const BlueNoiseGenDesc& desc,
    int threadCount,
    Image& output,
    EventCallback eventCallback,    
    void* eventUserData)
{
    int wh = desc.width * desc.height;
    output.init(desc.width, desc.height, desc.depth);

    SimdSearcher searcher(desc.width, desc.height, desc.depth, desc.seed, threadCount);
    KernelRunner<DistanceKernel> distanceKernel(desc.width, desc.height, desc.depth, threadCount);
    distanceKernel.kernel().init(desc.rho2, desc.width, desc.height, desc.depth);

    noice::Image distancePixels;
    distancePixels.init(desc.width, desc.height, desc.depth);
    distancePixels.clear(0.0f);

    ispc::PixelState currentPixel = {};
    
    EventArguments callbackArgs;
    callbackArgs.userData = eventUserData;
    int eventCounter = 0;
    for (int pixelIt = 0; pixelIt < output.pixelCount(); ++pixelIt)
    {
        float rank = (float)pixelIt / (float)output.pixelCount();
        searcher.setValidity(currentPixel, false);
        int offset = ispc::GetOffset(currentPixel);
        output[offset] = rank;
        
        int currX = offset % desc.width;
        int currY = (offset / desc.width) % desc.height;
        int currZ = (offset / wh);
        
        distanceKernel.kernel().args(currX, currY, currZ);
        distanceKernel.run(distancePixels.img());
        currentPixel = searcher.findMin(distancePixels.img());

        if (useEventCallback)
        {
            if (eventCounter > output.getEventFrequency())
            {
                eventCounter = 0;
                callbackArgs.pixelsProcessed = pixelIt + 1;
                eventCallback(callbackArgs);
            }
            ++eventCounter;
        }
    }

    if (useEventCallback)
    {
        callbackArgs.pixelsProcessed = (int)output.pixelCount();
        eventCallback(callbackArgs);
    }

    return Error::Ok;
}

Error blueNoiseGenerator(
    const BlueNoiseGenDesc& desc,
    int threadCount,
    Image& output)
{
    if (output.hasEventCb())
    {
        return blueNoiseGeneratorTemplate<true>(desc, threadCount, output, output.getEventCb(), output.getEventUserData());
    }
    else
    {
        return blueNoiseGeneratorTemplate<false>(desc, threadCount, output, nullptr, nullptr);
    }
}

}
