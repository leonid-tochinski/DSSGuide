// Parse Disney Guilde interface
// by L. Tochinski 2021

#ifndef PARSE_GUIDE_H_
#define PARSE_GUIDE_H_
#include <string>
#include <vector>
#include <unordered_map>


struct guide_item_type
{
	std::string title;
	std::string type;
	std::string img_url;
};

typedef std::unordered_map<std::string, std::vector<guide_item_type> > guide_data_type;

bool get_guide_data(guide_data_type& guide_data);



#endif PARSE_GUIDE_H_

