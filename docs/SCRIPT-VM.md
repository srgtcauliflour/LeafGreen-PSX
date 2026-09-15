# Script VM

This is a deliberately tiny host-testable VM scaffold for the future LGPSX-019 event system. Its prototype opcodes are **not** claimed to be LeafGreen bytecode. They exist to establish execution state, bounds checking, variables, yielding/waiting and deterministic tests before the exact game command set is mapped.

Production work must map required LeafGreen commands from the exact Rev 1 reference and implement them behind portable callbacks for text, movement, flags, warps, items and other game services. Never bake PS1 GPU/controller/CD calls into script opcode handlers.
