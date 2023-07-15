// Unit tests for map based trie


#define CATCH_CONFIG_MAIN
#include "catch/catch.hpp"

#include <string>
#include <vector>

#include "Node.hpp"


TEST_CASE("test_search", "map_trie")
{
    Trie t;

    std::vector<std::string> inputs = {
        "triangle",
        "triage",
        "trifecta",
        "triathlon",
        "triumphant"
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

}
