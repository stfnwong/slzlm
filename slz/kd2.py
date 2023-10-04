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
    __slots__ = ("split", "left", "right")

    def __init__(self, split, left: Optional[Self], right: Optional[Self]):
        self.split = split
        self.left = left
        self.right = right



# Tree structure
class KDTree:

    def __init__(self, points: List[KDPoint]):
        #from pudb import set_trace; set_trace()
        self.root = self._build(0, points)
        # save bounds here, if it comes to that

    def __len__(self) -> int:
        return self._num_children()

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
        # TODO: Not toally sure what this segment is doing... finding the "correct" midpoint?
        m = len(points) // 2
        d = points[m]

        while (m+1) < len(points) and points[m+1].coords[split_dim] == d.coords[split_dim]:
            m += 1

        d = points[m]
        s = (split_dim+1) % len(d)

        return KDNode(
            split_dim,
            self._build(s, points[:m]),
            self._build(s, points[m+1:])
        )


if __name__ == "__main__":

    #points = [KDPoint(x, y) for x, y in [(2, 3), (5, 4), (9, 6), (4, 7), (8, 1), (7, 2)]]
    points = [KDPoint(p) for p in [(2, 3), (5, 4), (9, 6), (4, 7), (8, 1), (7, 2)]]

    tree = KDTree(points)

    print(f"len(points): {len(points)}, len(tree): {len(tree)}")
