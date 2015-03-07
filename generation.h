#pragma once
#include <stdint.h>

struct ImageSource {
	char const* pixelBytes;
	uint32_t rowPitchInBytes;
	uint32_t rowCount;
	uint32_t columnCount;
};

struct NormalSink {
	char* normalBytes;
	uint32_t rowPitchInBytes;
};

bool GenerateNormalMap(ImageSource const sources[4], NormalSink sink);
