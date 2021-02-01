// in-memory jpeg decompress
// by L. Tochinski, 2021

// TODO: better cleanup on exit

#include <string.h>
#include <iostream>
#include <IL/il.h>
#include "decompress_jpeg.h"

using namespace std;

/// @brief          in-memory jpeg to bitmap decompressor
///
/// Initialize IL library if needed. Configure decompressor, decompress JPEG to RGB bitmap
/// 
/// @param in       jpeg data buffer 
/// @param in_size  jpeg buffer size
/// @param out      bitmap buffer
/// @param out_size bitmap buffer size
/// @param width    bitmap width
/// @param height   bitmap  height
/// @return         true if successful 
bool decompress_jpeg(const char* in, int in_size, char* out, int& out_size, int& width, int& height)
{

	static bool initialized = false;
	if (!initialized)
	{
		// init DevIL. This needs to be done only once per application
		ilInit();
		initialized = true;
	}
	
	static unsigned int imageID = 0xffffff;
	if (imageID == 0xffffff)
	{
		// generate an image name
		ilGenImages(1, &imageID);
		// bind it
		ilBindImage(imageID);
	}

	ILboolean success = ilLoadL(IL_JPG, in, in_size);

	ilConvertImage(IL_RGB, IL_UNSIGNED_BYTE);
	// check to see if everything went OK
	if (!success) {
		ilDeleteImages(1, &imageID);
		imageID = 0xffffff;
		return false;
	}
	ilBindImage(imageID);
	width = ilGetInteger(IL_IMAGE_WIDTH);
	height = ilGetInteger(IL_IMAGE_HEIGHT);
	int bmp_data_size = ilGetInteger(IL_IMAGE_SIZE_OF_DATA);
	if (out_size < bmp_data_size)
	{
		cerr << "bmp buffer too small" << endl;
		return false;
	}
	out_size = bmp_data_size;
	unsigned char* data = ilGetData();
	memcpy(out, data, out_size);
	return true;
}


// #define _DECOMPRESS_JPEG_UNIT_TEST_

#ifdef _DECOMPRESS_JPEG_UNIT_TEST_

#include <fstream>
#include <memory>


// check uncompressed image here: https://rawpixels.net/

#define BUF_SIZE  150000

int main()
{
	const char* jpg_file = "C:\\Users\\leoni\\Documents\\work\\Disney\\images\\Donald Duck.jpg";
	auto myfile = std::fstream(jpg_file, std::ios::in | std::ios::binary);

	unique_ptr<char[]> in(new char[BUF_SIZE]);
	myfile.read(in.get(), BUF_SIZE);
	int img_size = myfile.gcount();

	int out_size = 0, width = 0, height = 0;
	decompress_jpeg(in.get(), img_size, 0, out_size, width, height);

	unique_ptr<char[]> out(new char[out_size]);
	decompress_jpeg(in.get(), img_size, out.get(), out_size, width, height);

	const char* raw_bmp_file = "C:\\Users\\leoni\\Documents\\work\\Disney\\images\\Donald_Duck.raw";
	auto myoutfile = std::fstream(raw_bmp_file, std::ios::out | std::ios::binary);
	myoutfile.write(out.get(), out_size);
	return 0;
}

#endif // _DECOMPRESS_JPEG_UNIT_TEST_