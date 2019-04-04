#pragma once

extern "C" {
	#include <libavformat/avformat.h>
}

#include <memory>
#include <vector>
#include <cuda_runtime.h>
#include <mutex>

/** @addtogroup cppAPI
@{
*/

/** Class with supported frame output color formats
 @details Used in @ref TensorStream::getFrame() function
*/
enum FourCC {
	Y800, /**< Monochrome format, 8 bit for pixel */
	RGB24, /**< RGB format, 24 bit for pixel, color plane order: R, G, B */
	BGR24 /**< RGB format, 24 bit for pixel, color plane order: B, G, R */
};

/**
@}
*/

/*
Structure contains description of desired conversions.
*/
struct VPPParameters {
	unsigned int width;
	unsigned int height;
	FourCC dstFourCC;
};

int NV12ToRGB24(AVFrame* src, AVFrame* dst, int maxThreadsPerBlock, cudaStream_t* stream);
int NV12ToBGR24(AVFrame* src, AVFrame* dst, int maxThreadsPerBlock, cudaStream_t* stream);
int resizeNV12Nearest(AVFrame* src, AVFrame* dst, int maxThreadsPerBlock, cudaStream_t * stream);
int resizeNV12Bilinear(AVFrame* src, AVFrame* dst, int maxThreadsPerBlock, cudaStream_t * stream);

class VideoProcessor {
public:
	int Init(bool _enableDumps = false);
	/*
	Check if VPP conversion for input package is needed and perform conversion.
	Notice: VPP doesn't allocate memory for output frame, so correctly allocated Tensor with correct FourCC and resolution
	should be passed via Python API	and this allocated CUDA memory will be filled.
	*/
	int Convert(AVFrame* input, AVFrame* output, VPPParameters& format, std::string consumerName);
	int DumpFrame(AVFrame* output, std::shared_ptr<FILE> dumpFile);
	void Close();
private:
	bool enableDumps;
	cudaDeviceProp prop;
	//own stream for every consumer
	std::vector<std::pair<std::string, cudaStream_t> > streamArr;
	std::mutex streamSync;
	//own dump file for every consumer
	std::vector<std::pair<std::string, std::shared_ptr<FILE> > > dumpArr;
	std::mutex dumpSync;
	/*
	State of component
	*/
	bool isClosed = true;
};