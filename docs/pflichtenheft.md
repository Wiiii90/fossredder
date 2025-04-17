# Pflichtenheft – FOSSredder

## 1. Zielsetzung
Die Anwendung „FOSSredder“ dient der einfachen, lokalen Verwaltung von Ausgaben für einen privaten Vermieter mit mehreren Immobilien. 
Ziel ist es, wiederkehrende und einmalige Kosten zu erfassen, diese automatisiert bestimmten Kategorien zuzuordnen und daraus strukturierte Abrechnungen zu generieren.
Zusätzlich soll das System in der Lage sein, Kontoauszüge im PDF-Format sowie Excel- oder CSV-Dateien zu verarbeiten, relevante Daten zu extrahieren und diese in die Verwaltung der Ausgaben zu integrieren.
Eine benutzerfreundliche Oberfläche ermöglicht die Analyse und Verwaltung der Daten. 

## 2. Anforderungen

### 2.1 Funktionale Anforderungen
Das System soll folgende Funktionen bereitstellen:

#### Immobilien verwalten
- Das System ermöglicht die Verwaltung mehrerer Immobilien.
- Jede Immobilie hat folgende Attribute:
  - Name der Immobilie
  - Adresse
  - Beschreibung (optional)
- Ausgaben und Abrechnungen können einer spezifischen Immobilie zugeordnet werden.
- Das System bietet die Möglichkeit, neue Immobilien hinzuzufügen, bestehende zu bearbeiten oder zu löschen.

#### Abrechnungen erstellen
- Das System erstellt Abrechnungen (zum direkten Kopieren oder als PDF) für einen wählbaren Zeitraum (monatlich oder jährlich).
- Abrechnungen für:
  - Eine spezifische Immobilie
  - Alle Immobilien zusammen
- Die Abrechnung enthält:
  - Alle relevanten Ausgaben samt Ausgabekategorie, sortiert nach Datum.
  - Saldo.
- Benutzerdefinierte Elemente wie ein Text, Logo oder eine Fußzeile können in die Abrechnung zur Generierung einer kompletten PDF integriert werden.

#### Kategorien verwalten
- Das System bietet eine Übersicht über Ausgabekategorien.
- Kategorien können hinzugefügt, bearbeitet oder gelöscht werden.
- Kategorien können als „umlegbar“ oder „nicht umlegbar“ markiert werden, um die spätere Kostenabrechnung zu erleichtern.

#### PDF-Verarbeitung
- Das System ist in der Lage, PDF-Dateien (z. B. Kontoauszüge) einzulesen und zu verarbeiten.
- Der Nutzer übermittelt dem System eine PDF-Datei, die vom System abschnittsweise gelesen wird.
- Das System extrahiert relevante Informationen aus der PDF, insbesondere:
  - Höhe des Betrags
  - Typ der Ausgabekategorie
  - Datum der Abbuchung
- Der Nutzer überprüft die extrahierten Daten und kann diese manuell korrigieren.
- Vor dem Speichern eines Eintrags muss der Nutzer die Daten bestätigen.
- Das System speichert die Einträge mit den folgenden Attributen:
  - Höhe des Betrags
  - Typ der Ausgabekategorie
  - Datum der Abbuchung
  - Umlegbar (Ja/Nein)
  - Zugehörige Immobilie
- Das System bietet eine Verwaltungsmöglichkeit für bisherige Einträge, auch aus vorherigen Kontoauszügen.

#### Import und Export von Excel- und CSV-Dateien
- Das System ermöglicht den Import von Excel- oder CSV-Dateien, die bereits strukturierte Daten enthalten.
- Die Dateien müssen ein festgelegtes Format einhalten, um korrekt eingelesen zu werden.
- Nach dem Import können die Daten wie bei der PDF-Verarbeitung überprüft, bearbeitet und gespeichert werden.
- Daten können als CSV oder Excel-Datei exportiert werden, um sie in anderen Programmen weiterzuverarbeiten.
- Der Benutzer wählt das Zielformat und den Speicherort über die grafische Oberfläche aus.

#### Backups und Wiederherstellung
- Das System bietet die Möglichkeit, alle Daten in einer ZIP-Datei zu sichern.
- Backups enthalten alle Daten des Systems, einschließlich Kategorien, Ausgaben, Immobilien und interner Strukturen.
- Backups können über die GUI erstellt und wiederhergestellt werden.
- Die Wiederherstellung ermöglicht es, das System in den Zustand eines früheren Backups zurückzusetzen.

#### Benutzeroberfläche
- Die GUI umfasst folgende Funktionen:
  - Verwaltung von Immobilien.
  - Import von PDF-Dateien (z. B. Kontoauszüge).
  - Import und Export von Excel- oder CSV-Dateien.
  - Verwaltung von Kategorien.
  - Erstellung von Abrechnungen (für einzelne Immobilien oder alle zusammen).
  - Übersicht über die aktuellsten Ausgaben und Einnahmen.
  - Analyse-Tools, die es ermöglichen, Daten in Form von Diagrammen und Plots zu visualisieren.
- Das Layout ist responsiv und passt sich an verschiedene Bildschirmgrößen an.
- Optional ist ein Dunkel-/Hellmodus wählbar.

### 2.2 Nicht-funktionale Anforderungen
- Die Anwendung läuft auf Windows 10 oder höher.
- Die Benutzeroberfläche ist in deutscher Sprache gehalten; der Code in Englisch.
- Es erfolgt ausschließlich lokale Datenspeicherung (z. B. JSON oder SQLite).
- Die Software ist erweiterbar um zukünftige Features wie:
  - Erweiterte grafische Auswertungen (z. B. interaktive Diagramme).
  - Scanner-Integration zur Belegerfassung.
  - Synchronisation mit Cloud-Diensten (optional).

## 3. Systemarchitektur (geplant)
- Programmiersprache: C++17 oder höher
- GUI-Toolkit: Qt6 (in Kombination mit CMake)
- Build-System: CMake
- Projektstruktur:
  - `build/` – Build-Ausgaben
  - `docs/` – Dokumentation
  - `include/` – Header-Dateien
    - `models/` – Datenmodelle (z. B. Kategorien, Ausgaben, Immobilien)
    - `views/` – Ansichten (z. B. GUI-Komponenten)
    - `controllers/` – Steuerungsklassen
  - `src/` – Quellcode
    - `models/` – Implementierung der Datenmodelle
    - `views/` – Implementierung der Ansichten
    - `controllers/` – Implementierung der Steuerung

## 4. Benutzeroberfläche (Skizze geplant)
- Startansicht mit einer Übersicht über die aktuellsten Einnahmen und Ausgaben.
- Navigationselemente für:
  - Verwaltung von Immobilien.
  - Import von PDF- und Excel-/CSV-Dateien (inklusive Format-Informationen für den Nutzer).
  - Verwaltung von Kategorien.
  - Erstellung von Abrechnungen (für einzelne Immobilien oder alle zusammen).
  - Analyse-Tools zur Visualisierung der Daten.
- Filter- und Suchfunktionen erleichtern das Auffinden von Datensätzen.
- Export- und Backup-Schaltflächen befinden sich im jeweiligen Bereich oben rechts.

## 5. Abgrenzung
- Kein Online-Zugriff oder Cloud-Integration in der ersten Version.
- Keine automatische Steuerberechnung oder ELSTER-Anbindung.
- Keine automatische Verknüpfung mit Online-Bankkonten.
- Keine Nutzeranmeldung oder Authentifizierung erforderlich.

## 6. Weiteres
- Das Projekt ist als freie Software (FOSS) konzipiert.
- Eine Lizenzierung unter MIT oder GPLv3 ist vorgesehen.

Letzte Aktualisierung: 2025-04-17
