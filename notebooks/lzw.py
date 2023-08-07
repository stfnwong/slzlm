# ---
# jupyter:
#   jupytext:
#     formats: ipynb,py:percent
#     text_representation:
#       extension: .py
#       format_name: percent
#       format_version: '1.3'
#       jupytext_version: 1.14.7
#   kernelspec:
#     display_name: Python 3 (ipykernel)
#     language: python
#     name: python3
# ---

# %% [markdown]
# # LZW Encoding
#
# This notebook shows the use of the `lzw_encode` and `lzw_decode` methods in `slz`. There is both a functional implementation and an object-oriented (stateful) implementation.
#
#
# ### Encoding
# The encoding function `lzw_decode` has the following signature
#
#
# ```python
# def lzw_encode(data: Union[str, bytes]) -> str:
# ```
#
# In reality, the `str` return is really just a byte stream returned from the encoder. It almost always makes sense to do `b = lzw_encode(data).encode("utf-8")`. The reason the return type is `str` rather than `bytes` is to do with how bytes work in Python and what is possible with `pybind11`. See for example [https://github.com/pybind/pybind11/issues/1236].
#
# The signature in the underlying implementation is
#
# ```c++
# std::stringstream lzw_encode(const std::string_view input);
# ```
#
# Because the return type is strictly `str` we need to explicitly convert to `bytes` if a byte stream is required by doing ```enc_out.encode("utf-8")```.

# %% [markdown]
# ## Encoding format.
#
# The output of the encoder is a byte stream containing the compressed bytes. To be able to recover the stream information correctly when decoding some header information is prepended to the data. The data stream consists of
# symbols that vary in size from 2-4 bytes per symbol. At the start of the stream all symbols are 2 bytes. As the prefix tree gets longer it becomes possible to gain more compression by using a larger symbol that represents a longer prefix. At some point the stream will contain symbols that are all 3 bytes long, and then even later all symbols will be 4 bytes long. When decoding the stream we need to know at which point the size of the symbols changes. We encode this information in a 12 byte header at the start of the stream
#
# The header format is
#
# - (__4 bytes__) - offset in stream where the first 24-bit code is. If this is zero there are no 24-bit codes
# - (__4 bytes__) - offset in stream where the first 32-bit code is. If this is zero there are no 32-bit codes.
# - (__4 bytes__) - total number of codes in the stream.
#
# The byte at offset 12 is the first byte in the data stream and is part of a 2-byte symbol.

# %%
# %load_ext autoreload
# %autoreload 2

# %%
# TODO: hack - need to pip install package to avoid this?
import os, sys

if ".." not in sys.path:
    sys.path.insert(0, "..")

# %%
import numpy as np
from slz import lzw_encode      # import the functional encoder


# We start with the example from the unit test
test_input = "babaabaaa"

# We convert the input to a byte array
func_inp = np.frombuffer(test_input.encode("utf-8"), dtype="uint8")

encoded = lzw_encode(func_inp)

print(f"Length of input string  : {len(test_input.encode('utf-8'))}")
print(f"Length of output string : {len(encoded)}")
print(f"bytes: {encoded}")

# %% [markdown]
# Note that the output is longer than the input. This is often the case in general with compression - significant compression is achieved in the limit but not always for small inputs. In this specific case though some of the overhead is in the header itself. The first 12 bytes provide information for decoding. In this example the header is longer than the original string, and actually contains relatively little information. If we skip the first 12 bytes

# %%
print(f"bytes: {encoded[11:]}")

# %% [markdown]
# We can decode a stream with this header using `lzw_decode`. This accepts an `np.ndarray` of bytes that is expected to have the header format described above, followed by a byte stream of compressed bytes. The signature for `lzw_decode` is
#
# ```python
# def lzw_decode(data: np.ndarray) -> str:
# ```
#
# The underlying representation accepts a pointer to a `uint8_t` array, but is wrapped to accept a `pybind11::array_t<uint8_t>`. The wrapper function accepts an `np.ndarray` of bytes. This means that if we wish to read (for example) text files from disk, we need to perform a conversion 
#
# ```c++
# pybind11::array_t<uint8_t> lzw_decode(const pybind11::array_t<uin8_t>& data)
# ```

# %%
from slz import lzw_decode

decoded = lzw_decode(encoded)

print(f"Length of input string  : {len(encoded)}")
print(f"Length of output string : {len(decoded)}")
print(f"str: {decoded}")

dec_string = "".join([chr(c) for c in decoded])
print(f"{dec_string}"")

# %% [markdown]
# ### Encoding a stream from a file
#
# Notice that the compression ration on small inputs is very poor. At minimum the size of the stream needs to be logn enoguh to amortize the length of the header. Additionally, we tend to get better compression performance when we have a deep prefix tree. Constructing a deep prefix tree requires us to consume more symbols. In the following cells we explore compressing longer streams.
#
# To avoid getting psy-op'd on copyright issues we use the collected works of Shakespear from Project Gutenberg.

# %%
input_filename = "../test/shakespear.txt"

with open(input_filename, "rb") as fp:
    text = fp.read()

sample_len = 1024
    
substr = text[:sample_len]
print(type(substr))
print(substr)
print(len(substr))

# %% [markdown]
# We encode the string using `lzw_encode`.

# %%
substr_inp = np.frombuffer(substr, dtype="uint8")
enc = lzw_encode(substr_inp)
print(len(enc))
print(enc)
