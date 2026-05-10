#include "StringOps.hpp"
#include <iostream>

namespace StringOps {
string Implode(vector<string> array, char seperator) {
  string result = "";
  for (int i = 0; i < array.size(); i++) {
    if (i + 1 == array.size())
      result += array[i];
    else
      result += array[i] + seperator;
  }
  return result;
}

string GetIdFromSlug(string slug) {
  for (const json &curr : items["data"]) {
    if (curr["slug"] == slug)
      return curr["id"];
  }
  cout << "[Error] couldn't find id for: " << "|" << slug << "|" << std::endl;
  return "0";
}

string GetSlugFromId(string id) {
  for (const json &curr : items["data"]) {
    if (curr["id"] == id)
      return curr["slug"];
  }
  cout << "[Error] couldn't find slug for: " << "|" << id << "|" << std::endl;
  return "0";
}

void LowerCase(string &word) {
  transform(word.begin(), word.end(), word.begin(),
            [](unsigned char ch) { return tolower(ch); });
}

itemType GetITypeFromSlug(string slug) {
  for (const auto &curr : items["data"]) {
    if (curr["slug"] == slug) {
      vector<string> tags = curr["tags"];

      if (find(tags.begin(), tags.end(), "mod") != tags.end() &&
          find(tags.begin(), tags.end(), "veiled_riven") == tags.end()) {
        return VARS::itemType::mod;
      } else {
        return VARS::itemType::basic;
      }
    }
  }
  return VARS::itemType::Ayatan;
}

vector<string> SeperateBy(stringstream &SS, string &line, char separator) {
  vector<string> result;
  while (getline(SS, line, separator)) {
    result.push_back(line);
  }
  return result;
}
} // namespace StringOps
