// Can I put the python module in a new file?

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

// Stringstream test 
#include <sstream>


#include "Trie.hpp"
#include "LZW.hpp"
//#include "Stream.hpp"


namespace py = pybind11;


PYBIND11_MODULE(slz, m)
{
    
    // LZStream
    py::class_ <LZStream>(m, "LZStream")
        .def(py::init<>())
        .def("write", &LZStream::write);

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
    m.def("lzw_encode", &lzw_encode);
}
