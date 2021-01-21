// jansson wrapper
// by L. Tochinski 2021

#ifndef JSON_PARSER_H_
#define JSON_PARSER_H_

#include <string>
#include <vector>

struct json_t;

class json_parser {
   public:
      json_parser();
      virtual ~json_parser();

      bool read(const char* json_buf, size_t json_buf_len);
      bool set(json_t *json_obj);

      bool get(const char* key_path, std::string& val) const;
      bool get(const char* key_path, json_parser& sub_object) const;

      bool array_get(const char* key_path, std::vector<json_parser>& array) const;

      bool get_key(std::string& key) const;
      bool get_child(json_parser& child_obj) const;

      operator json_t* () const { return json_obj_; };

   private:
      json_t *json_obj_;
      static int parse_keys_from_path(const char* path, std::vector<std::string>& obj_keys);
      bool get_object(std::vector<std::string>& obj_keys, json_parser& obj) const;
      static bool is_path(const char * key_path);
};

#endif /* JSON_PARSER_H_ */
