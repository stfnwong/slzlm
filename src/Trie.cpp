/*
 * TRIE
 * Trie Node
 */

#include <pybind11/pybind11.h>

#include "Trie.hpp"


void Trie::insert(const std::string_view word)
{
    auto* node = this->root.get();
    for(auto const c: word)
    {
        auto& children = node->children;
        auto it = children.find(c);
        if(it == children.end())
            it = children.emplace(c, std::make_unique<TrieNode>()).first;
        
        node = it->second.get();
    }

    node->value = this->cur_key;
    this->cur_key++;
    node->leaf = true;
}


bool Trie::search(const std::string_view word) const
{
    auto* node = this->root.get();
    for(auto const c: word) 
    {
        auto& children = node->children;
        auto it = children.find(c);
        if(it == children.end())
            return false;            // we don't have it

        node = it->second.get();
    }

    return node->leaf;
}


uint32_t Trie::search_key(const std::string_view word) const
{
    // TODO: factor this out
    auto* node = this->root.get();
    for(auto const c: word) 
    {
        auto& children = node->children;
        auto it = children.find(c);
        if(it == children.end())
            return 0;            // we don't have it

        node = it->second.get();
    }

    return node->value;
}

// Version using arrays rather than maps


void TrieArray::insert(const std::string_view word)
{
    auto* node = this->root.get();
    for(auto const c: word)
    {
        int idx = int(c);
        if(!node->children[idx].get())
            node->children[idx] = std::make_unique<TrieArrayNode>();

        node = node->children[idx].get();
    }

    node->value = this->cur_key;
    this->cur_key++;
    node->leaf = true;
}

bool TrieArray::search(const std::string_view word) const
{
    auto* node = this->root.get();
    for(auto const c: word)
    {
        int idx = int(c);
        if(!node->children[idx].get())
            return false;

        node = node->children[idx].get();
    }

    return node->leaf;
}

uint32_t TrieArray::search_key(const std::string_view word) const
{
    // TODO: factor this out
    auto* node = this->root.get();
    for(auto const c: word) 
    {
        int idx = int(c);
        if(!node->children[idx].get())
            return 0;

        node = node->children[idx].get();
    }

    return node->value;
}




// TODO: for LZ I think I need a version that gives back the key



// Python binding test
namespace py = pybind11;


PYBIND11_MODULE(slz, m)
{
    //py::module_::create_extension_module m("slz", "Trie");
    //py::module_ m("slz", "Trie");
    py::class_ <Trie>(m, "Trie")
        .def(py::init<>())
        .def("insert", &Trie::insert)
        .def("search", &Trie::search);

    
    //return m.ptr();
}
