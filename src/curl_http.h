// by L. Tochinski 2021

#ifndef __CURL_HTTP_H_
#define __CURL_HTTP_H_

#include <curl/curl.h>
#include <memory>


class curl_http
{
public:
	curl_http(size_t initial_buf_size);
	~curl_http();
	bool exec(const char* url, const char* content_type);
	/// @brief  get data bufer size
	/// @return size
	size_t get_data_size() const { return data_size; }
	/// @brief  get data buffer
	/// @return data buffer
	const char* get_data() const { return buf.get(); }


private:
	static size_t curl_callback(void* ptr, size_t size, size_t nmemb, void* usr);

	CURL* curl;
	struct curl_slist* headers;
	std::unique_ptr<char[]> buf;
	size_t             buf_size;  // allocared buf size
	size_t             data_size;
};

#endif // __CURL_HTTP_H_