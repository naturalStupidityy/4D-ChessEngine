-- Bijection.lean

import «Types»

-- Prove φ⁻¹(φ(x)) = x
theorem phi_inverse_left (c : Coord) :
  phi_inv (phi c) = c := by
  simp [phi, phi_inv]
  -- Lean simplifies modular arithmetic automatically

-- Prove φ(φ⁻¹(i)) = i
theorem phi_inverse_right (i : Nat) (h : i < 256) :
  phi (phi_inv i) = i := by
  simp [phi, phi_inv]
