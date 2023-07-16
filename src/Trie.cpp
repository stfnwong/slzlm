/*
 * TRIE
 * Trie Node
 */

#include <pybind11/pybind11.h>

#include "Trie.hpp"

// Private inner implementation of search
//std::unique_ptr<TrieNode> Trie::search_inner(const std::string_view word) const
const TrieNode* Trie::search_inner(const std::string_view word) const
{
    const auto* node = this->root.get();
    for(auto const c: word) 
    {
        auto& children = node->children;
        auto it = children.find(c);
        if(it == children.end())
            return nullptr;
            //return std::unique_ptr<TrieNode>(nullptr);            // we don't have it

        node = it->second.get();
    }

    return node;
}



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
    auto result = this->search_inner(word);
    if(result != nullptr)
        return result->leaf;

    return false;
}


uint32_t Trie::search_key(const std::string_view word) const
{
    auto result = this->search_inner(word);
    if(result != nullptr)
        return result->value;

    return false;
}



// Version using arrays rather than maps


const TrieArrayNode* TrieArray::search_inner(const std::string_view word) const
{
    auto* node = this->root.get();
    for(auto const c: word)
    {
        int idx = int(c);
        if(!node->children[idx].get())
            return nullptr;

        node = node->children[idx].get();
    }

    return node;
}


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
    auto result = this->search_inner(word);
    if(result != nullptr)
        return result->leaf;

    return false;
}

uint32_t TrieArray::search_key(const std::string_view word) const
{
    auto result = this->search_inner(word);
    if(result != nullptr)
        return result->value;

    return NULL_KEY_VAL;
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
