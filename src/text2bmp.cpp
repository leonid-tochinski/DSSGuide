// Text to bitmap converter
// By L.Tochinski 2021

// TODO make font and bitmap configurable

#include <iostream>
#include <memory>
#include <string.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include "text2bmp.h"

using namespace std;

#define FONT_FILE_NAME "NewWaltDisneyFontRegular-BPen.ttf"

// use 25pt at 100dpi 
#define FONT_CHAR_WIDTH 25

class text2bmp
{
public:
    text2bmp(int width, int height);
    ~text2bmp();
    bool convert(const char* text);
    const char* get_bytes() const { return data.get(); };
    int  get_width() const { return width_; };
    int  get_height() const { return height_; };
    int  get_size()  const { return width_ * height_; };
private:
    void draw_bitmap(FT_Bitmap* bitmap, FT_Int  x, FT_Int  y);
private:
    std::unique_ptr<char[]> data;
    int width_;
    int height_;
    FT_Library  library;
    FT_Face     face;
};

text2bmp::text2bmp(int width, int height) : 
            width_(width),
            height_(height),
            library(0),
            face(0)
{
    if (FT_Init_FreeType(&library))
    {
        cerr << "FreeType error: cannot initialize Library" << endl;
        throw 1;
    }

    if (FT_New_Face(library, FONT_FILE_NAME, 0, &face))
    {
        cerr << "FreeType error: cannot create face object for " << FONT_FILE_NAME << " font" << endl;
        throw 2;
    }

    if (FT_Set_Char_Size(face, FONT_CHAR_WIDTH * 64, 0, 100, 0))
    {
        cerr << "FreeType error: set character size " << endl;
        throw 2;
    }
    data.reset(new char[width * height]);
    memset(data.get(), 0, width_ * height_);
}

text2bmp::~text2bmp()
{
    FT_Done_Face(face);
    FT_Done_FreeType(library);
}

bool text2bmp::convert(const char* text)
{
    memset(data.get(), 0, width_ * height_);
    FT_GlyphSlot  slot = face->glyph;
    int pen_x = 10, pen_y = FONT_CHAR_WIDTH + 2;
    int num_chars = strlen(text);
    for (int n = 0; n < num_chars; n++)
    {
        // load glyph image into the slot (erase previous one)
        FT_Error  error = FT_Load_Char(face, text[n], FT_LOAD_RENDER);
        if (error)
        {
            cerr << "FreeType error: FT_Load_Char failed " << endl;
            continue;  
        }
        // now, draw to our target surface 
        draw_bitmap(&slot->bitmap, pen_x + slot->bitmap_left, pen_y - slot->bitmap_top);
        // increment pen position 
        pen_x += slot->advance.x >> 6;
    }
    return true;
}

void text2bmp::draw_bitmap(FT_Bitmap* bitmap, FT_Int  x, FT_Int  y)
{
    //    FT_Int  i, j, p, q;
    FT_Int  x_max = x + bitmap->width;
    FT_Int  y_max = y + bitmap->rows;
    char *image = data.get();
    for (FT_Int i = x, p = 0; i < x_max; i++, p++)
    {
        for (FT_Int j = y, q = 0; j < y_max; j++, q++)
        {
            if (i < 0 || j < 0 ||
                i >= width_ || j >= height_)
                continue;
            image[j * width_ + i] |= bitmap->buffer[q * bitmap->width + p];
        }
    }
}

/// @brief        Convert text to grayscale bitmap
///
/// Create instance of text2bmp object when run first time
/// convert string to grayscale 1 bpp butmap
/// 
/// @param text   text string
/// @param width  [out] bitmap width
/// @param height [out] bitmap height
/// @param size   [out] bitmap size
/// @return 
const char* convert_text2bmp(const char* text, int& width, int& height, int& size)
{
    static unique_ptr<text2bmp> text_bmp;
    // signleton lazy initialization
    if (!text_bmp)
    {
        try
        {
            text_bmp.reset(new text2bmp(TEXT_BMP_WIDTH, TEXT_BMP_HEIGHT));
        }
        catch (int error)
        {
            // TODO : it will try to attempt to create it each time
            cerr << "text2bmp initialization failed ( " << error << ")" << endl;
            return 0;
        }
    }

    if (text_bmp->convert(text))
    {
        width = text_bmp->get_width();
        height = text_bmp->get_height();
        size = text_bmp->get_size();
        return text_bmp->get_bytes();
    }
    return 0;
}


//#define TEXT2BMP_UNIT_TEST

#ifdef TEXT2BMP_UNIT_TEST

// check uncompressed image here: https://rawpixels.net/

#include <fstream>

int main()
{
    int width, height, size;
    const char* buf = convert_text2bmp("Test string yg", width, height, size);
    {
        const char* raw_bmp_file = "C:\\Users\\leoni\\Documents\\work\\FreeType\\Debug\\text.raw";
        auto myoutfile = std::fstream(raw_bmp_file, std::ios::out | std::ios::binary);
        myoutfile.write(buf, size);
    }
    buf = convert_text2bmp("string Test yg", width, height, size);
    {
        const char* raw_bmp_file = "C:\\Users\\leoni\\Documents\\work\\FreeType\\Debug\\text1.raw";
        auto myoutfile = std::fstream(raw_bmp_file, std::ios::out | std::ios::binary);
        myoutfile.write(buf, size);
    }

    return 0;
}
#endif // TEXT2BMP_UNIT_TEST
