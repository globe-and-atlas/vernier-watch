---
generated_by: "OpenAI Codex (GPT-6; exact variant not exposed)"
timestamp: "2026-11-08T16:49:47-06:00"
---
# Publish native previews

The authorized 0.1.0 release is at https://apps.repebble.com/2b6429b8b3e04a79aab2a4b2.

Build isolated fixtures with `python3 execution/build_fixture.py`. Capture each platform serially with `python3 execution/build_store_media.py --platform emery --reset-emulator` and the corresponding gabbro command. Inspect emulator ownership before resetting. The reset backs up flash; never drive a shared emulator concurrently.

Preview frames come from deterministic native fixture poses, with identity and geometry markers checked then removed. Three identical complete LCD frames are required per pose. Pendulum shows an accurately timed optional smooth swing; Vernier uses an explicitly disclosed accelerated time-lapse. Both include six palettes. Production is preserved separately in dist/ and release/.

Use the installed Pebble tool Python environment to run `execution/publish_store.py validate`, then `publish` once. The publisher validates exact production SHA, UUID, version, target platforms, five assets per platform, dimensions, and GIF frame counts. It guards an in-flight POST; reconcile any uncertain result before another write. Authentication uses the existing SDK account without storing credentials in the repository.

Select Time 2 and Round 2 explicitly on the public page. Record each ordered five-image gallery in .tmp/store/ui-gallery.json. Run publisher `verify` to compare the public PBW hash, both GIF bytes, and all PNG pixels. Preserve receipts and proof in release/. An upload receipt alone is not public verification.

Physical-watch battery endurance remains unmeasured.
