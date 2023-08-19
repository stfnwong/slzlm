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



# Try a 2-d version first
@dataclass
class Node:
    split: Point
    cut_dim: int
    left: Optional[Self]
    right: Optional[Self]

    def dim(self) -> int:
        return self.split.dim()



def build_test(points: List[Point], idx_from: int, idx_to: int, depth: int) -> Node:
    axis = depth % points[0].dim()
    median = idx_from + ((idx_to - idx_from) // 2)
    count = idx_to - idx_from + 1

    # If there is only one point then return a leaf node
    if count == 1:
        return Node(points[0], axis, None, None)

    #pp = sorted(points[idx_from:], key=lambda p: p.coords[0])
    pp = sorted(points, key=lambda p: p.coords[0])
    print(f"median: {median}, count: {count}, len(points): {len(points)}, len(pp): {len(pp)}, idx_from: {idx_from}, idx_to: {idx_to}")
    midpoint = points[median]
    #midpoint = pp[median]
    node = Node(midpoint, axis, None, None)
    node.left = build_test(pp, idx_from, median, depth+1)
    node.right = build_test(pp, median+1, idx_to, depth+1)

    return node







class KDTree:
    def __init__(self, points: List[Point]):
        #self.nodes = [Node() for _ in range(len(points))]
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
    from random import random
    num_points = 8
    xmax = 120; ymax = 119
    points = [Point((random() * xmax, random() * ymax)) for _ in range(num_points)]

    from pudb import set_trace; set_trace()
    nn = build_test(points, 0, len(points)-1, 0)

    print(nn)
