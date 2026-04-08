-- BoundaryCount.lean

import «Types»

-- Boundary: at least one coordinate is 0 or 3
def isBoundary (c : Coord) : Bool :=
  c.x = 0 ∨ c.x = 3 ∨
  c.y = 0 ∨ c.y = 3 ∨
  c.z = 0 ∨ c.z = 3 ∨
  c.w = 0 ∨ c.w = 3

-- Generate all 256 cells
def allCoords : List Coord :=
  List.range 4 >>= fun x =>
  List.range 4 >>= fun y =>
  List.range 4 >>= fun z =>
  List.range 4 >>= fun w =>
  [{x := x, y := y, z := z, w := w}]

def boundaryCells :=
  allCoords.filter isBoundary

#eval boundaryCells.length  -- should be 240
