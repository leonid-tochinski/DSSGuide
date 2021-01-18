// jansson wrapper
// by L. Tochinski 2021

#include "json_parser.h"
#include <iostream>

using namespace std;


json_parser::json_parser() : json_obj_(0)
{
}

json_parser::~json_parser() 
{
    if (!json_obj_)
    {
       json_decref(json_obj_);
    }
}

bool json_parser::read(const char* json_buf, size_t json_buf_len)
{
   json_error_t json_error;
   json_obj_ = json_loadb(json_buf, json_buf_len, JSON_REJECT_DUPLICATES,  &json_error);
   if (json_obj_ == 0)
   {
      cerr << "Cannot open json buffer for read" << endl;
      return false;
   }
   return true;
}

bool json_parser::set(json_t *json_obj)
{
   if(json_obj == 0 || !json_is_object(json_obj)) 
   {
      return false;
   }
   json_obj_ = json_obj;
   return true;
}

bool json_parser::get(const char* key_path, std::string& val) const
{
    string key_name;
    json_parser object;
    if (is_path(key_path))
    {
        vector<string> obj_keys;
        parse_keys_from_path(key_path, obj_keys);
        key_name = obj_keys.back();
        obj_keys.pop_back();
        if (!get_object(obj_keys, object))
        {
            return false;
        }
    }
    else
    {
        key_name = key_path;
        object.set(json_obj_);
    }


   json_t *json_obj = json_object_get(object, key_name.c_str());
   if(json_obj == 0 || !json_is_string(json_obj)) 
   {
      cout << key_path << " : - absent" << endl;
      return false;
   }
   val = json_string_value(json_obj);
//   cout << key << " - present <" << val.c_str() << ">" << endl;
   return true;
}

bool json_parser::get(const char* key_path, json_parser& object) const
{
    if (is_path(key_path)) 
    {
        vector<string> obj_keys;
        parse_keys_from_path(key_path, obj_keys);
        return get_object(obj_keys, object);
    }

   json_t *json_obj = json_object_get(json_obj_, key_path);
   if(json_obj == 0 || !json_is_object(json_obj)) 
   {
      cout << key_path << " : - absent" << endl;
      return false;
   }
//   cout << key << " : - present" << endl;
   object.set(json_obj);
   return true;
}

bool json_parser::array_get(const char* key_path, std::vector<json_parser>& array) const {

    string key_name;
    json_parser object;
    if (is_path(key_path))
    {
        vector<string> obj_keys;
        parse_keys_from_path(key_path, obj_keys);
        key_name = obj_keys.back();
        obj_keys.pop_back();
        if (!get_object(obj_keys, object))
        {
            return false;
        }
    }
    else
    {
        key_name = key_path;
        object.set(json_obj_);
    }

    json_t* json_obj = json_object_get(object, key_name.c_str());
    if (json_obj == NULL || !json_is_array(json_obj)) {
        std::cout << key_path << " - absent!" << std::endl;
        return false;
    }
    array.clear();
    size_t array_size = json_array_size(json_obj);
    for (size_t index = 0; index < array_size; ++index) {
        json_t* entry_obj = json_array_get(json_obj, index);
        json_parser entry;
        if (!entry.set(entry_obj)) {
            std::cout << "Bad array entry!" << std::endl;
        }
        else {
            array.push_back(entry);
        }
    }
    return true;
}

bool json_parser::get_key(string& key) const
{
    void* iter = json_object_iter(json_obj_);
    if (iter)
    {
        key = json_object_iter_key(iter);
        return true;
    }
    return false;
}

bool json_parser::get_child(json_parser& child_obj) const
{
    void* iter = json_object_iter(json_obj_);
    if (iter)
    {
        json_t* child = json_object_iter_value(iter);
        void* iter_child = json_object_iter(child);
        if (iter_child)
        {
            child_obj.set(child);
            return true;
        }
    }

    return false;
}

int json_parser::parse_keys_from_path(const char* key_path, vector<string>& obj_keys)
{
    string s = key_path;
    std::string delimiter = "/";

    size_t pos = 0;
    std::string token;
    while ((pos = s.find(delimiter)) != std::string::npos)
    {
        token = s.substr(0, pos);
        obj_keys.push_back(token);
        s.erase(0, pos + delimiter.length());
    }
    if (!s.empty())
    {
        obj_keys.push_back(s);
    }
    return obj_keys.size();
}


bool json_parser::get_object(vector<string>& obj_keys, json_parser& obj) const
{
   json_parser current_obj;
   current_obj.set(json_obj_);
   json_parser child_obj;
   for (auto key : obj_keys)
   {
       bool error = false;
       if (key.compare("*") == 0)
       {
           error = !current_obj.get_child(child_obj);
       }
       else
       {
           error = !current_obj.get(key.c_str(), child_obj);
       }
       if (error)
       {
           cerr << "bad object key path" << endl;
           return false;
       }
       current_obj.set(child_obj);
   }
   obj.set(current_obj);
   return true;
}

bool json_parser::is_path(const char* key_path)
{
    return string(key_path).find('/') != string::npos;
}
