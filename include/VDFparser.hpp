#pragma once
#include <unordered_map>
#include <fstream>
#include <string>
#include <sstream>
#include <optional>
#include <vector>
#include <iostream>

using namespace std;
namespace Parser {
template <template <typename...> class Container>
struct VDF_Node {
    Container<string, string> values;
    Container<string, VDF_Node<Container>> childern;
};

inline vector<string> GetElements(string line){
    enum class states {OPEN, CLOSED};
    states state = states::CLOSED;
    vector<string> result;
    string element;

    for(char const chr : line){
        if(chr == '"'){
            switch (state)
            {
            case states::OPEN :
                state = states::CLOSED;
                result.push_back(element);
                element = "";
                break;
            case states::CLOSED :
                state = states::OPEN;
                break; 
            }
            continue;
        }
        if(state == states::OPEN) element.push_back(chr);
    }
    return result;
}

template <template <typename...> class Container>
void ChildRecurse(VDF_Node<Container> &root, vector<string>::iterator &ln_iter, vector<string>::iterator const end){
    while ((ln_iter + 1) != end)
    {
        string next = *(ln_iter + 1);
        string current = *ln_iter;

        if(next.find("{") != string::npos){
            VDF_Node<Container> child;
            string key = GetElements(current)[0];
            // cout << "adding child: " << key << endl;
            ln_iter += 2;
            ChildRecurse(child, ln_iter, end);
            root.childern.insert({key, child});
        } else if(current.find("}") != string::npos){
            // cout << "exiting child" << endl;
            ln_iter++;
            return;
        }
        else{
            auto elements = GetElements(current);
            // cout << "adding value(k/v): " << elements[0] << " " << elements[1] << endl;
            root.values.insert({elements[0], elements[1]});
            ln_iter++;
        }
    }
}

template <template <typename...> class Container>
optional<VDF_Node<Container>> Parse(ifstream input){
    vector<string> lines;
    string line;
    while (getline(input, line))
    {
        lines.push_back(line);
    }
    if(lines.empty()) return nullopt;
    
    VDF_Node<Container> parent_node;
    auto begin_iter = lines.begin();

    ChildRecurse(parent_node, begin_iter, lines.end());
    return parent_node;
}
}