// Can I put the python module in a new file?

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

// Stringstream test 
#include <sstream>


#include "Trie.hpp"
#include "LZW.hpp"


namespace py = pybind11;


// Wrappers for encode/decode standalone functions
std::string py_lzw_encode(const std::string& input)
{
    return py::bytes(lzw_encode(input).str());
}

std::string py_lzw_decode(const std::string& input)
{
    std::stringstream ss(input);
    return py::bytes(lzw_decode(ss).str());
}



PYBIND11_MODULE(slz, m)
{
    // LZW object 
    py::class_ <LZWDict>(m, "LZWDict")
        .def(py::init<>())
        .def("encode", &LZWDict::encode)
        .def("get_code", &LZWDict::get_code)
        .def("contains", &LZWDict::contains);

    // Prefix tree
    py::class_ <Trie>(m, "Trie")
        .def(py::init<>())
        .def("insert", &Trie::insert)
        .def("search", &Trie::search);

    // Function encode 
    m.def("lzw_encode", &py_lzw_encode);
    m.def("lzw_decode", &py_lzw_decode);
}
