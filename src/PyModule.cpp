// Can I put the python module in a new file?

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

// Stringstream test 
#include <sstream>


#include "Trie.hpp"
#include "LZW.hpp"
//#include "Stream.hpp"


namespace py = pybind11;


//std::string bytes_test(std::stringstream& input)
py::bytes bytes_test(const std::string_view input)            // std::string works the same way in interpreter
{
    std::ostringstream out;

    //char c;
    //while(input)
    //{
    //    if(!input.get(c))
    //        break;
    //    if(input.eof() || input.fail())
    //        break;
    //    out.write(reinterpret_cast<const char*>(&c), sizeof(const char));
    //}

    for(const auto c : input)
        out.write(reinterpret_cast<const char*>(&c), sizeof(const char));

    return py::bytes(out.str());
}



py::bytes encode_wrapper(const std::string& input)
{
    LZWDict lzw;
    std::stringstream ss;
    ss << input;
    
    auto ret = lzw.encode(ss);

    return py::bytes(ret.data.str());
}



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

    m.def("encode_wrapper", &encode_wrapper);
    m.def("bytes_test", &bytes_test);
}
