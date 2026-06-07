# DDC Training / Integration Handoff

Date: 2026-04-04

## Status

Core training/integration work has been advanced substantially.

### Completed in this pass

- Confirmed official DDR pack download sources already configured in the repo and verified completed download markers.
- Prepared filtered JSON buckets for single/double DDR chart training.
- Switched the top-level training orchestration to use the PyTorch trainer in the current environment.
- Trained the practical 8-bucket placement layout:
  - single Easy / Medium / Hard / Challenge
  - double Easy / Medium / Hard / Challenge
- Trained an onset model from `dance-single_Hard`.
- Rebuilt the difficulty-evaluator dataset and trained both:
  - `dance-single`
  - `dance-double`
- Fixed the submodule difficulty-training script so global NaN cleanup does not silently eliminate one mode.
- Added detailed documentation of findings in `docs/TRAINING_ANALYSIS_2026-04-04.md`.
- Added corpus audit tooling/report in `scripts/audit_corpus.py` and `docs/CORPUS_AUDIT_2026-04-04.md`.
- Extended `dataset/extract_json.py` to support `.ssc` files via `simfile`.
- Validated that refreshed local extraction increases coverage from 1234 to 1254 songs and from 9241 to 9403 charts.
- Documented the exact `.ssc`-driven corpus delta in `docs/SSC_EXPANSION_ANALYSIS_2026-04-04.md`.
- Updated the FFR difficulty-data loader so it also prefers `.ssc` over `.sm` where available.
- Validated the refreshed FFR preprocessing path at roughly 1255 simfiles / 9407 charts.
- Added note-object semantic audit tooling/report in `scripts/audit_note_objects.py` and `docs/NOTE_OBJECT_SEMANTICS_2026-04-04.md`.
- Added `docs/RETRAINING_REFRESH_PLAN_2026-04-04.md` describing the exact next-phase retraining workflow.
- Added `scripts/compare_bucket_counts.py` and `docs/BUCKET_SPLIT_DELTA_2026-04-04.md` to quantify exact downstream split-file deltas after `.ssc`-inclusive preparation.
- Added `scripts/audit_repo_health.py` and `docs/REPO_HEALTH_AUDIT_2026-04-04.md` to quantify remaining normalization blockers in the repository.
- Resolved top-level conflict-marker files in `AGENTS.md`, `CLAUDE.md`, `GEMINI.md`, `GPT.md`, `LLM_INSTRUCTIONS.md`, and `setup.py`.
- Resolved conflict-marker files in `autochart.py`, `learn/beatcalc.py`, `learn/data_gen.py`, `learn/models_v2.py`, `scripts/train_v2.py`, `scripts/smd_1_extract.sh`, and `scripts/smd_4_analyze.sh`.
- Refreshed the repo-health audit and reduced unresolved merge-conflict-marker files from 15 to 2.
- Added `docs/LEGACY_SUBTREE_QUARANTINE_2026-04-04.md` documenting the decision to treat the final `ddc_stepmania/` conflict-marker files as quarantined legacy-subtree content.
- Added `scripts/audit_refresh_readiness.py` and `docs/SSC_REFRESH_READINESS_2026-04-04.md` to record the exact state of the prepared `.ssc`-inclusive refresh work directory.
- Added resume-friendly skip flags to `scripts/train_all.py` so the refresh run can be restarted safely without repeating completed work.
- Launched the actual `.ssc`-inclusive refresh run and documented it in `docs/TRAINING_REFRESH_LAUNCH_2026-04-04.md`.
- Captured an in-flight runtime progress snapshot in `docs/TRAINING_REFRESH_PROGRESS_2026-04-04.md`, including initial onset checkpoint production.
- Captured a later progress snapshot in `docs/TRAINING_REFRESH_PROGRESS_2_2026-04-04.md`, showing onset checkpoint-set completion and transition into the first practical SymNet bucket stage.
- Captured a further progress snapshot in `docs/TRAINING_REFRESH_PROGRESS_3_2026-04-04.md`, showing the first practical bucket checkpoint (`dance-single_Easy/model_01.pth`) being produced.
- Captured another progress snapshot in `docs/TRAINING_REFRESH_PROGRESS_4_2026-04-04.md`, showing `dance-single_Easy` advancing to at least `model_02.pth` while remaining in progress.
- Captured another progress snapshot in `docs/TRAINING_REFRESH_PROGRESS_5_2026-04-04.md`, showing `dance-single_Easy` advancing to at least `model_03.pth` while later-epoch (`4/10`) log output was observed.
- Captured another progress snapshot in `docs/TRAINING_REFRESH_PROGRESS_6_2026-04-04.md`, showing `dance-single_Easy` advancing to at least `model_04.pth` while later-epoch (`5/10`) log output was observed.
- Captured another progress snapshot in `docs/TRAINING_REFRESH_PROGRESS_7_2026-04-04.md`, showing `dance-single_Easy` advancing to at least `model_05.pth` while later-epoch (`6/10`) log output was observed.
- Captured another progress snapshot in `docs/TRAINING_REFRESH_PROGRESS_8_2026-04-04.md`, showing `dance-single_Easy` reaching a full observed 10-checkpoint set and `dance-single_Medium` advancing to at least `model_06.pth` while later-epoch (`7/10`) log output was observed.
- Captured another progress snapshot in `docs/TRAINING_REFRESH_PROGRESS_9_2026-04-04.md`, showing the run staying alive deeper into `Epoch 7/10` with artifact recency still indicating `dance-single_Medium` as the active practical bucket frontier.
- Captured another progress snapshot in `docs/TRAINING_REFRESH_PROGRESS_10_2026-04-04.md`, showing `dance-single_Medium` advancing to at least `model_07.pth` while the active log progressed into `Epoch 8/10`.
- Captured another progress snapshot in `docs/TRAINING_REFRESH_PROGRESS_11_2026-04-04.md`, showing the run remaining alive while the monitored log progressed substantially deeper into the late portion of `Epoch 8/10` with `dance-single_Medium` still the active frontier.
- Captured another progress snapshot in `docs/TRAINING_REFRESH_PROGRESS_12_2026-04-04.md`, showing `dance-single_Medium` advancing to at least `model_08.pth`, completion of `Epoch 8/10` validation, and transition into `Epoch 9/10`.
- Captured another progress snapshot in `docs/TRAINING_REFRESH_PROGRESS_13_2026-04-04.md`, showing `dance-single_Medium` advancing to at least `model_09.pth`, transition into `Epoch 10/10`, and two active Python processes visible while the run continued without interruption.
- Captured another progress snapshot in `docs/TRAINING_REFRESH_PROGRESS_14_2026-04-04.md`, showing all practical single-mode buckets appearing complete and `dance-double_Easy` advancing to at least `model_08.pth` as the active frontier.
- Captured another progress snapshot in `docs/TRAINING_REFRESH_PROGRESS_15_2026-04-04.md`, showing `dance-double_Easy` advancing to at least `model_09.pth` and the active log progressing into `Epoch 10/10` for the current double-mode bucket.
- Added `docs/TRAINING_REFRESH_COMPLETION_2026-04-04.md`, recording the first full completion-state inventory: all practical single/double placement buckets show complete checkpoint sets and refreshed FFR artifacts are present for both `dance-single` and `dance-double`.
- Added `scripts/package_refresh_bundle.py` to package a completed refresh work directory into a cleaner deployment bundle layout.
- Added `docs/REFRESH_DEPLOYMENT_AND_ARROWVORTEX_VERIFICATION_2026-04-04.md`, documenting refreshed runtime compatibility, ArrowVortex-oriented usage paths, and local bundle-export strategy.
- Verified `AutoChart` initialization succeeds with `data/ssc_refresh_work/models` and `data/ssc_refresh_work/ffr_models`.
- Validated bundle packaging in dry-run mode via `python scripts/package_refresh_bundle.py data/ssc_refresh_work C:/Users/hyper/AppData/Local/Temp/ssc_refresh_bundle --latest_only --dry_run`.
- Updated `README.md` and `models_v3/README.md` to reference the refreshed local artifact paths and bundle-packaging command.
- Replaced the conflicted root `README.md` with a clean current-state overview.
- Updated versioning/documentation files to `0.2.32`.

## Key Findings

- Beginner placement was not part of the final 8-run practical export plan.
- The active `.ssc`-inclusive refresh has now progressed beyond the first practical bucket: `dance-single_Easy` shows a full observed 10-checkpoint set and `dance-single_Medium` has already accumulated at least 6 checkpoints.
- The refresh now appears complete for the core user-requested retraining objective: all practical single/double placement buckets show complete checkpoint sets, and refreshed floating-point FFR artifacts are present for both `dance-single` and `dance-double`.
- The refreshed artifact layout is structurally compatible with the current AutoChart / ArrowVortex-oriented runtime path: `models_dir=data/ssc_refresh_work/models` and `ffr_dir=data/ssc_refresh_work/ffr_models` initialize successfully.
- `dance-double_Beginner` has effectively no usable dataset size in the observed official-pack corpus.
- The DDC symbolic training path does not automatically collapse chart content to tap-only tokens.
- The difficulty evaluator *does* currently reduce charts to tap notes only, so shock arrows/mines/holds/rolls/lifts/fakes are not fully represented there.
- `.ssc` support now exists in extraction, but the full downstream filtering/training refresh against the expanded corpus is still pending.
- Corpus audit confirmed 20 `.ssc` files exist in the raw official DDR corpus and are now recoverable through the refreshed extractor.
- Corpus audit confirmed extracted note vocabulary contains substantial non-binary symbols: `2`, `3`, and `M`.
- Note-object semantic audit now documents the strongest supported interpretation of observed symbols:
  - `1` = tap
  - `2` = hold head
  - `3` = tail
  - `M` = mine
  - no observed `4`, `A`, `F`, `K`, or `L` in the refreshed official-pack extraction

## Important Repository Notes

- Large generated artifacts should not be pushed casually.
- `output_v132/` is heavyweight local training output and should remain local.
- local model exports are also large and should use a deliberate artifact/publication strategy.

## Recommended Next Steps

1. Run `scripts/package_refresh_bundle.py` without `--dry_run` to create a local deployment-ready bundle when ready.
2. Perform a small end-to-end ArrowVortex-oriented smoke test using `infer/ddc_server.py --models_dir data/ssc_refresh_work/models --ffr_dir data/ssc_refresh_work/ffr_models`.
3. Optionally run a representative post-refresh AutoChart validation pass against a few songs/audio inputs.
4. Add `dance-single_Beginner` placement training as an optional extension if still desired.
5. Extend the difficulty evaluator to include non-tap object semantics.
6. Optionally normalize the quarantined `ddc_stepmania/` conflict-marker files in a dedicated legacy-maintenance pass later.
7. Decide whether and how any final model artifacts should be published or distributed outside the local workspace.
