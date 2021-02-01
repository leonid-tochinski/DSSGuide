/// @file    guide_obj.h
/// guide object OpenGL class
/// @author L. Tochinski 
/// @date 02/21/2021

#ifndef GUIDE_OBJ_H_
#define GUIDE_OBJ_H_

#include <deque>
#include <string>
#include "parse_guide.h"

// number of items on screen
#define NUM_ROWS 4
#define NUM_COLUMNS 5

// image size
#define TEX_IMAGE_WIDTH 256
#define TEX_IMAGE_HEIGHT 144

/// @brief screen item discription 
struct item_type
{
    std::string name;      ///< title
    media_item_type type;  ///< media type
    int texture_index;     ///< texture index in textures array initialized with item's image 
};

/// @brief screen collection discription 
struct collection_type
{
    std::string name;             ///< title
    int title_texture_index;      ///< texture index in text_textures array initialized with collection's text bitmap
    std::deque<item_type> items;  ///< array of items visible on screen
};

struct GLFWwindow;

/// @brief OpenGL wrapper
/// @class guide_obj
class guide_obj
{
public:
    virtual ~guide_obj();
    virtual bool init();
    void run() const;

protected:
    guide_obj();
    void load_texture(int texture_index, const unsigned char* bmp);
    void load_text_texture(int texture_index, const char* text);
    virtual void process_new_selection(int new_selected_row, int new_selected_col);
    virtual void select(){}

    int selected_row;                              ///< currently selected row 
    int selected_col;                              ///< currently selected column
    std::deque<collection_type> collections;       ///< array of collections, describing items on the screen 

private:
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    void key_callback(int key, int scancode, int action, int mods);
    bool render() const;
    void render_text_txture(int index, float x, float y) const;

    unsigned int textures[NUM_ROWS * NUM_COLUMNS];  ///< Image textures array
    unsigned int text_textures[NUM_ROWS + 1];       ///< Text textures array. number of collections + selected tile
    GLFWwindow* window;
    static guide_obj* inst;
};

#endif // GUIDE_OBJ_H_
