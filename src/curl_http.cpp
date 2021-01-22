// curl wrapper 
// by L. Tochinski 2021
// inspired by https://niranjanmalviya.wordpress.com/2018/06/23/get-json-data-using-curl/

#include <iostream>
#include <string>
#include <assert.h>
#include <string.h>
#include "curl_http.h"

using namespace std;

static class curl_global
{
public:
	curl_global() { curl_global_init(CURL_GLOBAL_ALL); } // sets the program environment
	~curl_global() { curl_global_cleanup(); }
} curl_global_;


curl_http::curl_http(size_t initial_buf_size) :
	curl(0),
	headers(0),
	buf_size(0),
	data_size(0)
{
	curl = curl_easy_init(); // initialize curl
	if (!curl)
	{
		cerr << "ERROR : Curl initialization\n" << endl;
		throw 0;
	}
	//ignore ssl verification if you don't have the certificate or don't want secure communication
	// you don't need these two lines if you are dealing with HTTP url
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L); //maximum time allowed for operation to finish, its in seconds
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_callback); // pass our call back function to handle received data
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, this); // pass the variable to hold the received data

	buf.reset(new char[initial_buf_size]);
	buf_size = initial_buf_size;
}


curl_http::~curl_http()
{
	if (curl)
		curl_easy_cleanup(curl);
	if (headers)
		curl_slist_free_all(headers);
}

size_t curl_http::curl_callback(void* ptr, size_t size, size_t nmemb, void* usr)
{
	assert(usr);

	size_t data_chunk_size = size * nmemb;
	curl_http* inst = (curl_http*)usr;

	if (inst->data_size + data_chunk_size > inst->buf_size)
	{
		inst->buf_size = inst->data_size + data_chunk_size + 100 * 1024;
		char* new_buf = new char[inst->buf_size];
		memcpy(new_buf, inst->buf.get(), inst->data_size);
		inst->buf.reset(new_buf);
	}

	memcpy(inst->buf.get() + inst->data_size, ptr, data_chunk_size);
	inst->data_size += data_chunk_size;
	return data_chunk_size;
}

bool curl_http::exec(const char* url, const char* content_type)
{
	assert(curl);

	data_size = 0;

	if (headers)
	{
		curl_slist_free_all(headers);
		headers = 0;
	}
	headers = curl_slist_append(headers, "User-Agent: libcurl-agent/1.0");
	headers = curl_slist_append(headers, "Cache-Control: no-cache");
	string content_type_hdr = "Content-Type: ";
	content_type_hdr += content_type;
	headers = curl_slist_append(headers, content_type_hdr.c_str());
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

	curl_easy_setopt(curl, CURLOPT_URL, url); // pass the url

	CURLcode status = curl_easy_perform(curl); // execute request
	if (status != 0)
	{
		cerr << "Error: Request failed on URL : " << url << endl;
		cerr << "Error Code: " << status << " Error Detail : " << curl_easy_strerror(status) << endl;
		return false;
	}

	// zero terrminate buffer.
	buf[data_size] = 0;

	return true;
}





//#define __CURL_UNIT_TEST__

#ifdef __CURL_UNIT_TEST__
#include <fstream>

int main()
{
	curl_http curl(100000);
	const char* url = "https://cd-static.bamgrid.com/dp-117731241344/home.json";
	if (curl.exec(url, "application/json"))
	{
		cout << curl.get_data() << endl;
	}

	url = "https://prod-ripcut-delivery.disney-plus.net/v1/variant/disney/992C518AD77C922AF2C3D8C72D34FD960CF62836108CDFE5556E7B436BC876CD/scale?format=jpeg&quality=90&scalingAlgorithm=lanczos3&width=500";
	if (curl.exec(url, "image/jpg"))
	{
		auto myfile = std::fstream("img.jpg", std::ios::out | std::ios::binary);
		myfile.write(curl.get_data(), curl.get_data_size());
//		myfile.close();

	}
	return 0;
}

#endif // __CURL_UNIT_TEST__
