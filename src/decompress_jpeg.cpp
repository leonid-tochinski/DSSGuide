// jpgtest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <jpeglib.h>
#include "decompress_jpeg.h"


using namespace std;
bool decompress_jpeg(const char* in, int in_size, char* out, int& out_size, int& width, int& height)
{
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);
	jpeg_mem_src(&cinfo, (unsigned char*)in, in_size);

	bool error = false;
	if (JPEG_HEADER_OK != jpeg_read_header(&cinfo, TRUE))
	{
		cerr << "doesn't seem jpeg data" << endl;
		error = true;
	}
	else
	{
		jpeg_start_decompress(&cinfo);

		width = cinfo.output_width;
		height = cinfo.output_height;
		int pixel_size = cinfo.output_components;
		int bmp_size = width * height * pixel_size;
		if (out == 0)
		{
			out_size = bmp_size;
		}
		else if (out_size < bmp_size)
		{
			out_size = bmp_size;
			cerr << "raw bitmap output buffer is too small" << endl;
			error = true;
		}
		else
		{
			// The row_stride is the total number of bytes it takes to store an entire scanline (row). 
			int row_stride = width * pixel_size;
			while (cinfo.output_scanline < cinfo.output_height)
			{
				unsigned char* buffer_array[1];
				buffer_array[0] = (unsigned char*)out + (cinfo.output_scanline) * row_stride;
				if (1 != jpeg_read_scanlines(&cinfo, buffer_array, 1))
				{
					cerr << "bad jpeg data" << endl;
					error = true;
					break;
				}
			}
		jpeg_finish_decompress(&cinfo);
		}
	}
	jpeg_destroy_decompress(&cinfo);

	return !error;
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