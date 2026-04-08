-- KnightVectors.lean

import «Types»

def isKnightMove (dx dy dz dw : Int) : Bool :=
  let moves := [dx, dy, dz, dw]
  let absMoves := moves.map Int.natAbs
  (absMoves.count 2 = 1) ∧ (absMoves.count 1 = 1)

-- Generate all possible moves in range [-3,3]
def allMoves : List (Int × Int × Int × Int) :=
  List.range 7 >>= fun a =>
  List.range 7 >>= fun b =>
  List.range 7 >>= fun c =>
  List.range 7 >>= fun d =>
  [(a-3, b-3, c-3, d-3)]

def knightMoves :=
  allMoves.filter (fun (dx,dy,dz,dw) =>
    isKnightMove dx dy dz dw)

#eval knightMoves.length  -- should be 48
