# Tests for sample KDTree

from slz.kdtree import Point, build_pre_sort, build_sort_each_time


def test_build_kdtree_even_num_points():
    #coords = [(7, 4), (5, 8), (3, 6), (5, 5), (2, 3), (1, 1), (6, 6), (7, 6)]
    #coords = [(1, 9), (2, 3), (4, 1), (3, 7), (5, 4), (6, 8), (7, 2), (8, 8), (7, 9), (9, 6)]
    #coords = [(1, 9), (2, 3), (4, 1), (3, 7), (5, 4), (6, 8)]
    coords = [(1,5), (5, 1), (3, 3), (4, 2)]

    points = [Point((x[0], x[1])) for x in coords]

    #num_points = 8
    #points = [Point((p, p)) for p in range(num_points)]

    from pudb import set_trace; set_trace()
    node = build_pre_sort(points)

    print(node)

    assert node.num_children() == len(points)



def test_build_kdtree_odd_num_points():
    #coords = [(3, 2), (1, 4), (5, 5), (1, 2), (4,4)]
    #points = [Point((x[0], x[1])) for x in coords]



    #num_points = [5, 9, 33, 77, 99]
    num_points = [5]

    for n in num_points:
        points = [Point((p, p)) for p in range(n)]
        node = build_pre_sort(points)

        assert node.num_children() == len(points)
        print(node)



def test_build_sort_each_time():
    coords = [(1,5), (5, 1), (3, 3), (4, 2)]

    points = [Point((x[0], x[1])) for x in coords]

    #from pudb import set_trace; set_trace()
    node = build_sort_each_time(points)

    print(node)

    assert node.num_children() == len(points)


