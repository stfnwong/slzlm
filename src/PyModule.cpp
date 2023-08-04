#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

// Stringstream test 
#include <sstream>
#include <string>


#include "Trie.hpp"
#include "LZW.hpp"


namespace py = pybind11;


/*
 * Python wrappers.
 *
 * The point of these wrappers is mostly to try and get the byte handling 
 * between Python and C++ right. I have no idea what I am doing for the most part and
 * am just experimenting with different things to see what happens. Ideally we don't 
 * copy any of the input bytes.
 */
std::string py_lzw_encode(const std::string_view data)
{
    //py::buffer_info info(py::buffer(data).request());
    //const char* buf = reinterpret_cast<const char*>(info.ptr);
    //size_t length = static_cast<size_t>(info.size);
    //std::string_view s(buf, length);

    //auto rv = lzw_encode(s);
    //return std::string_view(
    //        reinterpret_cast<char*>(rv.data()),
    //        rv.size()
    //);

    return py::bytes(lzw_encode(data).str());
}


std::string_view py_lzw_decode(py::bytes data)
{
    py::buffer_info info(py::buffer(data).request());
    const char* buf = reinterpret_cast<const char*>(info.ptr);
    size_t length = static_cast<size_t>(info.size);
    std::string_view s(buf, length);

    auto rv = lzw_decode_sv(s);
    return std::string_view(
            reinterpret_cast<char*>(rv.data()),
            rv.size()
    );
    //return py::bytes(lzw_decode_sv(s));
    //return py::bytes(lzw_decode_sv(s).str());
}


// Helper function to avoid return copy
// source: https://github.com/pybind/pybind11/issues/1042#issuecomment-642215028

//template <typename Seq>
//inline py::array_t<typename Seq::value_type> as_pyarray(Seq&& sequence)
//{
//    auto size = sequence.size();
//    auto data = sequence.data();
//
//    std::unique_ptr<Seq> seq_ptr = std::make_unique<Seq>(std::move(sequence));
//    auto capsule = py::capsule(seq_ptr.get(), [](void* p) {
//        std::unique_ptr<Seq>(reinterpret_cast<Seq*>(p));
//    });
//
//    seq_ptr.release();
//
//    return py::array(size, data, capsule);
//}
//



py::array_t<char> py_numpy_test(const py::array_t<char>& data)
{
    py::buffer_info info = data.request();
    char* ptr = static_cast<char*>(info.ptr);

    // make another pointer for the output data
    auto result = py::array_t<char>(info.size);
    py::buffer_info res_info = result.request();
    char* res_ptr = static_cast<char*>(res_info.ptr);

    for(size_t i = 0; i < info.size; ++i)
        res_ptr[i] = (2 * ptr[i])  % 256;


    return result;
    ///return py::array(vec.size(), vec.data());
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

    m.def("numpy_test", &py_numpy_test);
}
