# Test that we can use the python interface to LZW

from slz import LZWDict


def test_lzw():
    test_str = "babaabaaa"
    lzw = LZWDict()

    exp_res = [98, 97, 256, 257, 97]
    res = lzw.encode(test_str)

    assert exp_res == res

