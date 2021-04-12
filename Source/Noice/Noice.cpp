#include <noice/noice.h>
#include "BlueNoiseGenerator.h"
#include "Image.h"
#include <ImfImage.h>
#include <ImfRgbaFile.h>
#include <ImfOutputFile.h>
#include "KernelRunner.h"
#include "SimdSearcher.h"


namespace noice
{

void makeBlueNoise(int w, int h, int d, int threadCount)
{
    auto t0 = std::chrono::high_resolution_clock::now();
    BlueNoiseGenDesc desc;
    desc.width = w;
    desc.height = h;
    desc.depth = d;
    noice::Image outputImage;
    blueNoiseGenerator(desc, threadCount, outputImage);

    auto t1 = std::chrono::high_resolution_clock::now();
    auto ms_result = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0);
    std::cout << "Duration: " << ms_result.count() << " ms " << std::endl;

    //////////////////////

    #if 0
    ispc::Image img;
    img.width = w;
    img.height = h;
    img.data = pixels.data();


    Imf::RgbaOutputFile file ("testImage.exr", w, h, Imf::WRITE_R);
    file.setFrameBuffer (outputPixels.data(), 1, w);
    file.writePixels(h);
    #else

    int scanLines = h * d;
    Imf::Header header(w, scanLines);
    header.channels().insert("R", Imf::Channel(Imf::FLOAT));    
    try {
        Imf::OutputFile file("multiLayer2.exr", header);
        Imf::FrameBuffer frameBuffer;

        frameBuffer.insert("R", Imf::Slice(Imf::FLOAT, (char*)(outputImage.img().data),
            sizeof(float), sizeof(float) * w));

        file.setFrameBuffer(frameBuffer);
        file.writePixels(scanLines);
    } catch (const std::exception& exc) {
        std::cout << exc.what() << std::endl;
    }
    
    #endif
}

}
