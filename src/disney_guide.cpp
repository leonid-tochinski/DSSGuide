/// @file    disney_guide.cpp
///
/// Main module. Disney Guide specific business logic
/// 
/// @author L. Tochinski 
/// @date 02/21/2021

#include <memory>
#include <string>
#include <iostream>
#include <fstream>
#include <deque>
#include <algorithm>
#include <chrono>
#include <stdio.h>
#include <string.h>
#include "parse_guide.h"
#include "curl_http.h"
#include "decompress_jpeg.h"
#include "guide_obj.h"

using namespace std;

#define JPEG_BUF_SIZE 30000
#define BMP_BUF_SIZE  150000

#define STOCK_IMAGE "Disney.jpg"  // 256x144
#define BASE_IMAGE_URL "https://prod-ripcut-delivery.disney-plus.net/v1/variant/disney/"
#define IMAGE_URL_PARAMS "/scale?format=jpeg&quality=90&scalingAlgorithm=lanczos3&width="  

#define SELECTED_ITEM_TEXT_TEX_IDX NUM_ROWS // selected title text texture is the last element of text texture array

/// @brief main object
/// @class disney_guide
class disney_guide : public guide_obj
{
public:
    disney_guide();
    virtual bool init();

private:
    virtual void process_new_selection(int new_selected_row, int new_selected_col);
    virtual void select();
    bool initilize_item(const guide_item_type& guide_item, int texture_index, item_type& item);
    void get_item_name(const item_type item, string& name);
    guide_data_type guide_data;   ///< guide data, fetched from JSON 
    int first_collection_idx;     ///< guide_data[first_collection_idx] --> guide_obj::collections[0]
    deque<int> fist_item_idx;     ///< guide_data[first_collection_idx].item[fist_item_idx[0]] --> guide_obj::collections[0].item[0]
};


disney_guide::disney_guide() : first_collection_idx(0)
{
    // fetch guide data from json on web server
  //  get_guide_data(guide_data);
}


/// @brief Initialize collections array from collections guide data
///
/// Initialize base class, collections array from guide data, get images, initialize structures
/// 
/// @return true if successful 
bool disney_guide::init()
{
    auto start = std::chrono::steady_clock::now();
    guide_obj::init();
    // fetch guide data from json on web server
    get_guide_data(guide_data);

    cout << "Getting artwork..." << endl;

    for (int row = 0; row < NUM_ROWS; ++row)
    {
        auto& guide_collection = guide_data[row];
        collection_type collection;
        collection.name = guide_collection.title;
        int num_columns = min(NUM_COLUMNS, (int)guide_collection.items.size());
        collection.title_texture_index = row;
        load_text_texture(collection.title_texture_index, collection.name.c_str());
        for (int col = 0; col < num_columns; ++col)
        {
            guide_item_type& guide_item = guide_collection.items[col];
            item_type item;
            initilize_item(guide_item, row * NUM_COLUMNS + col, item);
            collection.items.push_back(item);
        }
        collections.push_back(collection);
        fist_item_idx.push_back(0);
    }
    string name;
    get_item_name(collections[selected_row].items[selected_col], name);
    load_text_texture(SELECTED_ITEM_TEXT_TEX_IDX, name.c_str());

    std::chrono::duration<float> diff = std::chrono::steady_clock::now() - start;
    cout << endl << "Done ( " << diff.count() << " sec )" << endl;
    return true;
}

/// @brief Initialize screen item from guide item
/// 
/// Copy title text, media type, download image, convert to bitmap, initialize texture
/// 
/// @param guide_item 
/// @param texture_index 
/// @param item 
/// @return true if successful
bool disney_guide::initilize_item(const guide_item_type& guide_item, int texture_index, item_type& item)
{
    cout << '.'; // show progress
    item.name = guide_item.title;
    item.type = guide_item.type;
    item.texture_index = texture_index;
    unique_ptr<char[]> bmp(new char[BMP_BUF_SIZE]);
    bool error = false;
    // stringify image ID
    char img_id[IMAGE_ID_LEN * 2 + 1];
    for (int i = 0; i < IMAGE_ID_LEN; ++i)
    {
        snprintf(img_id + i * 2, sizeof(img_id) - i * 2, "%02hX", (unsigned short)guide_item.img_id[i]);
    }
    if (*(unsigned long long*)guide_item.img_id == 0LL) // we can't have sixteen zeros in the beginning of the string for valid image id
    {
        error = true;
    }
    else
    {
        string img_url = BASE_IMAGE_URL;
        img_url += img_id;
        img_url += IMAGE_URL_PARAMS;
        // request image size matching texture size
        static char img_size_str[5]=""; // up to 9999
        if (img_size_str[0]==0)
        { 
            snprintf(img_size_str, sizeof(img_size_str), "%d", TEX_IMAGE_WIDTH);
        }
        img_url += img_size_str;

        curl_http curl(JPEG_BUF_SIZE);
        if (!curl.exec(img_url.c_str(), "image/jpg"))
        {
            cerr << "curl failed" << endl;
            error = true;
        }
        else
        {
            const unsigned char* jpeg_data = (unsigned char*)curl.get_data();
            // check jpeg signature FF D8 FF
            if (curl.get_data_size() > 3 && jpeg_data[0] == 0xFF && jpeg_data[1] == 0xD8 && jpeg_data[2] == 0xFF)
            {
                int out_size = BMP_BUF_SIZE, width = 0, height = 0;
                error = !decompress_jpeg(curl.get_data(), curl.get_data_size(), bmp.get(), out_size, width, height);
            }
            else
            {
                error = true;
                cerr << "bad jpeg signature" << endl;
            }
        }
    }
    if (error == true)
    {
        cerr << "bad image ID: " << img_id << ". Using stock image." << endl;
        // TODO add path to stock jpeg same as exe
        auto stock_jpeg = std::fstream(STOCK_IMAGE, std::ios::in | std::ios::binary);
        unique_ptr<char[]> in(new char[BMP_BUF_SIZE]);
        stock_jpeg.read(in.get(), BMP_BUF_SIZE);
        int img_size = (int)stock_jpeg.gcount();
        int out_size = BMP_BUF_SIZE, width = 0, height = 0;
        if (!decompress_jpeg(in.get(), img_size, bmp.get(), out_size, width, height))
        {
            cerr << "Cannot get stock image " << STOCK_IMAGE << endl;
            memset(bmp.get(), 0, BMP_BUF_SIZE);
        }
    }
    load_texture(item.texture_index, (const unsigned char*)bmp.get());
    return true;
}

/// @brief Prepend prefix to item name if needed
/// @param new_selected_row 
/// @param new_selected_col 
void disney_guide::get_item_name(const item_type item, string& name)
{
    if (item.type == DmcSeries)
    {
        name = "Series: ";
    }
    else if (item.type == StandardCollection)
    {
        name = "Collection: ";
    }
    name += item.name;
}

/// @brief handle changing selection on the screen
///
/// Handle selected item, modify item/collection when needed
/// 
/// @param new_selected_row 
/// @param new_selected_col 
void disney_guide::process_new_selection(int new_selected_row, int new_selected_col)
{
    if (new_selected_row != selected_row)
    {
        if (new_selected_row < 0)
        {
            if (first_collection_idx > 0)
            {
                auto start = std::chrono::steady_clock::now();
                // recycle collection
                auto collection = *collections.rbegin();
                // get first texture index was used by row to be deleted for recycle
                int idx = collection.items[0].texture_index;
                int first_texture_idx = idx - (idx % NUM_COLUMNS);
                collection.items.clear();
                // remove bottom row first item index
                fist_item_idx.pop_back();
                // remove bottom row
                collections.pop_back();
                // decrement first displayed collection index
                --first_collection_idx;
                // create fresh collection
                const auto& guide_collection = guide_data[first_collection_idx];
                collection.name = guide_collection.title;
                // we can have small collections
                int num_columns = min(NUM_COLUMNS, (int)guide_collection.items.size());
                for (int col = 0; col < num_columns; ++col)
                {
                    const guide_item_type& guide_item = guide_collection.items[col];
                    item_type item;
                    // textures indexes are sequenction for frem collection
                    initilize_item(guide_item, first_texture_idx + col, item);
                    collection.items.push_back(item);
                }
                // add collection text
                load_text_texture(collection.title_texture_index, collection.name.c_str());
                collections.push_front(collection);
                fist_item_idx.push_front(0);
                std::chrono::duration<float> diff = std::chrono::steady_clock::now() - start;
                cout << endl << diff.count() << " sec" << endl;
            }
        }
        else if (new_selected_row == NUM_ROWS)
        {
            if ((unsigned int)(first_collection_idx + NUM_ROWS) < guide_data.size() - 1)
            {
                auto start = std::chrono::steady_clock::now();
                // recycle collection
                auto collection = *collections.begin();
                // get first texture index was used by row to be deleted for recycle
                int idx = collection.items[0].texture_index;
                int first_texture_idx = idx - (idx % NUM_COLUMNS);
                collection.items.clear();
                // remove top row first item index
                fist_item_idx.pop_front();
                // remove top row
                collections.pop_front();
                // increment first displayed collection index
                ++first_collection_idx;
                // create fresh collection
                const auto& guide_collection = guide_data[first_collection_idx + NUM_ROWS];
                collection.name = guide_collection.title;
                // we can have small collections
                int num_columns = min(NUM_COLUMNS, (int)guide_collection.items.size());
                for (int col = 0; col < num_columns; ++col)
                {
                    const guide_item_type& guide_item = guide_collection.items[col];
                    item_type item;
                    // textures indexes are sequenction for frem collection
                    initilize_item(guide_item, first_texture_idx + col, item);
                    collection.items.push_back(item);
                }
                load_text_texture(collection.title_texture_index, collection.name.c_str());
                collections.push_back(collection);
                fist_item_idx.push_back(0);
                std::chrono::duration<float> diff = std::chrono::steady_clock::now() - start;
                cout << endl << diff.count() << " sec" << endl;
            }
        }
        else
        {
            selected_row = new_selected_row;
        }
    }
    // adjust selected column when number of items less than number of columns
    int num_columns = collections[selected_row].items.size();
    if (num_columns < NUM_COLUMNS) 
    {
        if (selected_col >= num_columns || new_selected_col == num_columns)
        {
            new_selected_col = selected_col = num_columns - 1;
        }
    }

    if (new_selected_col != selected_col)
    {
        if (new_selected_col < 0)
        {
            if (fist_item_idx[selected_row] > 0)
            {
                int& first_item = fist_item_idx[selected_row];
                auto& items = collections[selected_row].items;
                // copy last item
                item_type item = items[NUM_COLUMNS - 1];
                // remove last item
                items.pop_back();
                --first_item;
                // get guide data for last item
                const auto& guide_collection = guide_data[first_collection_idx + selected_row];
                const auto& guide_item = guide_collection.items[first_item];
                initilize_item(guide_item, item.texture_index, item);
                items.push_front(item);
            }
        }
        else if (new_selected_col == NUM_COLUMNS)
        {
            const auto& guide_items = guide_data[first_collection_idx + selected_row].items;
            if (fist_item_idx[selected_row] + NUM_COLUMNS < (int)guide_items.size())
            {
                int& first_item = fist_item_idx[selected_row];
                auto& items = collections[selected_row].items;
                // copy first item
                item_type item = items[0];
                items.pop_front(); // remove first item
                ++first_item;
                const auto& guide_item = guide_items[first_item + NUM_COLUMNS - 1];
                initilize_item(guide_item, item.texture_index, item);
                items.push_back(item);
            }
        }
        else
        {
            selected_col = new_selected_col;
        }
    }

    string name;
    get_item_name(collections[selected_row].items[selected_col], name);
    load_text_texture(SELECTED_ITEM_TEXT_TEX_IDX, name.c_str());
}

void disney_guide::select()
{
   cout << "Selected: " << collections[selected_row].items[selected_col].name << endl;
}

//--------------------------------

int main()
{
    cout << "=======----->>>>> Disney Guide <<<<<-----=======" << endl
        << "   Use [Left | Back | Right | Up | Down] keys for navigation" << endl << endl
        << "   ESC for exit" << endl << endl;

    disney_guide guide;
    if (guide.init())
    {
        guide.run();
    }
    return 0;
}

