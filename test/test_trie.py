# This is really a test of whether we can import the Trie objects and use them in python.

from slz import Trie

def test_trie():
    inputs = [
        "fuck", "your", "couch", "conch", "counter",
        "country", "cause", "company", "charlie", "murphy"
    ]

    trie = Trie()
    for s in inputs:
        trie.insert(s)

    search_terms = ["couch", "charlie", "charles", "charlton", "chad", "chud"]
    exp_results = [True, True, False, False, False, False]

    for s, exp in zip(search_terms, exp_results):
        assert exp == trie.search(s)
