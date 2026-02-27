# Copilot Instructions

## Project Guidelines
- User prefers concrete, project-specific analysis rather than generic suggestions; wants me to inspect the actual repository files and give actionable findings.
- User prefers not to use Ninja and does not want CMake or vcpkg reinstalled or large deletions.
- User prefers not to extract debug logs for recurring vcpkg/qtdeclarative failures; uses admin PowerShell and subst mappings for short paths.
- User prefers receiving individual, step-by-step PowerShell commands to run when debugging build issues.
- When working with QML in the repository, always include new QML files in `ui/qml/qml.qrc` and only use valid properties (e.g., for `ColumnLayout`, use `Layout.leftMargin` instead of `leftPadding` or wrappers).

## Naming Conventions
- Avoid using the suffix "Model" in names (e.g., do not use "ImportJobListModel"); instead, use neutral names, while "Manager" is acceptable. User finds the "Model" suffix annoying and wants naming consistency across UI models.
- Use shorter, cleaner naming for UI/QML bridge classes (e.g., avoid long names like `ImportProofImageProvider`).