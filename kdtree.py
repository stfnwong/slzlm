# Sample KDTree

from dataclasses import dataclass
from typing import List, Optional, Self



@dataclass
class Point:
    coords: List[float]

    # This only handles the 2-d case for now
    #def __init__(self, x: Iterable[float]):
    #    for elem in x:
    #        self.coords.append(elem)

    def dim(self) -> int:
        return len(self.coords)     # should be 2

    def __repr__(self) -> str:
        points = ", ".join([f"{c:.3f}" for c in self.coords])
        return f"Point<{self.dim()}>({points})"



# Try a 2-d version first
@dataclass
class Node:
    split: Point
    cut_dim: int
    left: Optional[Self]
    right: Optional[Self]


    def dim(self) -> int:
        return self.split.dim()

    def __repr__(self) -> str:
        return f"Node({self.split})"

    def __str__(self) -> str:
        s = "Node "
        cur_level = 0
        q = [(self, cur_level)]

        while q:
            cur_node, node_level = q.pop(0)

            if node_level != cur_level:
                s += "\n"
                cur_level = node_level

            s += f"{cur_node.split}[{cur_node.cut_dim}] "
            if cur_node.left:
                q.append((cur_node.left, node_level+1))
            if cur_node.right:
                q.append((cur_node.right, node_level+1))

        return s




def build_test(points: List[Point], idx_from: int, idx_to: int, depth: int) -> Node:
    axis = depth % points[0].dim()
    median = idx_from + ((idx_to - idx_from) // 2)
    count = idx_to - idx_from + 1

    #pp = sorted(points[idx_from:], key=lambda p: p.coords[0])
    pp = sorted(points, key=lambda p: p.coords[0])

    print(f"median: {median}, count: {count}, len(points): {len(points)}, len(pp): {len(pp)}, idx_from: {idx_from}, idx_to: {idx_to}")

    print(f"pp: {pp}")

    midpoint = pp[median]
    node = Node(midpoint, axis, None, None)

    if count == 1:
        return node

    node.left = build_test(pp, idx_from, median, depth+1)
    node.right = build_test(pp, median+1, idx_to, depth+1)

    return node



def build_pre_sort(points: List[Point]) -> Node:

    def build(
        dim_sorted: List[List[int]],
        points: List[Point],
        idx_from: int,
        idx_to: int,
        depth: int
    ) -> Node:
        """
        dim_sorted: list of lists, where each inner list is the index of the sorted values
        for a given axis. For instance, set of 2-dimensional points will contain two lists
        (one for axis 0, one for axis 1), and each list in the index of the elements in
        sorted order.
        """

        axis = depth % points[0].dim()
        median = idx_from + ((idx_to - idx_from) // 2)
        count = idx_to - idx_from
        midpoint = points[dim_sorted[axis][median]]

        print(f"median: {median}, count: {count}, idx_from: {idx_from}, idx_to: {idx_to}")
        print(f"midpoint: {midpoint}")

        node = Node(midpoint, axis, None, None)
        if count <= 1:
            return node

        node.left = build(dim_sorted, points, idx_from, median, depth+1)
        node.right = build(dim_sorted, points, median+1, idx_to, depth+1)

        return node


    dim_sorted = [
        [i[0] for i in sorted(enumerate(points), key=lambda p: p[1].coords[dim])]
        for dim in range(points[0].dim())
    ]

    node = build(dim_sorted, points, 0, len(points)-1, 0)
    return node







class KDTree:
    def __init__(self, points: List[Point]):
        self.nodes: List[Node] = []
        self._build(0, len(points)-1, points, 0)

    def _build(self, idx_from: int, idx_to: int, points: List[Point], depth: int) -> None:

        axis = depth % points[0].dim()
        median = idx_from + ((idx_to - idx_from) // 2)

        # This is where you'd expect most of the construction time to be
        #points.sort(key=lambda p: p.coords[axis])
        pp = sorted(points[idx_from:], key=lambda p: p.coords[axis])
        midpoint = pp[median]

        node = Node(midpoint, axis, None, None)
        node.left = self._build(idx_from, median, points, depth+1)
        node.right = self._build(median+1, idx_to, points, depth+1)

        self.nodes.append(node)



if __name__ == "__main__":

    #coords = [
    #    (3, 1), (4, 2), (1, 3), (3, 3),
    #    (5, 7), (7, 8), (1, 9), (8, 2),
    #]

    #coords = [(4, 4), (2, 7), (1,6), (5, 5)]
    coords = [(1, 1), (2, 2), (3, 3), (4, 4), (5, 5), (6, 6)]
    #coords = [(7, 4), (5, 8), (3, 6), (5, 5), (2, 3), (1, 1), (6, 6), (7, 6)]
    #coords = [(10, 10), (5, 5), (1,1)]

    points = [Point((x[0], x[1])) for x in coords]
    nn = build_test(points, 0, len(points)-1, 0)
    print(f"\n{nn}\n")

    nn = build_pre_sort(points)
    print(f"\n{nn}\n")


    #from random import random
    #num_points = 4
    #xmax = 120; ymax = 119
    #points = [Point((random() * xmax, random() * ymax)) for _ in range(num_points)]

    ##from pudb import set_trace; set_trace()
    #nn = build_test(points, 0, len(points)-1, 0)

    #print(nn)
