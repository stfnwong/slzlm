# Test that we can use the python interface to LZW

import numpy as np
from slz import (
    lzw_encode,
    lzw_decode,
)


# The encoded output, each element is the value of a single byte in the stream,
# the first 12 bytes are the expected header
exp_int_seq = [
    0, 0, 0, 0, 0, 0, 0, 0, 5, 1, 0, 0,  # header
    98, 0, 97, 0, 0, 1, 1, 1, 97, 0, 4, 1  # stream
]


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

    enc_text = lzw_encode(inp)
    dec_text = lzw_decode(enc_text)

    assert len(dec_text) == sample_len

    assert "".join(chr(c) for c in dec_text) == "".join([chr(c) for c in text[:sample_len]])
