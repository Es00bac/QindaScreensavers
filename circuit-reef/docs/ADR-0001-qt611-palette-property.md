# ADR-0001: distinguish reef colors from Qt Quick palette

- Status: Accepted
- Date: 2026-09-29
- Scope: Circuit Reef public Qt/QML embedding API

Qt 6.11 supplies a base Item palette property. CircuitReef's existing QString
palette property overrides it with an incompatible type and produces a QML
property-cache warning. Fatal-warning native locker qualification rejects that
module. The compositor lock and PAM boundaries cannot hide this dependency error.

The scene now exposes reefPalette, setReefPalette and reefPaletteChanged.
The CLI --palette option and renderer palette value are unchanged.
QML consumers must migrate palette: "lagoon" to reefPalette: "lagoon"; retaining
a QString alias under palette would reproduce the incompatible override.
The inherited Qt Quick palette keeps its own type and meaning. This deliberate
embedding API migration is required before the corrected module is packaged.

A focused fatal-warning QML component regression constructs the real item,
checks both property types and observes native change notifications. Native
greeter qualification uses the private built module; no live installation is
performed. The overlay's released source pin remains unchanged until the
delivery manager creates a coherent replacement recipe.
