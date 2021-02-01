// Parse Disney Guilde interface
// by L. Tochinski 2021

#ifndef PARSE_GUIDE_H_
#define PARSE_GUIDE_H_
#include <string>
#include <vector>

#define IMAGE_ID_LEN 32

enum media_item_type { DmcSeries, DmcVideo, StandardCollection, OtherType };

/// @brief Item description structure
struct guide_item_type
{
	std::string title;                  ///< title
	media_item_type type;               ///< media type
	unsigned char img_id[IMAGE_ID_LEN]; ///< Image ID as binary arrray 
};

/// @brief collection of items
struct guide_collection_type
{
	std::string title;                   ///< title 
	std::vector<guide_item_type> items;  ///< array of items in collection
};

/// @brief array of collections collection type
///
/// This array keeps all the collections and items data
/// 
typedef std::vector<guide_collection_type> guide_data_type;

/// @brief Initialize collections array
///
/// Retrieve home JSON, dynamic JSON files into memory, parse data, 
/// extract titles for collections and titles, media types and image IDs for items 
/// 
/// @param guide_data collections array
/// @return true if successful
bool get_guide_data(guide_data_type& guide_data);

#endif // PARSE_GUIDE_H_

