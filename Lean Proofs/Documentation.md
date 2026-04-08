## Module Diagram

C++ Engine Logic
        ↓
    Types.lean
    /        \
Bijection   KnightVectors
    \        /
  BoundaryCount
        ↓
MovegenTermination
        ↓
    lake build
        ↓
 Verified Correctness




 
## Input / Output Algorithm

Input:
- C++ Engine Logic Specifications

Output:
- Verified Lean Proofs

Steps:
1. Define board and coordinate types in Lean
2. Convert C++ formulas into Lean functions
3. State theorem (e.g., bijection)
4. Apply proof tactics (simp, rw, intro)
5. Run lake build
6. If success → proofs verified
7. Else → debug errors
