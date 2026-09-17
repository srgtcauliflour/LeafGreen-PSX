# Script VM

This is a deliberately tiny host-testable VM scaffold for the future LGPSX-019 event system. Its prototype opcodes are **not** claimed to be LeafGreen bytecode. They exist to establish execution state, bounds checking, variables, yielding/waiting and deterministic tests before the exact game command set is mapped.

Production work must map required LeafGreen commands from the exact Rev 1 reference and implement them behind portable callbacks for text, movement, flags, warps, items and other game services. Never bake PS1 GPU/controller/CD calls into script opcode handlers.

## Text service callback (`OP_TEXT`)

`lg_script_set_text_fn(vm, fn, context)` registers a portable callback of
type `LGScriptTextFn` (`bool fn(void *context, uint8_t text_id)`). `OP_TEXT`
reads a 1-byte id operand and calls it: a rejecting or missing callback is a
script error (there is nothing able to resolve the wait), and an accepting
callback moves the VM to a new `LG_SCRIPT_BLOCKED` status. Unlike
`LG_SCRIPT_WAITING` (which `lg_script_step` clears automatically on the next
call, a one-frame wait), `LG_SCRIPT_BLOCKED` never clears on its own —
`lg_script_step` is a no-op while blocked. The caller must call
`lg_script_unblock(vm)` once the game service has actually finished (e.g.
the `LGWindowState`-driven dialogue reached `LG_WINDOW_DONE`), since only
that service knows when its own multi-frame work is done.

The `text_id` byte is our own scaffolding, not a LeafGreen text/dialogue
index: no ROM evidence maps real script bytecode ids to real dialogue text
yet. This establishes the blocking/callback machinery a real opcode set
will need for text, movement, warps and similar multi-frame game services.

## Movement service callback (`OP_MOVE`)

`lg_script_set_move_fn(vm, fn, context)` registers `LGScriptMoveFn`
(`bool fn(void *context, int8_t dx, int8_t dy)`). `OP_MOVE` reads two
signed 1-byte operands and validates the same step shape
`lg_player_step()` accepts (each of dx/dy in `{-1,0,1}`, never both
nonzero) before calling it -- an invalid shape, a missing callback, or a
rejecting callback are all script errors. An accepting callback blocks the
VM exactly like `OP_TEXT` does, since a real move is a multi-frame slide
into the next tile, not a one-step VM action; the caller calls
`lg_script_unblock()` once that finishes.
