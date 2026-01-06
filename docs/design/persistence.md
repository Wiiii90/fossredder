# Persistence — FOSSredder

Date: 2026-01-06
Author: Wilhelm Altemeier
Status: Draft

Storage strategy
The project uses SQLite as the primary on‑disk storage. Databases are opened via `createSqliteDb(dbPath)` (see `persistence/Factory.h`). The running application stores the project database file under the user's profile directory by default (main program composes a path using `$HOME/.fossredder` and the default filename `fossredder.db`).

Schemas and files
- Schema implementation and migration logic live in `persistence/src/SqliteSchema.cpp` and are applied automatically when a database is opened (`SqliteDb` constructor calls `SqliteSchema::ensure`).
- Main domain tables created by the schema include: `configs`, `actors`, `actor_aliases`, `properties`, `contracts`, `contract_actors`, `contract_properties`, `statements`, `transactions`.
- Repository factories and concrete repository implementations are in `persistence/src` and headers under `persistence/include/persistence/repositories/`. Use `persistence/Factory.h` to create repository instances.

Migration policy
- Schema versioning is implemented via SQLite `PRAGMA user_version` (see `SqliteSchema::getUserVersion` / `setUserVersion`).
- `SqliteSchema::migrate` contains forward migrations and is executed on database open; implement new migrations by extending this function and bumping `user_version`.
- Migration policy: automatic, in‑place migrations at open time. Back up the database file before running major migrations when upgrading production data.

Backup and restore
- Application state is exported/imported via the `AppStateStore` abstraction (`persistence/AppStateStore.h` / `AppStateStore.cpp`).
- `AppStateStore::save` writes the application state inside a SQLite transaction and returns an `AppStateStoreResult` describing impact (deleted ids etc.).
- The default backup artifact is the SQLite database file. The UI and `FileController` currently manage file-level operations (open, new, save); the code sets atomic load/save callbacks that use `AppStateStore` to read/write the DB contents.

Operational notes
- Foreign keys are enabled (`PRAGMA foreign_keys = ON`) in schema initialization.
- Migrations and schema creation are executed automatically on DB open; errors during schema execution throw exceptions.
- Repository implementations (actors, properties, statements, transactions, contracts, configs) provide CRUD operations; prefer using repository interfaces from `persistence/include/persistence/repositories/`.
- `AppStateStore::save` uses a transaction (`SqliteTransaction`) and performs cleanup of records not present in the saved state (see deletion helpers in `AppStateStore.cpp`).
- For integrity and recovery: consider copying the DB file to a safe location before performing upgrades or bulk imports.

References
- `persistence/src/SqliteSchema.cpp` (schema + migrations)
- `persistence/src/SqliteDb.cpp` (DB open/close)
- `persistence/include/persistence/Factory.h` (factory functions)
- `persistence/src/AppStateStore.cpp` and `persistence/include/persistence/AppStateStore.h` (backup/restore and state save/load)

Notes
- This document records the current, code‑observed persistence design. If storage requirements change (e.g. add optional SQLite + JSON hybrid or export formats), update this file and add concrete JSON schema / SQL DDL under `docs/design/appendix/schemas/`.