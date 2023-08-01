// Can I put the python module in a new file?

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

// Stringstream test 
#include <sstream>


#include "Trie.hpp"
#include "LZW.hpp"


namespace py = pybind11;


// Wrappers for encode/decode standalone functions
// Note that we wrap these because the std::string input can accept bytes
// from the python shell. This means we can take either a byte string or a utf-8 string
// (ie: a printable string), and then optionally create a stringstream from that.
//
// https://pybind11.readthedocs.io/en/stable/advanced/cast/strings.html?highlight=string#return-c-strings-without-conversion
//
// Returning rdbuf() doesn't work here either. It also turns out that returning a string
// without copying is not generally possible, see https://github.com/pybind/pybind11/issues/1236
// for details

std::string py_lzw_encode(const std::string& input)
{
    return py::bytes(lzw_encode(input).str());
}

std::string py_lzw_decode(const std::string& input)
{
    std::stringstream ss(input);
    return py::bytes(lzw_decode(ss).str()); 
}


// DEFLATE function wrappers 


PYBIND11_MODULE(slz, m)
{
    // LZW object 
    py::class_ <LZWDict>(m, "LZWDict")
        .def(py::init<>())
        .def("encode", &LZWDict::encode)
        .def("get_code", &LZWDict::get_code)
        .def("contains", &LZWDict::contains);

    // DEFLATE functions

    // Function encode 
    m.def("lzw_encode", &py_lzw_encode);
    m.def("lzw_decode", &py_lzw_decode);
}
