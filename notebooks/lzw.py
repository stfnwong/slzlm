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
import os
os.chdir("..")    # cant load slz as a module from the notebooks directory

# %%
from slz import lzw_encode      # import the functional encoder

# We start with the example from the unit test
test_input = "babaabaaa"

encoded = lzw_encode(test_input)
enc_bytes = encoded.encode("utf-8")

print(f"Length of input string  : {len(test_input.encode('utf-8'))}")
print(f"Length of output string : {len(enc_bytes)}")
print(f"bytes: {enc_bytes}")

# %% [markdown]
# Note that the output is longer than the input. This is often the case in general with compression - significant compression is achieved in the limit but often not for small inputs. In this specific case though some of the overhead is in the header itself. The first 12 bytes provide information for decoding. In this example the header is longer than the original string, and actually contains relatively little information. If we skip the first 12 bytes

# %%
print(f"bytes: {enc_bytes[11:]}")

# %% [markdown]
# We can decode a stream with this header using `lzw_decode`. This accepts a byte stream that is expected to have the header format described above, followed by a byte stream of compressed bytes. The signature for `lzw_decode` is
#
# ```python
# def lzw_decode(data: bytes) -> str:
# ```
#
# The underlying representation accepts a `std::stringstream` but is wrapped to accept a `const std::string&`. The function accepts `str` or `bytes`, but a `str` formatted for display may not preserve the header information correctly.
#
# ```c++
# std::string lzw_decode(const std::string& data)
# ```

# %%
from slz import lzw_decode

dec_out = lzw_decode(enc_bytes)

print(f"Length of input string  : {len(enc_bytes)}")
print(f"Length of output string : {len(dec_out)}")
print(f"str: {dec_out}")

# %% [markdown]
# ### Encoding a stream from a file
#
# Notice that the compression ration on small inputs is very poor. At minimum the size of the stream needs to be logn enoguh to amortize the length of the header. Additionally, we tend to get better compression performance when we have a deep prefix tree. Constructing a deep prefix tree requires us to consume more symbols. In the following cells we explore compressing longer streams.
#
# To avoid getting psy-op'd on copyright issues we use the collected works of Shakespear from Project Gutenberg.

# %%
input_filename = "test/shakespear.txt"

with open(input_filename, "r") as fp:
    text = fp.read()
    #text = "".join(fp.readlines())

substr = text[1024:2048]
print(type(substr))
print(substr)
print(len(substr))

# %% [markdown]
# We encode the string using `lzw_encode`.

# %%

#enc = lzw_encode(str(substr))
enc = lzw_encode(substr.encode("utf-8"))


print(len(enc))
