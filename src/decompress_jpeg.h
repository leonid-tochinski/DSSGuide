/// @file    decompress_jpeg.h
///
/// converts jpeg to RGB bitmap in memory
/// 
/// @author L. Tochinski 
/// @date 02/21/2021

#ifndef DECOMPRESS_JPEG_H_
#define DECOMPRESS_JPEG_H_

bool decompress_jpeg(const char* in, int in_size, char* out, int& out_size, int& width, int& height);

#endif // DECOMPRESS_JPEG_H_


