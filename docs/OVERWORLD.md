# Overworld core

The overworld model is intentionally platform-neutral. Maps expose dimensions plus compact cells containing a metatile ID, collision class and elevation. Player movement asks the map whether a destination is enterable; rendering and controller polling remain outside this module.

The first implementation provides deterministic grid movement/collision primitives that can be host-tested before PS1 integration. LeafGreen behaviour such as ledges, doors, warps, object events, elevation interactions, running, biking, surfing and scripted movement will extend this model without putting PS1 hardware calls into game logic.

This is preparatory work for LGPSX-012 through LGPSX-018; it is not marked complete until converted real M0 maps render and behave correctly.
