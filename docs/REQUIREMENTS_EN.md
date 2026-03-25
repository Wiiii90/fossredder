# Requirements — FOSSredder

## 1. Purpose

FOSSredder is a desktop application for managing expenses for private landlords with multiple properties. The goal is to capture recurring and one-off expenses, assign categories automatically where possible, and generate structured reports. The system supports importing PDF bank statements as well as CSV/XLSX files and provides a user review flow before data is persisted.

## 2. Requirements

### 2.1 Functional Requirements

#### Property management
- Manage multiple properties
- Attributes: name, address, optional description
- Associate expenses and reports with specific properties
- Actions: create, edit, delete

#### Reporting
- Generate reports (monthly or yearly) for single or all properties
- Reports include categorized expenses with date, amount and balances
- Customizable report elements (free text, logo, footer)

#### Categories
- Manage categories (create/edit/delete)
- Mark categories as allocatable or non-allocatable

#### PDF processing
- Import PDF statements
- Multi-stage processing pipeline:
  1. Render PDF pages to images (Poppler)
  2. Preprocess images and detect tables/regions (OpenCV)
  3. OCR (Tesseract)
  4. Candidate extraction and suggestion (adapter-based LLM optional)
- User review with confirm/correct/ignore options
- Traceability to original PDF regions

#### CSV/XLSX processing
- Import structured CSV/XLSX files
- User review like for PDF imports
- Export to CSV/XLSX

#### Persistence
- Runtime storage: SQLite (implemented in `persistence/`)
- Export/Import formats for data interchange: CSV, JSON

#### Backup & Restore
- Create full backups (ZIP) via UI
- Restore to revert application state

### 2.2 Non-functional Requirements
- Platform: Windows 10 or higher
- UI language: German (primary), English (docs)
- Language: C++20
- Build system: CMake (minimum 4.0.1)
- Dependency management: vcpkg (manifest mode)

## 3. Architecture
- See `docs/design/` for architecture sketches and design notes.

Last updated: 2026-03-25
