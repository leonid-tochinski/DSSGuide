// by L. Tochinski 2021
// All OpenGL specific stuff
#include <GLFW/glfw3.h>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <deque>
#include <algorithm>
#include "guide_obj.h"
#include "text2bmp.h"

using namespace std;

#define SCREEN_WIDTH 720
#define SCREEN_HEIGHT 480

#define TEX_IMAGE_WIDTH 256
#define TEX_IMAGE_HEIGHT 144

#define    TEX_TEXT_TOP_MARGIN 30.f
#define SELECTED_ITEM_TITLE_LEFT_MARGIN 500.f
#define SELECTED_ITEM_TITLE_TOP_MARGIN 20.f


#define LEFT_MARGIN 30.f
#define TOP_MARGIN  70.f
#define X_IMAGE_SPACING 20
#define Y_IMAGE_SPACING 70

#define SCALING_FACTOR 0.1f

guide_obj* guide_obj::inst = 0;

guide_obj::guide_obj() : window(0),
                         selected_row(0),
                         selected_col(0)
{
    inst = this;
    memset(textures, 0, sizeof(textures));
    memset(text_textures, 0, sizeof(text_textures));
}

guide_obj::~guide_obj()
{
    if (textures[0] != 0)
    {
        glDeleteTextures(sizeof(textures) / sizeof(GLuint), textures);
    }
    if (text_textures[0] != 0)
    {
        glDeleteTextures(sizeof(text_textures) / sizeof(GLuint), text_textures);
    }
    glfwTerminate();
    inst = 0;
}

bool guide_obj::init()
{
    if (!glfwInit())
    {
        return false;
    }

    // Create a windowed mode window and its OpenGL context
    window = glfwCreateWindow(SCREEN_WIDTH * 2, SCREEN_HEIGHT * 2, "Disney Guide", NULL, NULL);
    if (!window)
    {
        return false;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);

    glfwSetKeyCallback(window, key_callback);
    // init tetures
    glGenTextures(NUM_ROWS * NUM_COLUMNS, textures);
    for (int i = 0; i < NUM_ROWS * NUM_COLUMNS; ++i)
    {
        glBindTexture(GL_TEXTURE_2D, textures[i]); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
        // set the texture wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // initialize texture 
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TEX_IMAGE_WIDTH, TEX_IMAGE_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    }

    glGenTextures(NUM_ROWS, text_textures);
    for (int i = 0; i < NUM_ROWS + 1; ++i)
    {
        glBindTexture(GL_TEXTURE_2D, text_textures[i]);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, TEXT_BMP_WIDTH, TEXT_BMP_HEIGHT, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, 0);
    }
    glBindTexture(GL_TEXTURE_2D, 0);

    glEnable(GL_TEXTURE_2D);
    // initialize coordinates system
    glViewport(0, 0, SCREEN_WIDTH * 2, SCREEN_HEIGHT * 2); // specifies the part of the window to which OpenGL will draw (in pixels), convert from normalised to pixels
    glMatrixMode(GL_PROJECTION); // projection matrix defines the properties of the camera that views the objects in the world coordinate frame. Here you typically set the zoom factor, aspect ratio and the near and far clipping planes
    glLoadIdentity(); // replace the current matrix with the identity matrix and starts us a fresh because matrix transforms such as glOrpho and glRotate cumulate, basically puts us at (0, 0, 0)
    glOrtho(0, SCREEN_WIDTH * 2, SCREEN_HEIGHT * 2, 0, 0, 1); // essentially set coordinate system
    glMatrixMode(GL_MODELVIEW); // (default matrix mode) modelview matrix defines how your objects are transformed (meaning translation, rotation and scaling) in your world
    glLoadIdentity(); // same as above comment
    return true;
}

void guide_obj::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (inst != 0)
    {
        inst->key_callback(key, scancode, action, mods);
    }
}

void guide_obj::key_callback(int key, int scancode, int action, int mods)
{
    if (action != GLFW_PRESS /*&& action != GLFW_REPEAT*/)
    {
        return;
    }

    int new_selected_row = selected_row;
    int new_selected_col = selected_col;

    switch (key)
    {
        case GLFW_KEY_ENTER: cout << "Enter"; break;
        case GLFW_KEY_BACKSPACE:
        case GLFW_KEY_LEFT:  --new_selected_col; break;
        case GLFW_KEY_RIGHT: ++new_selected_col; break;
        case GLFW_KEY_DOWN:  ++new_selected_row; break;
        case GLFW_KEY_UP:    --new_selected_row; break;
        case GLFW_KEY_ESCAPE:   glfwSetWindowShouldClose(window, GLFW_TRUE); return;
        default:;
    }
    process_new_selection(new_selected_row, new_selected_col);
}

void guide_obj::process_new_selection(int new_selected_row, int new_selected_col)
{
    if (new_selected_col < 0)
        selected_col = 0;
    else if (new_selected_col == NUM_COLUMNS)
        selected_col = NUM_COLUMNS - 1;
    else
        selected_col = new_selected_col;

    if (new_selected_row < 0)
        selected_row = 0;
    else if (new_selected_row == NUM_ROWS)
        selected_row = NUM_ROWS - 1;
    else
        selected_row = new_selected_row;
}

void guide_obj::render_text_txture(int index, float x, float y) const
{
    static const GLfloat texture_vertices[] =
    {
        TEXT_BMP_WIDTH/2, 0,  // top right corner
        0, 0,    // top left corner
        0, TEXT_BMP_HEIGHT/2,      // bottom left corner
        TEXT_BMP_WIDTH/2, TEXT_BMP_HEIGHT/2,    // bottom right corner
    };
    static const GLfloat texVertices[] = { 1,0, 0,0, 0,1, 1, 1 };

    glPushMatrix();
    glTranslatef(x, y, 0);

    glBindTexture(GL_TEXTURE_2D, text_textures[index]);

    glEnableClientState(GL_VERTEX_ARRAY); // tell OpenGL that you're using a vertex array for fixed-function attribute
    glEnableClientState(GL_TEXTURE_COORD_ARRAY_EXT);

    glVertexPointer(2, GL_FLOAT, 0, texture_vertices); // point to the vertices to be used
    glTexCoordPointer(2, GL_FLOAT, 0, texVertices);
    glDrawArrays(GL_QUADS, 0, 4); // draw the vertixes
    glDisableClientState(GL_TEXTURE_COORD_ARRAY_EXT);
    glPopMatrix();
}

bool guide_obj::render() const
{
    static const GLfloat vertices[] =
    {
        255, 0,  // top right corner
        0, 0,    // top left corner
        0, 143,      // bottom left corner
        255, 143,    // bottom right corner
    };

    static const GLfloat texVertices[] = { 1,0, 0,0, 0,1, 1, 1 };

    int num_collections = collections.size();
    for (int row = 0; row < num_collections; ++row)
    {
        auto& collection = collections[row];
        render_text_txture(collection.title_texture_index, LEFT_MARGIN, TEX_TEXT_TOP_MARGIN + row * (TEX_IMAGE_HEIGHT + Y_IMAGE_SPACING));
        int num_items = collection.items.size();
        for (int col = 0; col < num_items; ++col)
        {
            auto& item = collection.items[col];
            glPushMatrix();
            float x_translation = LEFT_MARGIN + col * (TEX_IMAGE_WIDTH + X_IMAGE_SPACING);
            float y_translation = TOP_MARGIN + row * (TEX_IMAGE_HEIGHT + Y_IMAGE_SPACING);
            float scale = 1.0f;

            if (row == selected_row && col == selected_col)
            {
                scale += SCALING_FACTOR;
                x_translation -= SCALING_FACTOR * (TEX_IMAGE_WIDTH / 2);
                y_translation -= SCALING_FACTOR * (TEX_IMAGE_HEIGHT / 2);
            }

            glTranslatef(x_translation, y_translation, 0);
            glScalef(scale, scale, 0.0);

            glBindTexture(GL_TEXTURE_2D, textures[item.texture_index]);

            glEnableClientState(GL_VERTEX_ARRAY); // tell OpenGL that you're using a vertex array for fixed-function attribute
            glEnableClientState(GL_TEXTURE_COORD_ARRAY_EXT);

            glVertexPointer(2, GL_FLOAT, 0, vertices); // point to the vertices to be used
            glTexCoordPointer(2, GL_FLOAT, 0, texVertices);
            glDrawArrays(GL_QUADS, 0, 4); // draw the vertixes
            glDisableClientState(GL_TEXTURE_COORD_ARRAY_EXT);

            // draw selection rectangle
            if (row == selected_row && col == selected_col)
            {
                glDisable(GL_TEXTURE_2D);
                glColor3f(.5f, .5f, 0);
                static const GLfloat selection_vertices[] =
                {
                    255,0,    0, 0,   
                    0,0,      0,143,    
                    0,143,    255,143, 
                    255,143,  255, 0
                };
                glVertexPointer(2, GL_FLOAT, 0, selection_vertices); // point to the vertices to be used
                glDrawArrays(GL_LINES, 0, 8); // draw the vertixes
                glColor3f(1.f, 1.f, 1.f);
                glEnable(GL_TEXTURE_2D);
            }
            glBindTexture(GL_TEXTURE_2D, 0);

            glDisableClientState(GL_VERTEX_ARRAY); // tell OpenGL that you're finished using the vertex arrayattribute

            glPopMatrix();
        }
    }
    // selected item title
    render_text_txture(NUM_ROWS, SELECTED_ITEM_TITLE_LEFT_MARGIN, SELECTED_ITEM_TITLE_TOP_MARGIN);
    return true;
}

void guide_obj::run() const
{
    // Loop until the user closes the window
    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);
        // Render OpenGL here
        render();
        // Swap front and back buffers
        glfwSwapBuffers(window);
        // Poll for and process events
        glfwPollEvents();
    }
}

void guide_obj::load_texture(int texture_index, const unsigned char* bmp)
{
    glBindTexture(GL_TEXTURE_2D, textures[texture_index]); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TEX_IMAGE_WIDTH, TEX_IMAGE_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, bmp);
}

void guide_obj::load_text_texture(int texture_index, const char* text)
{
    int width, height, size;
    const char* bytes = convert_text2bmp(text, width, height, size);
    glBindTexture(GL_TEXTURE_2D, text_textures[texture_index]); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, TEXT_BMP_WIDTH, TEXT_BMP_HEIGHT, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, bytes);
}
