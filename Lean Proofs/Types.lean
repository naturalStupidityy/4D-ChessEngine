-- Types.lean

-- Define a coordinate in 4D
structure Coord where
  x : Nat
  y : Nat
  z : Nat
  w : Nat
deriving Repr, DecidableEq

-- Constraint: each coordinate is in {0,1,2,3}
def validCoord (c : Coord) : Prop :=
  c.x < 4 ∧ c.y < 4 ∧ c.z < 4 ∧ c.w < 4

-- Define Board as set of valid coordinates
def Board := { c : Coord // validCoord c }

-- Flat index mapping φ
def phi (c : Coord) : Nat :=
  c.x + 4*c.y + 16*c.z + 64*c.w

-- Inverse mapping
def phi_inv (i : Nat) : Coord :=
{
  x := i % 4,
  y := (i / 4) % 4,
  z := (i / 16) % 4,
  w := (i / 64) % 4
}
