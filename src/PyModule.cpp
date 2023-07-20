// Can I put the python module in a new file?

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

// Stringstream test 
#include <sstream>


#include "Trie.hpp"
#include "LZW.hpp"


namespace py = pybind11;


//std::string bytes_test(std::stringstream& input)
py::bytes bytes_test(const py::bytes& input)            // std::string works the same way in interpreter
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

    m.def("bytes_test", &bytes_test);

    m.def("return_bytes",
            []() {
            std::ostringstream oss;

            unsigned max_alpha_size = 256;
            for(unsigned i = 0; i < max_alpha_size; ++i)
                oss.write(reinterpret_cast<const char*>(&i), sizeof(uint8_t));

            //std::string s("\xba\xd0\xba\xd0");      // not valid UTF-8
            return py::bytes(oss.str());        // this ends up being bytes type in python shell
            }
         );
}
