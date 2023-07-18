// Can I put the python module in a new file?

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "Trie.hpp"
#include "LZW.hpp"



namespace py = pybind11;


PYBIND11_MODULE(slz, m)
{
    py::class_ <LZWDict>(m, "LZWDict")
        .def(py::init<>())
        .def("encode", &LZWDict::encode)
        .def("get_code", &LZWDict::get_code)
        .def("contains", &LZWDict::contains);

    py::class_ <Trie>(m, "Trie")
        .def(py::init<>())
        .def("insert", &Trie::insert)
        .def("search", &Trie::search);
}
