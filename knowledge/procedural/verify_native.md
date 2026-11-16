---
generated_by: "OpenAI Codex (GPT-6; exact variant not exposed)"
timestamp: "2026-11-08T14:48:12-06:00"
---
# Verify Vernier

Host model tests independently check86400seconds on each target, numeric format/rollover, geometry, and physical bounds. Pendulum tests smoothphase poses and bob/dial/footer separation. Vernier tests rotation, exact6secondalignment, nearesttickerror, and labelbounds. ActualClay Save/Cancel/reload covers configuration and normalized native payloads.

Native capture matrices cover palettes/custom, toggles, midnight/noon/late/rollover, dateformats and mechanism-specificposes. They send the actualClaySave payload, verify reinstallpersistence, and reject invalidnativevalues. Allcaptures require fixtureidentity/geometrymarkers followed by threeidenticalframes120msapart plus heading presence.

After fixtures, stop the owned emulator, clean-build production, and runverify.py foridentity/targetbinaries/JS/fixture-symbol exclusion. Pendulum's check_live_motion.py observes at leastfive intermediate bob positions from the actual productionPBW and restores defaultmotion. Fixture smooth-mode screenshots alone do not prove liveanimation.
