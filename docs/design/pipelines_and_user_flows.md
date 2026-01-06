# Pipelines and User Flows — FOSSredder

Date: 2026-01-06
Author: Wilhelm Altemeier
Status: Draft

## Pipelines

### Import pipeline
Reference: `docs/design/appendix/diagrams/import_sequence.mmd`

Pipeline stages
1. Poppler (render / extract)
   - Uses `api::poppler` request/response types and writes render/extract artifacts into the session `outputDir`.
2. OpenCV (preprocessing / detection)
   - Uses `api::opencv` requests to produce denoised/masked images, detected tables and crops.
3. Tesseract (OCR)
   - Uses `api::tesseract` requests and returns OCR outputs (text, TSV, words, tables).
4. Candidate Extraction / Parsing
   - Transforms OCR and preprocessing artifacts into domain `Statement`/`Transaction` candidates.
5. Domain Matching
   - Uses `ActorMatcher` to match extracted tokens to existing `Actor`/`Property` objects and produce ranked suggestions.
6. Review Presentation
   - Delivers candidates, suggestions and artifacts to the UI for user verification.
7. Persist
   - Persists confirmed domain data via the application persistence flow using transactions.

Notes
- Import implementation is produced by `createImportStatement(poppler, openCv, tesseract, dbg)` and wired in `app/src/main.cpp`.
- `ImportRequest` supplies `sourcePath`, `runRoot` and `runIdPrefix`; `ImportResult` returns a `Statement` and artifacts.

### Export pipeline (placeholder)
- Export pipeline details are not implemented in code yet. If/when added, document the stages here or in a dedicated `docs/design/pipelines/export.md`.

## User flows

### Review flow
- The UI obtains structured import results via `UiImportController` and presents per-candidate context (page, bounding box, extracted text) and suggested fields (amount, date, description, suggested actor/property).
- Per-candidate actions: Accept, Edit, Reject, Create Actor/Property, Mark Duplicate, Add Note.
- Bulk actions: apply changes to multiple candidates and bulk accept low-risk suggestions.
- New actors/properties created in review become available for matching within the same session.

## Persistence handoff
- `AppStateStore::save` performs atomic persistence using SQLite transactions and returns impact information.
- The application uses persistence APIs via `FileController` callbacks for load/save operations.

## Error handling
- Pipeline errors propagate to controllers and UI. Persistence operations use transactions and may raise errors on failure.