// Disney Guide specific business logic
// Main module
// By L. Tochinski 2021
#include <memory>
#include <string>
#include <iostream>
#include <fstream>
#include <deque>
#include <algorithm>
#include "parse_guide.h"
#include "curl_http.h"
#include "decompress_jpeg.h"
#include "guide_obj.h"

using namespace std;

#define JPEG_BUF_SIZE 30000
#define BUF_SIZE  150000

#define STOCK_IMAGE "Disney.jpg"  // 256x144

class disney_guide : public guide_obj
{
public:
    disney_guide();
    virtual bool init();

private:
    virtual void process_new_selection(int new_selected_row, int new_selected_col);
    bool initilize_item(const guide_item_type& guide_item, int texture_index, item_type& item);
    guide_data_type guide_data;
    int first_collection_idx;
    deque<int> fist_item_idx;

};

disney_guide::disney_guide() : first_collection_idx(0)
{
    get_guide_data(guide_data);
}

bool disney_guide::init()
{
    guide_obj::init();

    curl_http curl(JPEG_BUF_SIZE);
    unique_ptr<char[]> bmp(new char[BUF_SIZE]);

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
    load_text_texture(NUM_ROWS, collections[0].items[0].name.c_str());

    cout << endl << "Done" << endl;
    return true;
}

bool disney_guide::initilize_item(const guide_item_type& guide_item, int texture_index, item_type& item)
{
    item.name = guide_item.title;
    item.type = guide_item.type;
    item.texture_index = texture_index;
    string img_url = guide_item.img_url;
    size_t pos = img_url.find("width=500");
    img_url.replace(pos, string::npos, "width=256");
    curl_http curl(JPEG_BUF_SIZE);
    unique_ptr<char[]> bmp(new char[BUF_SIZE]);
    cout << '.'; // show progress
    bool error = false;
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
            int out_size = BUF_SIZE, width = 0, height = 0;
            error = !decompress_jpeg(curl.get_data(), curl.get_data_size(), bmp.get(), out_size, width, height);
        }
        else
        {
            error = true;
            cerr << "bad jpeg signature" << endl;
        }
    }
    if (error == true)
    {
        cerr << "can't get image from " << img_url << ". Using stock image." << endl;
        auto stock_jpeg = std::fstream(STOCK_IMAGE, std::ios::in | std::ios::binary);
        unique_ptr<char[]> in(new char[BUF_SIZE]);
        stock_jpeg.read(in.get(), BUF_SIZE);
        int img_size = (int)stock_jpeg.gcount();
        int out_size = BUF_SIZE, width = 0, height = 0;
        if (decompress_jpeg(in.get(), img_size, bmp.get(), out_size, width, height))
        {
            error = false;
        }
    }
    load_texture(item.texture_index, (const unsigned char*)bmp.get());
    return true;
}

void disney_guide::process_new_selection(int new_selected_row, int new_selected_col)
{
    if (new_selected_row != selected_row)
    {
        if (new_selected_row < 0)
        {
            if (first_collection_idx > 0)
            {
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
                cout << endl;
            }
        }
        else if (new_selected_row == NUM_ROWS)
        {
            if ((unsigned int)(first_collection_idx + NUM_ROWS) < guide_data.size() - 1)
            {
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
                cout << endl;
            }
        }
        else
        {
            selected_row = new_selected_row;
        }
        const string& selection_type = collections[selected_row].items[selected_col].type;
        string name;
        if (selection_type == "DmcSeries")
        {
            name = "Series: ";
        }
        if (selection_type == "StandardCollection")
        {
            name = "Collection: ";
        }
        name += collections[selected_row].items[selected_col].name;
        load_text_texture(NUM_ROWS, name.c_str());
        return;
    }

    if (new_selected_col == selected_col)
    {
        return;
    }

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
    load_text_texture(NUM_ROWS, collections[selected_row].items[selected_col].name.c_str());
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

