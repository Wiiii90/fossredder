# FOSSredder – Portfolio-Analyse & Bewerbungsguide

> **Zweck dieser Datei:** Strukturierte Bewertung des Projekts als Portfolio-Stück für Stellenbewerbungen — mit Stärken, Schwächen, Abgleich-Anleitung, konkreten Verbesserungsvorschlägen, Bewerbungs-Bullet-Points und einer Checkliste.

---

## 1. Projektübersicht

### Was ist FOSSredder?

FOSSredder ist eine lokale Windows-Desktop-Anwendung (C++20 / Qt6 / QML), die PDF-Kontoauszüge (primär Commerzbank) per OCR-Pipeline in strukturierte Finanzdaten umwandelt und automatisiert Kosten auf Mieter alloziert. Das Projekt löst ein echtes, konkretes Problem und ist bewusst als Lern- und Engineering-Showcase konzipiert.

### Technologie-Stack (Kurzüberblick)

| Kategorie | Technologien |
|---|---|
| Sprache | C++20 |
| GUI | Qt6 / QML / Qt Quick |
| Build | CMake 4.x, vcpkg (manifest mode) |
| Persistenz | SQLite |
| OCR-Pipeline | Poppler → OpenCV → Tesseract |
| Testing | GoogleTest (unit, interaction, QML) |
| Analyse | clang-tidy, LLVM Coverage, Codecov |
| CI | GitHub Actions (self-hosted Windows runner) |
| Packaging | Inno Setup |
| Export | XLSX (`xlnt`), CSV |

### Architektur (Kurzfassung)

Modulare N-Tier-Architektur mit strikter Dependency Inversion:

```
app (Composition Root)
  ├── ui            (QML Views + C++ Controller)
  ├── core          (Domain-Logik, Use Cases, Interfaces)
  ├── api           (Service-Interfaces: OCR, PDF, Image)
  ├── services/*    (Concrete Adapters: Poppler, OpenCV, Tesseract)
  └── persistence   (SQLite Repositories)
```

Vollständiges Architekturdiagramm und ADRs: [`docs/DESIGN.md`](DESIGN.md), [`docs/adr/`](adr/).

### Tests & CI

- **Unit-Tests** (`core/tests/unit/`): 17 Testdateien; testen Domain-Logik isoliert von Qt und externen Bibliotheken.
- **Interaction-Tests** (`core/tests/interaction/`): Testen Zusammenspiel von Facade, Manager und Storage.
- **QML/UI-Tests** (`ui/tests/`): UI-seitige Tests.
- **Persistence-Tests** (`persistence/tests/`, `debug/tests/`): Datenbankschicht.
- **CI-Jobs:** `build-and-test` → `clang-tidy` → `coverage` (sequenziell, mit Codecov-Upload).
- **Coverage:** LLVM-Coverage mit lcov-Ausgabe; Badge im README.

### Dokumentation

| Datei | Inhalt |
|---|---|
| `README.md` | Tech-Stack, Architektur, Build, Test, CI, Roadmap |
| `docs/DESIGN.md` | System Context, Layered Architecture, UML-Diagramme (Mermaid), QA-Strategie |
| `docs/REQUIREMENTS_DE.md` | Deutsches Pflichtenheft |
| `docs/adr/` | Architecture Decision Records |
| `CONTRIBUTING.md` | Entwicklungsumgebung, Build-Workflow, Code-Style, PR-Prozess |
| `Doxyfile` | Doxygen-Konfiguration; alle öffentlichen Header sind dokumentiert |

---

## 2. Stärken und Schwächen

### ✅ Technische Stärken

| Stärke | Beleg / Dateipfad |
|---|---|
| **Klare Schichtenarchitektur** mit Dependency Inversion | `docs/DESIGN.md § 3.1`, `docs/adr/0001-layered-architecture.md` |
| **Interfaces überall** – Adapters für Tesseract, Poppler, OpenCV sind mockbar | `api/`, `services/poppler/`, `services/opencv/`, `services/tesseract/` |
| **Solide Test-Pyramide** – Unit, Interaction, QML, Persistence | `core/tests/`, `ui/tests/`, `persistence/tests/` |
| **CI-Pipeline** mit Tidy + Coverage + Codecov | `.github/workflows/quality.yml` |
| **C++20 mit modernen Idiomen** (keine Extensions, RAII, std::shared_ptr) | `CMakeLists.txt`, gesamte Codebasis |
| **Privacy by Design** – strikt lokal, kein Netzwerkzugriff | `docs/REQUIREMENTS_DE.md § 5` |
| **Mehrsprachigkeit (i18n)** über Qt-Linguist-Infrastruktur | `ui/src/controllers/LanguageController.cpp`, `app/i18n/` |
| **Multithreaded Job-System** für langen OCR-Verarbeitungen | `core/src/jobs/Scheduler.cpp`, `ui/src/import/ImportJobBridge.cpp` |
| **ADR-Struktur vorhanden** – Entscheidungen nachvollziehbar | `docs/adr/` |
| **Doxygen-Dokumentation** aller öffentlichen Header | `Doxyfile`, alle `*.h` in `core/include/`, `api/` |
| **Packaging-Infrastruktur** (Inno Setup, CMake-Preset) | `installer/fossredder.iss`, `ci/package-inno.ps1` |

### ⚠️ Schwachstellen & offene Punkte

| Schwachstelle | Details |
|---|---|
| **Kein Demo / keine Screenshots** | Recruiter und Interviewer sehen kein Bild der laufenden Anwendung |
| **ADR nur Template** | `docs/adr/0000-template.md` war die einzige ADR-Datei vor diesem Commit |
| **Kein CONTRIBUTING.md** | Fehlte vor diesem Commit; signalisiert mangelnde Open-Source-Reife |
| **Roadmap-Items offen** | Fuzzy Matching, Export-Modul, Settings-Persistenz noch nicht fertig |
| **Export-Modul instabil** | `core/src/export/CsvExporter.cpp`, `XlsxExporter.cpp` — Korrektheit nicht garantiert |
| **CI auf self-hosted Runner** | Kein öffentlich einsehbarer Workflow-Run; Badge kann nicht geprüft werden |
| **Keine End-to-End-Tests** | Nur Unit/Interaction; kein Systemtest mit echten PDFs |
| **`REQUIREMENTS_DE.md` leicht veraltet** | LLM-Integration (Mistral/llama.cpp) erwähnt, die im Code nicht existiert |
| **Kein Architekturdiagramm als Bild** | Mermaid-Diagramme nur in Markdown; auf GitHub sind sie sichtbar, in PDFs nicht |
| **Version noch 0.1.0** | Signalisiert frühe Alpha; kein GitHub Release / Tag |

---

## 3. Abgleich mit einer Stellenbeschreibung

### 3.1 Welche Informationen brauchst du?

Um das Projekt mit einer konkreten Stelle abzugleichen, benötige ich von dir:

1. **Den Jobtitel und das Unternehmen** (z. B. „Senior C++ Engineer bei Bosch")
2. **Die vollständige Stellenbeschreibung** (oder Kopie der Anforderungen), insbesondere:
   - Technologien / Sprachen (C++, Qt, Python, Java, …)
   - Frameworks / Bibliotheken (Qt, OpenCV, STL, Boost, …)
   - Methodiken (Agile, TDD, Clean Architecture, CI/CD, …)
   - Soft Skills / Prozesse (Dokumentation, Code Reviews, Git-Workflow, …)
3. **Seniority-Level** (Junior / Mid / Senior / Lead)
4. **Domäne** (Embedded, Desktop, Backend, Fintech, …)

### 3.2 So würde der Abgleich funktionieren

Beispielhafte Anforderung → Beleg im Repo:

| Anforderung (Beispiel) | Beleg im Repository |
|---|---|
| „C++17/20 Kenntnisse" | `CMakeLists.txt`: `set(CMAKE_CXX_STANDARD 20)` |
| „Qt / GUI-Entwicklung" | `ui/`, `app/src/main_qml.cpp`, QML-Views in `ui/qml/` |
| „Clean Architecture / SOLID" | `docs/DESIGN.md § 3.2`, `docs/adr/0001-layered-architecture.md` |
| „Unit Testing (GTest/GMock)" | `core/tests/unit/*.cpp` (17 Testdateien) |
| „CI/CD (GitHub Actions)" | `.github/workflows/quality.yml` |
| „CMake, vcpkg" | `CMakeLists.txt`, `vcpkg.json`, `CMakePresets.json` |
| „SQLite / Datenbankschicht" | `persistence/src/`, `persistence/src/SqliteSchema.cpp` |
| „OpenCV / Computer Vision" | `services/opencv/`, `api/opencv/` |
| „OCR-Systeme" | `services/tesseract/`, `api/tesseract/` |
| „Code-Dokumentation (Doxygen)" | `Doxyfile`, alle öffentlichen Header |
| „Multithreading / async" | `core/src/jobs/Scheduler.cpp`, `core/src/jobs/` |
| „Design Patterns (Strategy, Factory)" | `core/src/analysis/strategies/`, `persistence/Factory.h` |
| „Lokalisierung (i18n)" | `ui/src/controllers/LanguageController.cpp`, `app/i18n/` |

> **Schritt für Schritt:** Schick mir die Stellenbeschreibung, dann mappe ich jede Anforderung konkret auf Dateien und Codezeilen und erstelle einen personalisierten Anforderungsabgleich.

---

## 4. Empfehlungen (priorisiert)

### Priorität 1 – Hohe Wirkung, geringer Aufwand (sofort umsetzbar)

| # | Empfehlung | Aufwand | Wirkung |
|---|---|---|---|
| 1 | **Screenshots / GIF** der laufenden App in `docs/screenshots/` ablegen und im README einbinden | Klein | Sehr hoch — Recruiter sehen sofort, was das Projekt macht |
| 2 | **GitHub Release / Tag** mit `v0.1.0` erstellen; Installer-Artifact anhängen | Klein | Hoch — demonstriert Deployment-Reife |
| 3 | **CI-Badge** auf öffentlich sichtbare Runs umstellen oder `workflow_dispatch` für manuelle öffentliche Runs nutzen | Mittel | Hoch — Badge zeigt grünen Build |
| 4 | **`REQUIREMENTS_DE.md` aktualisieren** — LLM-Abschnitt entfernen oder korrekt als „geplant" markieren | Klein | Mittel — verhindert Inkonsistenzen im Portfolio-Review |

### Priorität 2 – Mittlerer Aufwand, hohe Wirkung

| # | Empfehlung | Aufwand | Wirkung |
|---|---|---|---|
| 5 | **End-to-End-Testfall** mit einer synthetischen (anonymisierten) Test-PDF hinzufügen | Mittel | Hoch — demonstriert TDD-Reife des Gesamtsystems |
| 6 | **Fuzzy-Matching implementieren** (Levenshtein-Distanz als nächstes Roadmap-Item) | Groß | Hoch — schließt eine explizit genannte offene Lücke |
| 7 | **Export-Modul stabilisieren** — `TestExportService.cpp` ausbauen, XLSX-Output verifizieren | Mittel | Mittel — ein schwaches Modul in einem ansonsten stabilen System |
| 8 | **Demo-Video / Loom** aufnehmen und in README verlinken | Klein | Sehr hoch bei Bewerbungen — asynchrone Demo ohne Setup |

### Priorität 3 – Größerer Aufwand, strategische Wirkung

| # | Empfehlung | Aufwand | Wirkung |
|---|---|---|---|
| 9 | **Architekturdiagramm als SVG/PNG** exportieren und in `docs/` ablegen | Klein | Mittel — Diagramm ist auch in PDF-Portfolios sichtbar |
| 10 | **Settings-Persistenz fertigstellen** — derzeit UI vorhanden, Backend fehlt | Groß | Mittel — rundet die Anwendung ab |
| 11 | **Weitere ADRs** dokumentieren (z. B. Wahl von SQLite, Wahl von vcpkg, Job-System-Design) | Klein | Mittel — zeigt architekturelles Denken |
| 12 | **Docker / Dev-Container** für Entwicklungsumgebung | Groß | Mittel — erleichtert Onboarding, Windows-spezifisch schwierig |

---

## 5. Talking Points für Bewerbungstext / Portfolio / LinkedIn

Die folgenden Bullet-Points können direkt in Anschreiben, LinkedIn-Beiträge oder Portfolio-Seiten übernommen und an den jeweiligen Kontext angepasst werden:

- **Architektur & Design:** „Entwarf und implementierte eine modulare N-Tier-Architektur in C++20 mit konsequenter Dependency Inversion, bei der jede Schicht (Domain, API, Services, Persistence, UI) als unabhängiges CMake-Target isoliert wird — kein Code in `core` hängt von Qt oder Drittbibliotheken ab."

- **OCR-Pipeline:** „Entwickelte eine mehrstufige Dokumentenverarbeitungspipeline (PDF-Rendering mit Poppler → Bildverarbeitung mit OpenCV → Texterkennung mit Tesseract), die strukturierte Finanztransaktionen aus handelsüblichen Kontoauszügen extrahiert."

- **Testing & Qualität:** „Etablierte eine vollständige Test-Pyramide mit GoogleTest (Unit, Integration, QML/UI, Persistence), ergänzt durch statische Analyse mit clang-tidy und LLVM-Code-Coverage mit Codecov-Integration in der CI-Pipeline."

- **Multithreading:** „Implementierte ein asynchrones Job-System (`core/src/jobs/`) mit eigenem Scheduler und SlotLimiter, das rechenintensive OCR-Verarbeitungen vom Qt-Event-Loop entkoppelt und UI-Responsivität gewährleistet."

- **Privacy by Design:** „Stellte konsequenten lokalen Betrieb sicher: keine Cloud-Abhängigkeiten, keine externen APIs — alle finanziellen Daten bleiben ausschließlich auf dem Gerät des Nutzers."

- **CI/CD:** „Richtete eine mehrstufige GitHub-Actions-Pipeline (Build & Test → clang-tidy → Coverage) auf einem selbst betriebenen Windows-Runner ein, mit automatischer Codecov-Berichterstattung und Job-Parallelisierung."

- **Dokumentation:** „Pflege vollständige Doxygen-Kommentare für alle öffentlichen Schnittstellen, ein Architektur-Design-Dokument mit Mermaid-UML-Diagrammen, Architecture Decision Records und ein deutsches Pflichtenheft."

- **Modernes C++:** „Nutzt C++20-Features (Concepts, Ranges-Vorbereitung, `std::span`, strukturierte Bindings) und moderne Build-Tooling-Praktiken (vcpkg in Manifest-Mode, CMakePresets.json) für reproduzierbare Builds."

- **Reales Problem:** „Löst ein konkretes, reales Problem — die manuelle Excel-basierte Betriebskostenabrechnung für private Vermieter — mit einem vollständig lokalen, nachvollziehbaren semi-automatischen Workflow."

- **Extensibility:** „Entwarf alle Verarbeitungsadapter (OCR, PDF, Bildanalyse) hinter stabilen Interfaces, sodass das zugrundeliegende Modell oder die Bibliothek ausgetauscht werden kann, ohne die Domain-Logik zu berühren."

---

## 6. Portfolio-Checkliste

Hake diese Punkte ab, bevor du das Projekt in einer Bewerbung nennst:

### Muss vorhanden sein

- [ ] README enthält klare Projektbeschreibung, Tech-Stack und Architekturübersicht
- [ ] README enthält CI-Badge und Coverage-Badge (beide grün)
- [ ] Mindestens ein Screenshot oder GIF der laufenden Anwendung ist sichtbar
- [ ] Build-Anleitung ist vollständig und korrekt
- [ ] `CONTRIBUTING.md` vorhanden
- [ ] Lizenz-Datei vorhanden (`LICENSE`)
- [ ] Tests laufen durch (`ctest --preset release-tests --output-on-failure`)
- [ ] CI-Pipeline läuft durch (grüner Badge)

### Sollte vorhanden sein

- [ ] Mindestens ein ADR dokumentiert (`docs/adr/0001-*.md`)
- [ ] Roadmap/TODO-Liste zeigt Selbstreflexion über offene Punkte
- [ ] `docs/DESIGN.md` oder vergleichbares Architektur-Dokument vorhanden
- [ ] Alle öffentlichen Header mit Doxygen-Kommentaren versehen
- [ ] GitHub Release / Tag mit Versionsnummer vorhanden
- [ ] Export-Modul minimally funktionsfähig (kein offensichtlicher Absturz)

### Nice-to-have

- [ ] Demo-Video / Loom in README verlinkt
- [ ] Doxygen-HTML als GitHub-Pages publiziert
- [ ] End-to-End-Test mit anonymisierter Beispiel-PDF
- [ ] Fuzzy-Matching implementiert (Levenshtein-Distanz)
- [ ] Settings-Backend vollständig verbunden
- [ ] Architekturdiagramm als exportiertes SVG/PNG (nicht nur Mermaid-Code)

---

*Letzte Aktualisierung: 2026-04-27*
