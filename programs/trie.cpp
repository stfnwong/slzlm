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


    // Do search again, this time on separate lines just for debugging 
    bool r;
    for(const std::string s: {"bullshit", "enclose", "encapsulate", "entomb", "triangle", "triage"})
        r = trie.search(s);

    std::cout << r << std::endl;        // don't optimize out r



    return 0;
}
