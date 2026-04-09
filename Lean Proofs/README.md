# Formal Verification using Lean 4

This module ensures mathematical correctness of the 4D Chess Engine.

We formally verify:

1. Board Representation (4D Hypercube)
2. Flat Index Mapping (Bijective)
3. Knight Move Enumeration (48 moves)
4. Boundary Cell Count (240 cells)
5. Termination of Move Generation

## Why Formal Verification?

Unlike testing, formal proofs guarantee correctness for ALL cases.

## Tools Used

- Lean 4 Theorem Prover
- lake build system

## Pipeline

C++ Logic → Lean Definitions → Theorems → Proof → Verified

If `lake build` succeeds → system is mathematically correct
