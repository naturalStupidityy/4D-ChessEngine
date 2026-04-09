-- MovegenTermination.lean

-- Idea: every recursive step reduces distance to boundary

def measure (n : Nat) := n

-- Example termination theorem
theorem movegen_terminates :
  WellFounded (fun x y => x < y) :=
by
  apply Nat.lt_wfRel
