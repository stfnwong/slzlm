/*
 * This sample program is very inflexible
 */

#include <iostream>
#include <string>
#include <vector>

#include "Node.hpp"     // TODO: change this name


int main(int argc, char* argv[])
{
    Trie trie;

    std::vector<std::string> sample_strings = {
        "enclose", 
        "enslave",
        "encase",
        "triangle",
        "triathlon"
    };

    for(const std::string& s : sample_strings)
        trie.insert(s);

    for(const std::string s: {"enclose", "encapsulate", "entomb", "triangle", "triage"})
        std::cout << "[" << s << "] (" << (trie.search(s) ? "found" : "not found") << ")" << std::endl;

    return 0;
}
