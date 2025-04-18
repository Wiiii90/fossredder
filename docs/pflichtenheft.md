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
- Abschnittsweises Auslesen mit Texterkennung inkl. Textpositionen (z. B. via Poppler)
- Heuristische Gruppierung von Textblöcken in Absätze:
  - Berücksichtigung von vertikalem Abstand, Stil, Seitenstruktur
- Extraktion potenziell relevanter Inhalte (z. B. „Hausmeisterkosten 202,32 €“)
- Vorschläge zur Kategorie und zum Betrag durch System
- Benutzerprüfung mit Möglichkeit zur:
  - Bestätigung
  - Korrektur
  - Ignorieren
- Jeder Eintrag bleibt nachvollziehbar auf ursprünglichen Textblock rückverfolgbar
- Möglichkeit zur Markierung irrelevanter Textstellen durch Nutzer

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
- Datenspeicherung lokal (SQLite oder JSON)
- Erweiterbarkeit:
  - Grafische Auswertungen (z. B. interaktive Diagramme)
  - Scanner-Integration
  - Optionale Cloud-Anbindung
- Kein automatisches Buchen: Nutzer bestätigt jede Ausgabe
- Kein regex-basierter Parser: Stattdessen adaptive Heuristik

## 3. Systemarchitektur (geplant)

- Programmiersprache: C++17 oder höher
- GUI-Toolkit: Qt6
- Buildsystem: CMake
- Abhängigkeitsverwaltung: vcpkg
- Datenhaltung: SQLite oder JSON
- Projektstruktur:
```
fossredder/
├── docs/         # Dokumentation
├── include/      # Header-Dateien
│   ├── models/   # Datenmodelle (Property, Tenant, etc.)
│   ├── views/    # GUI- oder Konsolenansichten
│   └── controllers/ # Steuerungsklassen
├── src/          # Quellcode
│   ├── models/   # Implementierung der Datenmodelle
│   ├── views/    # Implementierung der Ansichten
│   └── controllers/ # Implementierung der Steuerung
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

- FOSS-Lizenzierung (MIT oder GPLv3)
- Fokus auf Transparenz und Sicherheit
- Volle lokale Kontrolle über Daten
- Datenschutz: Keine Netzübertragung, keine Cloudbindung
- Nachvollziehbarkeit: Jeder Verarbeitungsschritt einsehbar und rückverfolgbar

Letzte Aktualisierung: 2025-04-18
