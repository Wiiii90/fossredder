# Infra Testing Matrix

## Purpose

This document defines the concrete test matrix for the `infra` target family:
`archive`, `image-processing`, `pdf-rendering`, and `text-recognition`.

The goal is not to describe implementation details, but to capture the observable
contracts each infrastructure module must satisfy so the module boundaries stay
stable, replaceable, and easy to reason about.

## Scope

Included in this matrix:
- `infra/archive/*`
- `infra/image-processing/*`
- `infra/pdf-rendering/*`
- `infra/text-recognition/*`

Out of scope:
- `core` domain and application rules
- UI behavior
- persistence schema and repository behavior
- end-to-end app flows that only happen after multiple infra modules are combined

## Testing Principles

- Test each infra module in isolation first.
- Prefer file-backed fixtures and deterministic input data.
- Verify both success and failure paths.
- Check observable outputs, not private helper internals.
- Keep tests aligned with the module boundary:
  - `archive` tests package creation and file layout
  - `image-processing` tests crop/mask/denoise/detect behavior
  - `pdf-rendering` tests page rendering and extracted metadata
  - `text-recognition` tests OCR extraction and language/config handling

## Recommended Test Layout

- `infra/archive/tests/unit`
- `infra/archive/tests/integration`
- `infra/image-processing/tests/unit`
- `infra/image-processing/tests/integration`
- `infra/pdf-rendering/tests/unit`
- `infra/pdf-rendering/tests/integration`
- `infra/text-recognition/tests/unit`
- `infra/text-recognition/tests/integration`

## 1. Archive

### Module intent

`archive` is responsible for packaging export artifacts into ZIP archives.
It must preserve directory structure, produce deterministic output where possible,
and fail cleanly on invalid input or unsupported formats.

### Behavioral matrix

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| ARC-001 | Creates a ZIP archive from a flat directory | Integration | Temporary source directory with a few files | Call `IArchive::create(sourceDirectory, archivePath, Zip)` | Archive file is created and contains all files |
| ARC-002 | Preserves relative file paths | Integration | Source directory with nested subfolders | Create archive | Nested paths inside the ZIP match the source-relative layout |
| ARC-003 | Handles empty source directories | Integration | Empty source directory | Create archive | Archive is created successfully or the module returns a documented failure state |
| ARC-004 | Rejects unsupported archive formats | Unit | Archive adapter instance | Call `create(..., format != Zip)` | Returns `false` without writing output |
| ARC-005 | Rejects missing source directory | Unit | Non-existent source path | Call `create()` | Returns `false` |
| ARC-006 | Rejects unwritable archive path | Integration | Output path in a non-writable location | Call `create()` | Returns `false` and does not leave a partial archive behind |
| ARC-007 | Produces readable ZIP output | Integration | Valid source directory | Create archive and open it with a ZIP reader | The archive can be opened and enumerated |
| ARC-008 | Preserves binary file contents | Integration | Source directory with binary and text files | Create archive and extract files | Extracted bytes match the originals |
| ARC-009 | Handles files with spaces and Unicode-safe names | Integration | Source files with spaces or punctuation | Create archive | Archive entries retain the expected relative names |
| ARC-010 | Handles repeated creation over the same output path | Integration | Existing archive path | Re-create archive to same path | Old archive is replaced deterministically |

### Structural tests

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| ARC-S-001 | Adapter class is the only public implementation surface | Unit | Adapter header available | Inspect public API usage in tests | Tests depend on `IArchive` plus the adapter class only |
| ARC-S-002 | ZIP dependency stays isolated inside infra | Structural | Build target for `infra/archive` | Review include dependencies | `core` does not include `zip.h` directly |
| ARC-S-003 | Archive packaging remains replaceable | Unit | Mock or fake archive adapter | Use the adapter through `IArchive` | Callers interact through the port, not the ZIP library |

## 2. Image Processing

### Module intent

`image-processing` provides OpenCV-backed image operations for the import workflow:
crop, denoise, mask, and detect.
The module must remain a technical adapter boundary around the OpenCV dependency
and keep the public port contracts stable.

### Behavioral matrix

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| IMG-001 | Crops one image into multiple regions | Integration | Source image and multiple valid rectangles | Call crop adapter | Output files/bytes are created for each valid rectangle |
| IMG-002 | Cropping clamps rectangles to image bounds | Unit | Rectangle partially outside the source image | Crop image | Resulting crop is clipped safely and does not crash |
| IMG-003 | Cropping ignores empty rectangles | Unit | Zero-sized or invalid rectangles | Call crop adapter | Invalid rectangles are skipped |
| IMG-004 | Crop output format switches between JPG and PNG | Unit | Same input image | Run crop with JPG and PNG requests | Encoded bytes/file extensions match the requested format |
| IMG-005 | Denoise preserves valid input dimensions | Unit | Valid noisy image | Call denoise adapter | Output image exists and has matching dimensions |
| IMG-006 | Denoise supports median mode | Unit | Noisy source image | Request median denoise | Result is produced with the median path |
| IMG-007 | Denoise supports Gaussian mode | Unit | Noisy source image | Request Gaussian denoise | Result is produced with the Gaussian path |
| IMG-008 | Denoise supports bilateral mode | Unit | Noisy source image | Request bilateral denoise | Result is produced with the bilateral path |
| IMG-009 | Masking returns both mask and whiteout outputs | Integration | Renderable image with table or text content | Call mask adapter | Mask bytes and masked image bytes are produced |
| IMG-010 | Masking writes output file when requested | Integration | Writable output directory | Call mask adapter with output dir | Masked file is written to disk |
| IMG-011 | Masking handles near-empty images safely | Unit | Very small or blank image | Call mask adapter | Returns a valid result or a documented empty result without crashing |
| IMG-012 | Detection finds text blocks on structured images | Integration | Image with clear text regions | Call detect adapter | Detected boxes are returned |
| IMG-013 | Detection finds table regions on grid-like images | Integration | Image with visible table grid | Call detect adapter | Table cells or table blocks are returned |
| IMG-014 | Detection returns stable coordinates relative to the source image | Unit | Image with known geometry | Call detect adapter | Returned boxes map to the expected coordinates |
| IMG-015 | Detection tolerates images with little structure | Unit | Blank or low-contrast image | Call detect adapter | Result is empty or low-confidence, not a crash |
| IMG-016 | Debug output is optional | Unit | Debugger disabled | Run any image-processing operation | No debug writes are attempted |
| IMG-017 | Debug output is written when enabled | Integration | Debugger enabled | Run any image-processing operation | Expected debug artifacts are written |

### Structural tests

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| IMG-S-001 | Public API remains capability-based | Unit | Port headers available | Inspect adapter signatures | API speaks in crop, mask, denoise, detect terms |
| IMG-S-002 | OpenCV dependency stays isolated in infra | Structural | Build target for `infra/image-processing` | Review include dependencies | `core` does not include OpenCV headers directly |
| IMG-S-003 | Adapter naming stays uniform | Structural | Public headers and sources | Inspect class names | Public classes follow the `OpenCv...Adapter` naming rule |
| IMG-S-004 | Internal helpers do not leak into core | Structural | Port and adapter code | Check include graph | `core` only sees ports, not internal OpenCV helpers |

## 3. PDF Rendering

### Module intent

`pdf-rendering` provides Poppler-backed PDF document rendering and page metadata extraction.
It must be able to render pages, extract page text metadata, and serialize the
observable page information in a stable form.

### Behavioral matrix

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| PDF-001 | Opens a valid PDF document | Integration | Small valid PDF fixture | Call render or extract entry point | Document opens and pages are accessible |
| PDF-002 | Rejects missing PDF input | Unit | Non-existent file path | Call render or extract entry point | Operation fails cleanly |
| PDF-003 | Renders all pages of a multi-page PDF | Integration | PDF with multiple pages | Render document | One rendered result per page is produced |
| PDF-004 | Writes rendered page images to disk | Integration | Writable output directory | Render document with file output enabled | Page image files are written |
| PDF-005 | Returns in-memory PNG bytes for rendered pages | Integration | Valid PDF | Render document | Page image bytes are populated |
| PDF-006 | Preserves page dimensions in metadata | Unit | PDF with known page size | Extract metadata | Reported page width and height match the document |
| PDF-007 | Extracts text elements from page content | Integration | Text-bearing PDF page | Extract metadata | Text elements are returned with non-empty text |
| PDF-008 | Extracted bounding boxes are stable and page-relative | Integration | PDF with known text positions | Extract metadata | Text boxes are consistent with the page geometry |
| PDF-009 | Debug metadata JSON is written when enabled | Integration | Enabled debugger | Extract metadata | JSON artifact is written to debug output |
| PDF-010 | Debug image output is written when enabled | Integration | Enabled debugger | Render document | Rendered page bytes are written to debug output |
| PDF-011 | Cancel flag stops page iteration | Integration | Multi-page PDF and active cancel flag | Start render or extract with cancellation set | Processing stops early and returns partial or empty results |
| PDF-012 | Rendering tolerates unsupported page content gracefully | Unit | PDF with unusual content | Render page | Adapter returns a failure or partial result without crashing |

### Structural tests

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| PDF-S-001 | Public API remains capability-based | Unit | Port headers available | Inspect adapter signatures | API speaks in render and metadata terms |
| PDF-S-002 | Poppler dependency stays isolated in infra | Structural | Build target for `infra/pdf-rendering` | Review include dependencies | `core` does not include Poppler headers directly |
| PDF-S-003 | Adapter naming stays uniform | Structural | Public headers and sources | Inspect class names | Public classes follow the `Poppler...Adapter` naming rule |
| PDF-S-004 | Metadata serialization stays internal to infra | Structural | Source files available | Review include graph | JSON and Poppler helpers do not leak into `core` |

## 4. Text Recognition

### Module intent

`text-recognition` provides Tesseract-backed OCR and word extraction from image bytes.
It must handle tessdata resolution, engine mode configuration, and stable word/text
extraction behavior.

### Behavioral matrix

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| OCR-001 | Recognizes text from valid image bytes | Integration | Image bytes containing readable text | Call text extraction entry point | Text output is non-empty and meaningful |
| OCR-002 | Extracts word list from valid image bytes | Integration | Image bytes containing readable text | Call OCR word extraction | Word entries with boxes and confidence are returned |
| OCR-003 | Handles empty byte input safely | Unit | Empty byte array | Call extraction entry point | Returns empty text and no words |
| OCR-004 | Resolves tessdata from explicit path | Unit | Valid custom tessdata path | Call extraction entry point | The provided path is used successfully |
| OCR-005 | Falls back to environment tessdata path | Integration | `TESSDATA_PREFIX` set | Call extraction entry point | Environment-based tessdata resolution succeeds |
| OCR-006 | Falls back to bundled tessdata path | Integration | No explicit path and no env path | Call extraction entry point | Bundled `res/tessdata` is used when available |
| OCR-007 | Respects default language fallback | Unit | Recognition settings without language | Call extraction entry point | Language defaults to `deu` |
| OCR-008 | Honors custom language selection | Unit | Recognition settings with language | Call extraction entry point | Requested language is passed to the OCR engine |
| OCR-009 | Honors page segmentation mode | Unit | Recognition settings with a custom PSM | Call extraction entry point | OCR engine uses the requested PSM |
| OCR-010 | Honors engine mode selection | Unit | Recognition settings with a custom engine mode | Call extraction entry point | OCR engine uses the requested engine mode |
| OCR-011 | Applies character whitelist when configured | Unit | Recognition settings with whitelist | Call extraction entry point | OCR engine receives whitelist configuration |
| OCR-012 | Preserves inter-word spacing when configured | Unit | Recognition settings with spacing enabled | Call extraction entry point | Output reflects preserved word spacing |
| OCR-013 | Writes debug text output when enabled | Integration | Enabled debugger | Extract text or words | Debug text artifacts are written |
| OCR-014 | Returns stable word boxes and confidence values | Integration | Text-bearing image | Extract words | Word boxes and confidence are populated consistently |
| OCR-015 | Handles unreadable images without crashing | Unit | Corrupt or unreadable image bytes | Call extraction entry point | Returns empty text and empty word list |

### Structural tests

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| OCR-S-001 | Public API remains capability-based | Unit | Port headers available | Inspect adapter signatures | API speaks in recognition and word extraction terms |
| OCR-S-002 | Tesseract dependency stays isolated in infra | Structural | Build target for `infra/text-recognition` | Review include dependencies | `core` does not include Tesseract headers directly |
| OCR-S-003 | Adapter naming stays uniform | Structural | Public headers and sources | Inspect class names | Public classes follow the `Tesseract...Adapter` naming rule |
| OCR-S-004 | Tessdata resolution stays inside infra | Structural | Source files available | Review include graph | Tessdata lookup logic remains local to the module |

## 5. Cross-module consistency checks

| ID | Scope | Layer | Setup | Action | Expected |
|---|---|---|---|---|---|
| INF-001 | All infra modules expose a single, clear capability boundary | Structural | Public headers for all modules | Compare public APIs | Each module speaks only about its own responsibility |
| INF-002 | All infra modules keep backend dependencies hidden | Structural | Core and infra headers | Inspect includes | Backend headers do not leak into `core` |
| INF-003 | All infra modules use consistent Doxygen file headers | Structural | Public and source files | Inspect top-of-file docs | Each file has a clear `@file` and `@brief` header |
| INF-004 | All infra modules preserve deterministic behavior on repeated input | Integration | Stable fixtures | Re-run the same operation twice | Outputs are equivalent or documented as intentionally variable |
| INF-005 | All infra modules fail cleanly on invalid input | Unit | Invalid fixtures | Execute the public operation | Return value or result object reports failure without crashes |

## 6. Direct implementation order

If these tests are implemented in order, the following sequence is the most natural:
1. `archive` integration and structural tests
2. `image-processing` crop and denoise tests
3. `image-processing` mask and detect tests
4. `pdf-rendering` render and metadata tests
5. `text-recognition` OCR extraction and config tests
6. cross-module consistency tests

