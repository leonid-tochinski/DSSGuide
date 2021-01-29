// Parse Disney Guilde interface
// by L. Tochinski 2021

#ifndef PARSE_GUIDE_H_
#define PARSE_GUIDE_H_
#include <string>
#include <vector>

#define IMAGE_ID_LEN 32

enum media_item_type { DmcSeries, DmcVideo, StandardCollection, OtherType };
struct guide_item_type
{
	std::string title;
	media_item_type type; // can use 1 byte if critical
	unsigned char img_id[IMAGE_ID_LEN];
};

struct guide_collection_type
{
	std::string title;
	std::vector<guide_item_type> items;
};

typedef std::vector<guide_collection_type> guide_data_type;

bool get_guide_data(guide_data_type& guide_data);

#endif // PARSE_GUIDE_H_

