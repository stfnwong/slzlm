// Unit tests for map based trie


#define CATCH_CONFIG_MAIN
#include "catch/catch.hpp"

#include <string>
#include <vector>

#include <iostream>

#include "Trie.hpp"


TEST_CASE("test_search", "map_trie")
{
    Trie t;

    std::vector<std::string> inputs = {
        "triangle",  // key should be 1
        "triage",    // key = 2
        "trifecta",  // key = 3
        "triathlon", // key = 4
        "triumphant" // key = 5
    };

    for(const std::string& s: inputs)
        t.insert(s);

    std::vector<std::string> search_strings = {
        "triangle",
        "tribulation",
        "trounced",
        "triumphant",
        "nothing",
        "false"
    };

    std::vector<bool> exp_results = {
        true, false, false, true, false, false
    };

    std::vector<bool> results;
    for(const std::string& s: search_strings)
        results.push_back(t.search(s));

    REQUIRE(results.size() == exp_results.size());
    for(unsigned i = 0; i < results.size(); ++i)
        REQUIRE(results[i] == exp_results[i]);

    // Check that we get the expected keys
    for(unsigned i = 0; i < inputs.size(); ++i)
        //std::cout << "[" << inputs[i] << "]: " << t.search_key(inputs[i]) << std::endl;
        REQUIRE(t.search_key(inputs[i]) == i+1);

    std::vector<uint32_t> exp_key_results = {1, 0, 0, 5, 0, 0};
    std::vector<uint32_t> key_results;
    for(const std::string& s : search_strings)
        key_results.push_back(t.search_key(s));

    REQUIRE(key_results.size() == exp_key_results.size());
    for(unsigned i = 0; i < key_results.size(); ++i)
        REQUIRE(exp_key_results[i] == key_results[i]);
}


TEST_CASE("test_trie_encoding", "map_trie")
{
    Trie trie;
    
    const std::string test_data = "babaabaaa";
    trie.insert(test_data);

    bool res = trie.search(test_data);
    REQUIRE(res == true);
}
