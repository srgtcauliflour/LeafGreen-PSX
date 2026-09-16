# Save format foundation

The PS1 memory-card transport and logical LeafGreen save serialization are separate layers. A small `LGSV` versioned envelope is reserved now with stable save identity, payload size and CRC32. The actual gameplay payload schema remains deferred until the relevant game-state models are ported.

This early boundary supports later transactional/backup writes and gives M10 memory-card trading a stable save identity without making trade code depend on card-sector details. Do not interpret the current envelope as a complete PS1 memory-card file format; card directory/icon/header handling belongs to the PS1 backend.
