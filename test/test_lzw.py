# Test that we can use the python interface to LZW

import numpy as np
from slz import (
    lzw_encode,
    lzw_decode,
    LZWEncoder,
    LZWDecoder
)


# The encoded output, each element is the value of a single byte in the stream
exp_int_seq = [0, 0, 0, 0, 0, 0, 0, 0, 5, 1, 0, 0, 98, 0, 97, 0, 0, 1, 1, 1, 97, 0, 4, 1]


def test_lzw_encode():
    test_input = "babaabaaa"
    inp = np.frombuffer(test_input.encode("latin-1"), dtype="uint8")
    encode_out = lzw_encode(inp)

    assert len(encode_out) == len(exp_int_seq)

    for enc, i in zip(encode_out, exp_int_seq):
        assert enc == i


def test_lzw_decode():
    test_input = "babaabaaa"
    inp = np.frombuffer(test_input.encode("latin-1"), dtype="uint8")
    encode_out = lzw_encode(inp)
    decode_out = lzw_decode(encode_out)

    assert "".join(chr(c) for c in decode_out) == test_input


def test_lzw_large_text():
    test_filename = "test/shakespear.txt"

    # Turns out to be simpler to read text as binary
    with open(test_filename, "rb") as fp:
        text = fp.read()

    sample_len = 4096
    inp = np.frombuffer(text[:sample_len], dtype="uint8")
    #inp = np.frombuffer(text.encode("latin-1"), dtype="uint8")

    #from pudb import set_trace; set_trace()
    enc_text = lzw_encode(inp)
    dec_text = lzw_decode(enc_text)

    assert len(dec_text) == sample_len

    assert "".join(chr(c) for c in dec_text) == "".join([chr(c) for c in text[:sample_len]])



# Object-oriented encoder
def test_lzw_oo_encode():
    test_input = "babaabaaa"

    lzw = LZWEncoder()
    lzw.encode(test_input.encode("utf-8"))
    encode_out = lzw.get()

    assert len(encode_out) == len(exp_int_seq)

    for enc, i in zip(encode_out, exp_int_seq):
        assert enc == i


def test_lzw_oo_decode():
    lzw_dec = LZWDecoder();

    test_input = "babaabaaa"
    lzw_enc = LZWEncoder()
    lzw_enc.encode(test_input.encode("utf-8"))
    encode_out = lzw_enc.get()

    lzw_dec.decode(encode_out)

    decode_out = lzw_dec.get()
    print(len(decode_out))



def test_lzw_oo_encode_loop():
    test_filename = "test/shakespear.txt"

    # Turns out to be simpler to read text as binary
    with open(test_filename, "rb") as fp:
        text = fp.read()

    num_chunks = 8
    chunk_size = 512

    lzw = LZWEncoder()

    for chunk in range(num_chunks):
        s = text[chunk * chunk_size : (chunk+1) * chunk_size]
        lzw.encode(s)

    encode_out = lzw.get()
    # not sure what the actual encoding should be...
    assert len(encode_out) < (num_chunks * chunk_size)


def test_oo_decode_loop():
    test_filename = "test/shakespear.txt"

    # Turns out to be simpler to read text as binary
    with open(test_filename, "rb") as fp:
        text = fp.read()

    substr = text[:1024]

    lzw_enc = LZWEncoder()
    lzw_enc.encode(substr)
    encode_out = lzw_enc.get()

    # Can we decode back to the same string?
    lzw_dec = LZWDecoder()
    chunk_size = 128
    num_chunks = 8

    #from pudb import set_trace; set_trace()
    #for chunk in range(num_chunks):
    #    s = substr[chunk * chunk_size : (chunk+1) * chunk_size]
    #    lzw_dec.decode(s)          # Issue here with stringstream...

    #decode_out = lzw_dec.get()
    #assert len(decode_out) == len(substr)

