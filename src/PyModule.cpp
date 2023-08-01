// Can I put the python module in a new file?

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

// Stringstream test 
#include <sstream>
#include <string>


#include "Trie.hpp"
#include "LZW.hpp"


namespace py = pybind11;


// Wrappers for encode/decode standalone functions
std::string_view py_lzw_encode(py::bytes data)
{
    py::buffer_info info(py::buffer(data).request());
    const char* buf = reinterpret_cast<const char*>(info.ptr);
    size_t length = static_cast<size_t>(info.size);
    std::string_view s(buf, length);
    return py::bytes(lzw_encode(s).str());
}


std::string_view py_lzw_decode(py::bytes data)
{
    py::buffer_info info(py::buffer(data).request());
    const char* buf = reinterpret_cast<const char*>(info.ptr);
    size_t length = static_cast<size_t>(info.size);
    std::string_view s(buf, length);
    return py::bytes(lzw_decode_sv(s).str());
}


// TODO: .doc() for each of these, syntax is 
// .doc() = some_const_string

PYBIND11_MODULE(slz, m)
{
    // Object oriented encoder
    py::class_ <LZWEncoder>(m, "LZWEncoder")
        .def(py::init<>())
        .def("init", &LZWEncoder::init)
        .def("contains", &LZWEncoder::contains)
        .def("encode", &LZWEncoder::encode)
        .def("get", [](LZWEncoder &self) -> py::bytes {
                return py::bytes(self.get());
        })          // via https://github.com/pybind/pybind11/issues/1811
        .def("to_file", &LZWEncoder::to_file)
        .def("size", &LZWEncoder::size);

    // Object oriented decoder
    py::class_ <LZWDecoder>(m, "LZWDecoder")
        .def(py::init<>())
        .def("init", &LZWDecoder::init)
        // TODO: is there a way to zero-copy init a stringstream from string_view or 
        // py::bytes/py::buffer?
        //.def("decode", [](LZWDecoder& self, py::buffer data) {
        //        py::buffer_info info = data.request();
        //        std::stringstream ss(info.ptr);
        //        self.decode(ss);
        //})
        .def("decode", [](LZWDecoder &self, const std::string& data) {
                std::stringstream ss(data);
                self.decode(ss);        // TODO: wtf is wrong with this?
        })
        .def("get", [](LZWDecoder &self) -> py::bytes {
                return py::bytes(self.get());
        })          // via https://github.com/pybind/pybind11/issues/1811
        .def("size", &LZWDecoder::size);

    // Prefix tree
    py::class_ <Trie>(m, "Trie")
        .def(py::init<>())
        .def("insert", &Trie::insert)
        .def("search", &Trie::search);

    // Function encode 
    m.def("lzw_encode", &py_lzw_encode);
    // Function decode 
    m.def("lzw_decode", &py_lzw_decode);
}
