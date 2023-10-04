# Sample KDTree

from dataclasses import dataclass
from typing import Iterable, List, Optional, Self, Tuple, Union



@dataclass
class Point:
    coords: Union[List[float], Tuple[float]]

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
    left: Optional[Self] = None
    right: Optional[Self] = None

    def __repr__(self) -> str:
        return f"Node({self.split})"

    def __str__(self) -> str:
        s = "Node "
        cur_level = 0
        q = [(self, cur_level)]

        while q:
            cur_node, node_level = q.pop(0)

            if node_level != cur_level:
                s += "\n".ljust(node_level*2, " ")
                cur_level = node_level

            s += f"{cur_node.split}[{cur_node.cut_dim}] "
            if cur_node.left:
                q.append((cur_node.left, node_level+1))
            if cur_node.right:
                q.append((cur_node.right, node_level+1))

        return s

    def __len__(self) -> int:
        return self.num_children()

    def dim(self) -> int:
        return self.split.dim()

    def num_children(self) -> int:
        q = [self]
        count = 0

        while q:
            cur_node = q.pop(0)
            count += 1
            #print(f"cur_node: {repr(cur_node)}, count = {count}")
            if cur_node.left:
                q.append(cur_node.left)
            if cur_node.right:
                q.append(cur_node.right)

        return count





def build_pre_sort(points: List[Point]) -> Optional[Node]:

    def build(
        dim_sorted: List[List[int]],
        points: List[Point],
        idx_from: int,
        idx_to: int,
        depth: int
    ) -> Optional[Node]:
        """
        dim_sorted: list of lists, where each inner list is the index of the sorted values
        for a given axis. For instance, set of 2-dimensional points will contain two lists
        (one for axis 0, one for axis 1), and each list in the index of the elements in
        sorted order.
        """

        #if idx_from == idx_to:
        #    print(f"Modifying idx_to [{idx_to} -> {idx_to+1}]")
        #    idx_to += 1

        axis = depth % points[0].dim()
        median_idx = idx_from + ((idx_to - idx_from) // 2)
        count = idx_to - idx_from

        print(f"depth: {depth} axis: {axis} median_idx: {median_idx}, count: {count}, idx_from: {idx_from}, idx_to: {idx_to} range: {points[idx_from : idx_to]}")

        midpoint = points[dim_sorted[axis][median_idx]]
        node = Node(midpoint, axis)
        print(f"current node at : {node.split}\n")

        if count <= 1:
            return node

        node.left = build(dim_sorted, points, idx_from, median_idx-1, depth+1)
        node.right = build(dim_sorted, points, median_idx+1, idx_to, depth+1)
        #node.right = build(dim_sorted, points, median_idx+1, idx_to, depth+1)

        return node


    dim_sorted = [
        [i[0] for i in sorted(enumerate(points), key=lambda p: p[1].coords[dim])]
        for dim in range(points[0].dim())
    ]

    print(dim_sorted)
    print(" ")

    node = build(dim_sorted, points, 0, len(points)-1, 0)
    return node



def build_tree_sort_each_time(points: List[Point]) -> Optional[Node]:

    def build(
        points: List[Point],
        idx_from: int,
        idx_to: int,
        depth: int
    ) -> Optional[Node]:
        axis = depth % points[0].dim()

        points.sort(key=lambda p: p.coords[axis])
        median_idx = idx_from + ((idx_to - idx_from) // 2)
        midpoint = points[median_idx]
        count = idx_to - idx_from

        print(f"depth: {depth} axis: {axis} median_idx: {median_idx}, count: {count}, idx_from: {idx_from}, idx_to: {idx_to}")
        print(f"points: {points}")
        print(f"points[idx_from: median_idx] : {points[idx_from: median_idx]}")
        print(f"points[median_idx: idx_to]   : {points[median_idx: idx_to]}")
        print(" ")

        node = Node(midpoint, axis, None, None)
        if count <= 1:
            return node

        # TODO: don't need median_idx+1 due to python slicing?
        node.left = build(points, idx_from, median_idx, depth+1)
        node.right = build(points, median_idx, idx_to, depth+1)

        return node

    node = build(points, 0, len(points), 0)

    return node



#class kdtree:
#    def __init__(self, points: List[Point]):
#        pass
#


# TODO: this data structure mixes two things - the nodes are in a list but they should
# be connected by pointers
class KDTree:
    def __init__(self, points: List[Point]):
        self.root = self._build(0, len(points)-1, points, 0)

    def __len__(self) -> int:
        return len(self.root)

    def _build(self, idx_from: int, idx_to: int, points: List[Point], depth: int) -> Node:

        axis = depth % points[0].dim()
        median = idx_from + ((idx_to - idx_from) // 2)

        # This is where you'd expect most of the construction time to be
        #points.sort(key=lambda p: p.coords[axis])
        pp = sorted(points[idx_from:], key=lambda p: p.coords[axis])
        midpoint = pp[median]

        # I've decided I don't know how to make this construction work...
        node = Node(
            midpoint,
            axis,
            self._build(idx_from, median, points, depth+1),
            self._build(median+1, idx_to, points, depth+1)
        )
        #node = Node(midpoint, axis, None, None)
        #node.left = self._build(idx_from, median, points, depth+1)
        #node.right = self._build(median+1, idx_to, points, depth+1)

        return node


if __name__ == "__main__":

    points = [Point(p) for p in [(2, 3), (5, 4), (9, 6), (4, 7), (8, 1), (7, 2)]]

    tree = build_tree_sort_each_time(points)

    print(f"len(points): {len(points)}, len(tree): {len(tree)}")

