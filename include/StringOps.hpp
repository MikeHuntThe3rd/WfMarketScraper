#pragma once
#include "VARS.hpp"
#include "json.hpp"
#include <iostream>
#include <string>
#include <vector>

using namespace std;
using namespace VARS;
using json = nlohmann::json;
namespace StringOps {

string Implode(vector<string> array, char seperator);
string GetSlugFromId(string id);
string GetIdFromSlug(string slug);
itemType GetITypeFromSlug(string slug);
void LowerCase(std::string &word);

} // namespace StringOps
