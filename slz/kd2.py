# An alternative formulation of a kdtree
#
# This one based on the example from Rosetta Code


from dataclasses import dataclass
from typing import Any, List, Optional, Self, Tuple, Union
#from operator import itemgetter



@dataclass
class KDPoint:
    #coords: Union[List[float], Tuple[float]]
    coords: List[float]     # Forget about Tuple for now

    def __len__(self) -> int:
        return self.dim()

    def dim(self) -> int:
        return len(self.coords)

    # I find this formatting slightly easier to read in context
    def __repr__(self) -> str:
        points = ", ".join([f"{c:.3f}" for c in self.coords])
        return f"Point<{self.dim()}>({points})"

    def __getitem__(self, item: Any) -> float:
        return self.coords[item]



class KDNode:
    __slots__ = ("split", "point", "left", "right")

    def __init__(self, split, point: KDPoint, left: Optional[Self], right: Optional[Self]):
        self.split = split
        self.point = point
        self.left = left
        self.right = right

    def __str__(self) -> str:
        s = "Node "
        cur_level = 0
        q = [(self, cur_level)]

        while q:
            cur_node, node_level = q.pop(0)

            if node_level != cur_level:
                s += " \n".ljust(node_level*2, " ")
                cur_level = node_level

            s += f"{cur_node.split}[{cur_node.point}] "
            if cur_node.left:
                q.append((cur_node.left, node_level+1))
            if cur_node.right:
                q.append((cur_node.right, node_level+1))

        return s




# Tree structure
class KDTree:

    def __init__(self, points: List[KDPoint]):
        #from pudb import set_trace; set_trace()
        # save bounds here, if it comes to that

        self.root = self._build_by_idx(0, points, 0, len(points))
        #self.root = self._build(0, points)

    def __len__(self) -> int:
        return self._num_children()

    def __str__(self) -> str:
        return str(self.root)

    def _num_children(self) -> int:
        q = [self.root]
        count = 0

        while q:
            cur_node = q.pop(0)
            count += 1
            if cur_node.left:
                q.append(cur_node.left)
            if cur_node.right:
                q.append(cur_node.right)

        return count


    # NOTE: the Rosetta code version has bounds here... which are used in the find_nearest routine
    def _build(self, split_dim: int, points: List[KDPoint]) -> Optional[KDNode]:

        if not points:
            return None

        # Lets just do the sort each time for now and worry about pre-computation later
        #points.sort(key=itemgetter(split_dim))
        points.sort(key=lambda p: p.coords[split_dim])

        m = len(points) // 2
        d = points[m]

        # TODO: Not toally sure what this segment is doing... finding the "correct" midpoint?
        while (m+1) < len(points) and points[m+1].coords[split_dim] == d.coords[split_dim]:
            m += 1

        d = points[m]
        s = (split_dim+1) % len(d)

        return KDNode(
            split_dim,
            d,          # mid point
            self._build(s, points[:m]),
            self._build(s, points[m+1:])
        )

    def _build_by_idx(
        self,
        split_dim: int,
        points: List[KDPoint],
        idx_from: int,
        idx_to: int) -> Optional[KDNode]:

        if idx_from == idx_to:
            return None

        points.sort(key=lambda p: p.coords[split_dim])
        m = idx_from + (idx_to - idx_from) // 2
        d = points[m]
        while (m+1) < len(points) and points[m+1].coords[split_dim] == d.coords[split_dim]:
            m += 1
        d = points[m]


        # TODO: forget about adjusting the midpoint for now

        s = (split_dim+1) % len(d)

        return KDNode(
            split_dim,
            d,
            self._build_by_idx(s, points, idx_from, m),
            self._build_by_idx(s, points, m+1, idx_to)
        )




if __name__ == "__main__":

    points = [KDPoint(p) for p in [(2, 3), (5, 4), (9, 6), (4, 7), (8, 1), (7, 2)]]

    tree = KDTree(points)

    print(f"len(points): {len(points)}, len(tree): {len(tree)}")
    print(tree)
