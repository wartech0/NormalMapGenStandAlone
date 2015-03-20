#include "generation.h"
#include <math.h>
#include <string.h>

template <typename T>
bool Equal(T a, T b, T c, T d) {
	return a == b && a == c && a == d;
}

bool SameShape(ImageSource a, ImageSource b) {
	return a.rowCount == b.rowCount && a.columnCount == b.columnCount;
}

struct float3 {
	float x, y, z;
};

float3 operator + (float3 a, float3 b) {
	float3 out = a;
	out.x += b.x;
	out.y += b.y;
	out.z += b.z;
	return out;
}

enum OutputMethod {
	Overwrite,
	Add,
	AddNormalize
};

enum Direction {
	North,
	East,
	South,
	West
};

float Intensity(float3 pixel) {
	float f = pixel.y > pixel.x ? pixel.y : pixel.x;
	return pixel.z > f ? pixel.z : f;
}

float3 GetPixel(ImageSource img, uint32_t col, uint32_t row) {
	size_t const PixelSize = 4;
	uint32_t offset = col * PixelSize + row * img.rowPitchInBytes;
	char const* p = img.pixelBytes + offset;
	float3 out = { p[0] / 255.0f, p[1] / 255.0f, p[2] / 255.0f };
	return out;
}

float3 GetPixel(NormalSink img, uint32_t col, uint32_t row) {
	size_t const PixelSize = 12;
	uint32_t offset = col * PixelSize + row * img.rowPitchInBytes;
	float3 out;
	memcpy(&out, img.normalBytes + offset, PixelSize);
	return out;
}

void SetPixel(NormalSink img, uint32_t col, uint32_t row, float3 value) {
	size_t const PixelSize = 12;
	uint32_t offset = col * PixelSize + row * img.rowPitchInBytes;
	memcpy(img.normalBytes, &value, PixelSize);
}

float Dot(float3 v) {
	float out = v.x*v.x + v.y*v.y + v.z*v.z;
	return out;
}

float3 Normalize(float3 v) {
	float scalar = 1.0f / sqrtf(Dot(v));
	float3 out = { v.x * scalar, v.y * scalar, v.z * scalar };
	return out;
}

template <OutputMethod Method, Direction LightDirection>
void Process(ImageSource src, NormalSink dst) {
	for (uint32_t row = 0; row < src.rowCount; ++row) {
		for (uint32_t col = 0; col < src.columnCount; ++col) {
			float sample = Intensity(GetPixel(src, col, row));
			float unbiased = sample * 2.0f - 1.0f;
			float3 v = {};
			switch(LightDirection) {
				case North: v.y = unbiased; break;
				case East: v.x = unbiased; break;
				case South: v.y = -unbiased; break;
				case West: v.x = -unbiased; break;
			}
			switch(Method) {
				case Overwrite: {
					SetPixel(dst, col, row, v);
				} break;
				case Add: {
					SetPixel(dst, col, row, GetPixel(dst, col, row));
				} break;
				case AddNormalize: {
					float3 sum = GetPixel(dst, col, row) + v;
					SetPixel(dst, col, row, Normalize(sum));
				} break;
			}
		}
	}
}

bool GenerateNormalMap(ImageSource const sources[4], NormalSink sink) {
	if (!SameShape(sources[0], sources[1]) ||
	    !SameShape(sources[0], sources[2]) ||
	    !SameShape(sources[0], sources[3])) {
		return false;
	}
	Process<Overwrite, North>(sources[0], sink);
	Process<Add, East>(sources[1], sink);
	Process<Add, South>(sources[2], sink);
	Process<AddNormalize, West>(sources[3], sink);
	return true;
}
