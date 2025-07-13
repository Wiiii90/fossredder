# Pflichtenheft – FOSSredder

## 1. Zielsetzung

Die Anwendung „FOSSredder“ dient der lokalen Verwaltung von Ausgaben für private Vermieter mit mehreren Immobilien. Ziel ist es, wiederkehrende und einmalige Kosten zu erfassen, automatisiert Kategorien zuzuordnen und daraus strukturierte Abrechnungen zu generieren. Zusätzlich können Kontoauszüge (PDF) sowie Excel-/CSV-Dateien verarbeitet werden, um relevante Daten zu extrahieren und in das System zu integrieren. Eine benutzerfreundliche GUI erleichtert Analyse und Verwaltung.

## 2. Anforderungen

### 2.1 Funktionale Anforderungen

#### Immobilien verwalten
- Verwaltung mehrerer Immobilien
- Attribute: Name, Adresse, optionale Beschreibung
- Zuordnung von Ausgaben und Abrechnungen pro Immobilie
- Funktionen: Hinzufügen, Bearbeiten, Löschen

#### Abrechnungen erstellen
- Erstellung von Abrechnungen (monatlich oder jährlich) für:
  - Einzelne Immobilien
  - Alle Immobilien zusammen
- Enthalten: 
  - Ausgaben mit Kategorie, Datum, Betrag
  - Saldo
- Anpassbare Elemente: Freitext, Logo, Fußzeile (für PDF-Ausgabe)

#### Kategorien verwalten
- Übersicht über alle Ausgabekategorien
- Funktionen: Hinzufügen, Bearbeiten, Löschen
- Markierung: „umlegbar“ oder „nicht umlegbar“

#### PDF-Verarbeitung
- PDF-Dateien können importiert werden (z. B. Kontoauszüge)
- Verarbeitung erfolgt in einer mehrstufigen Pipeline:
  1. PDF zu Bild-Konvertierung: Mithilfe der Bibliothek **Poppler** werden PDF-Dokumente in Bildformate umgewandelt, um eine präzisere Verarbeitung zu ermöglichen.
  2. Optische Zeichenerkennung (OCR): Die Bilder werden mit **Tesseract** analysiert, um Textinformationen zu extrahieren. Dies verbessert die Erkennung von Buchungsinformationen und bereitet die Daten für die nächste Verarbeitungsstufe vor.
  3. Extraktion von Buchungsinformationen mit LLM: Ein leistungsfähiges Sprachmodell (**Mistral**) wird verwendet, um die extrahierten Daten zu analysieren und Buchungsinformationen wie Betrag, Datum, Kategorie und zugehörige Immobilie zu identifizieren.
     - Das Modell schlägt dem Nutzer eine mögliche Buchung vor.
     - Der Nutzer kann die Vorschläge überprüfen, anpassen und final bestätigen, bevor die Daten gespeichert werden.
- Die Integration des Mistral-Modells erfolgt über die Bibliothek **llama.cpp** (siehe `extern/llama.cpp`), die eine flexible und skalierbare Einbindung verschiedener LLMs ermöglicht.
  - Die Modellwahl ist konfigurierbar; weitere Modelle können bei Bedarf eingebunden werden.
  - Die Architektur unterstützt sowohl lokale als auch serverbasierte Inferenz, wodurch eine spätere Skalierung (z. B. für größere Datenmengen oder parallele Verarbeitung) möglich ist.
  - Die Kommunikation mit dem Modell erfolgt über klar definierte Schnittstellen (`include/llama/ILlamaEngine.h`, `src/llama/LlamaEngine.cpp`), die eine einfache Erweiterung und Anpassung erlauben.
- Benutzerprüfung mit Möglichkeit zur:
  - Bestätigung
  - Korrektur
  - Ignorieren
- Jeder Eintrag bleibt nachvollziehbar auf ursprünglichen Textblock rückverfolgbar.
- Möglichkeit zur Markierung irrelevanter Textstellen durch Nutzer.

#### Excel-/CSV-Verarbeitung
- Import strukturierter Excel-/CSV-Dateien mit fixem Format
- Benutzerüberprüfung wie bei PDF-Import
- Exportfunktion (Excel/CSV) wählbar über GUI
- Speicherung an benutzerdefiniertem Ort

#### Datenverwaltung
- Manuelle und automatische Erfassung von Ausgaben
- Jedes Ausgabe-Objekt enthält:
  - Betrag
  - Ausgabekategorie
  - Datum
  - Umlegbarkeit
  - Zugehörige Immobilie

#### Backup & Wiederherstellung
- Erzeugung vollständiger Backups als ZIP-Archive über die GUI
- Inhalt: Alle Ausgaben, Immobilien, Kategorien, Konfigurationen
- Wiederherstellung ermöglicht Rücksetzung auf vorherigen Zustand

#### Benutzeroberfläche (GUI)
- Hauptfunktionen:
  - Immobilienverwaltung
  - PDF/CSV/Excel-Import
  - Kategorisierung
  - Abrechnungserstellung
  - Datenanalyse (Diagramme/Plots)
- Visualisierung:
  - Extrahierte Textabsätze auf originaler PDF-Seite (Screenshot/Textoverlay)
- Filter/Suche zum schnellen Datenzugriff
- Responsives Layout (auch für kleinere Bildschirme)
- Dark-/Lightmode umschaltbar

### 2.2 Nicht-funktionale Anforderungen
- Plattform: Windows 10 oder höher
- Sprache:
  - GUI: Deutsch
  - Codebasis: Englisch
- Datenspeicherung lokal (JSON)
- Erweiterbarkeit:
  - Grafische Auswertungen (z. B. interaktive Diagramme)
- Kein automatisches Buchen: Nutzer bestätigt jede Ausgabe
- Kein regex-basierter Parser: Stattdessen adaptive Heuristik

## 3. Systemarchitektur (geplant)

- Programmiersprache: C++20
- GUI-Toolkit: Qt6
- Buildsystem: CMake (ab Version 3.15, Generator: Ninja)
- Abhängigkeitsverwaltung: vcpkg
- Datenhaltung: JSON
- KI-Integration: 
  - Die LLM-Komponente ist modular aufgebaut und befindet sich im Ordner `extern/llama.cpp`.
  - Die Schnittstellen zur KI sind in `include/llama/ILlamaEngine.h` und `src/llama/LlamaEngine.cpp` implementiert.
  - Die Modellkonfiguration und -auswahl erfolgt über die Projektkonfiguration und kann flexibel angepasst werden.
  - Die Architektur erlaubt den Austausch des Modells (z. B. Wechsel von Mistral zu Llama 3 oder anderen GGUF-kompatiblen Modellen) ohne größere Codeänderungen.
- Projektstruktur:
```
fossredder/ 
├── docs/             # Dokumentation 
├── include/          # Header-Dateien 
│   ├── models/       # Datenmodelle (Property, Tenant, etc.) 
│   ├── views/        # GUI- oder Konsolenansichten 
│   ├── controllers/  # Steuerungsklassen 
│   ├── managers/     # Datenmanagement (z. B. für Immobilien, Auszüge)
│   ├── persistence/  # Persistenzschicht (JSON)
│   ├── poppler/      # Schnittstellen für PDF-Verarbeitung
│   ├── ocr/          # Schnittstellen für OCR-Verarbeitung
│   └── llama/        # Schnittstellen und Engines für LLM-Integration 
├── src/              # Quellcode 
│   ├── models/       # Implementierung der Datenmodelle 
│   ├── views/        # Implementierung der Ansichten 
│   ├── controllers/  # Implementierung der Steuerung 
│   ├── managers/     # Implementierung der Datenmanagement-Klassen
│   ├── persistence/  # Implementierung der Persistenzschicht
│   ├── poppler/      # Implementierung der PDF-Verarbeitung
│   ├── ocr/          # Implementierung der OCR-Verarbeitung
│   └── llama/        # Implementierung der LLM-Engine 
├── extern/llama.cpp/ # Externe Bibliothek für LLMs (z. B. Mistral, Llama, etc.) 
├── CMakeLists.txt 
└── README.md
```

## 4. Benutzeroberfläche (Skizze)

- Dashboard mit Übersicht aktueller Einnahmen/Ausgaben
- Navigationsleiste mit Zugriff auf:
  - Immobilien
  - Import
  - Kategorien
  - Abrechnungen
  - Analysen
- Export-/Backup-Schaltflächen oben rechts
- Filter- und Suchfunktionen in allen Datenansichten

## 5. Abgrenzung

- Kein Online- oder Cloud-Zugriff (in v1)
- Keine ELSTER-/Steuerintegration
- Keine Kontozugriffe oder automatisierte Bankdatenverknüpfung
- Keine Nutzerverwaltung oder Authentifizierung

## 6. Weiteres

- FOSS-Lizenzierung (MIT)
- Fokus auf Transparenz und Sicherheit
- Volle lokale Kontrolle über Daten
- Datenschutz: Keine Netzübertragung, keine Cloudbindung
- Nachvollziehbarkeit: Jeder Verarbeitungsschritt einsehbar und rückverfolgbar

Letzte Aktualisierung: 2025-07-13