// guide object OpenGL class
// by L. Tochinski 2021

#ifndef GUIDE_OBJ_H_
#define GUIDE_OBJ_H_

#include <deque>
#include <string>
#include "parse_guide.h"

#define NUM_ROWS 4
#define NUM_COLUMNS 5

// defines displayed item
struct item_type
{
    std::string name;
    media_item_type type;
    int texture_index;
};
// defines displayed collection of item
struct collection_type
{
    std::string name;
    int title_texture_index;
    std::deque<item_type> items;
};

struct GLFWwindow;

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
protected:
    unsigned int textures[NUM_ROWS * NUM_COLUMNS];
    unsigned int text_textures[NUM_ROWS + 1];
    int selected_row;
    int selected_col;
    std::deque<collection_type> collections;
private:
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    void key_callback(int key, int scancode, int action, int mods);
    bool render() const;
    void render_text_txture(int index, float x, float y) const;
    GLFWwindow* window;
    static guide_obj* inst;
};

#endif // GUIDE_OBJ_H_