// by L. Tochinski 2021
// Parse Dysney Guide data utility

#include <algorithm>
#include <iostream>
#include "curl_http.h"
#include "json_parser.h"
#include "parse_guide.h"

using namespace std;

#define BASE_URL "https://cd-static.bamgrid.com/dp-117731241344"
#define HOME_JSON_CURL_BUF_SIZE 1200000
#define DYNAMIC_JSON_CURL_BUF_SIZE 500000

bool get_guide_data(guide_data_type& guide_data)
{
	cout << 'o'; // debug progress
	json_parser root;
	curl_http curl(HOME_JSON_CURL_BUF_SIZE);
	const char* url = BASE_URL "/home.json";
	if (!curl.exec(url, "application/json"))
	{
		return false;
	}

	root.read(curl.get_data(), curl.get_data_size());

	vector<json_parser> containers;
	root.array_get("data/StandardCollection/containers", containers);
	curl_http dynamic_json_curl(DYNAMIC_JSON_CURL_BUF_SIZE);
	for (auto cont : containers)
	{
		cout << 'o'; // debug progress
		guide_collection_type guide_collection;
		cont.get("set/text/title/full/set/default/content", guide_collection.title);
		vector<json_parser> items;
		string refId;
		if (cont.get("set/refId", refId))
		{
			string dynamic_json_url = BASE_URL  "/sets/";
			dynamic_json_url += refId;
			dynamic_json_url += ".json";

			if (dynamic_json_curl.exec(dynamic_json_url.c_str(), "application/json"))
			{
				json_parser dynamic_root;
				if (dynamic_root.read(dynamic_json_curl.get_data(), dynamic_json_curl.get_data_size()))
				{
					dynamic_root.array_get("data/*/items", items);
				}
			}
		}
		else
		{
			cont.array_get("set/items", items);
		}
		vector<guide_item_type> guide_items;
		guide_item_type guide_item;
		for (auto item : items)
		{
			item.get("type", guide_item.type);
			item.get("text/title/full/*/default/content", guide_item.title);
			item.get("image/tile/1.78/*/default/url", guide_item.img_url);
			guide_collection.items.push_back(guide_item);
		}
		if (!guide_collection.items.empty())
		{ 
			guide_data.push_back(guide_collection);
		}
		else
		{
			cout << "Empty collection '" << guide_collection.title << "', skipping" << endl;
		}
	}
	cout << endl;
	return true;
}

//#define __PARSE_GUIDE_UNIT_TEST__

#ifdef __PARSE_GUIDE_UNIT_TEST__
#include <fstream>

int main(int argc, char * argv[])
{
	guide_data_type guide_data;
	get_guide_data(guide_data);

	curl_http jpeg_curl(20000);

	for (auto collection_item : guide_data)
	{
		cout << "------> " << collection_item.title << " <------" << endl;
		for (auto item : collection_item.items)
		{
			cout << "title : " << item.title << endl;
			cout << "ttype : " << item.type << endl;
			cout << "img_url : " << item.img_url << endl;

			string img_url = item.img_url;
			size_t pos = img_url.find("width=500");
			img_url.replace(pos, string::npos, "width=250");
			if (argc > 1 && jpeg_curl.exec(img_url.c_str(), "image/jpg"))
			{
				string title = item.title;
				remove(title.begin(), title.end(), ':');
				string jpeg_path = argv[1];
				jpeg_path += "\\";
 				jpeg_path += title;
				jpeg_path += ".jpg";
				auto myfile = std::fstream(jpeg_path.c_str(), std::ios::out | std::ios::binary);
				myfile.write(jpeg_curl.get_data(), jpeg_curl.get_data_size());
			}
		}
	}
	return 0;
}
#endif // __PARSE_GUIDE_UNIT_TEST__