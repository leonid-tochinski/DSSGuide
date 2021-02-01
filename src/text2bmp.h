/// @file    text2bmp.h
///
/// text to bitmap converter
/// 
/// @author L. Tochinski 
/// @date 02/21/2021

#ifndef _TEXT2BMP_H_
#define _TEXT2BMP_H_

#define TEXT_BMP_WIDTH 700
#define TEXT_BMP_HEIGHT 35


const char* convert_text2bmp(const char* text, int& width, int& height, int& size);

#endif // _TEXT2BMP_H_

