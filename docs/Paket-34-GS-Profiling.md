# Paket 34 – GS-Profiling auf der PS4 Slim

## Zweck

Die GS-Profilierung sammelt Messwerte für die Suche nach CPU-seitigen Engpässen im PS2-Grafikpfad. Sie ist **standardmäßig ausgeschaltet**. Paket 33 bleibt unverändert; diese Anleitung gilt für den Entwicklungszweig `package-34-performance-fix`.

## Profilierung einschalten

1. PS4SX2 vollständig beenden.
2. Auf der PS4 im Dateimanager den Ordner `/data/PS4SX2/` öffnen.
3. Dort eine **leere Datei ohne Dateiendung** mit exakt diesem Namen anlegen: `vm-profile-gs`.
4. PS4SX2 starten und Budokai Tenkaichi 3 laden.
5. Eine reproduzierbare Spielsituation für etwa 2–3 Minuten laufen lassen. Möglichst dieselbe Szene verwenden und keine Menüs/Unterbrechungen einbeziehen.
6. PS4SX2 sauber beenden und anschließend die Protokolle aus `/data/PS4SX2/logs/` sichern.

Die Datei ist ein Marker, kein Konfigurationsdokument: Sie muss keinen Inhalt haben.

## Profilierung ausschalten

PS4SX2 beenden und die Datei `/data/PS4SX2/vm-profile-gs` wieder löschen. Alternativ verhindert `/data/PS4SX2/vm-no-gs-profile` die Profilierung ausdrücklich. **Achtung:** Der Marker `/data/PS4SX2/vm-verbose-log` erzwingt GS-Profiling ebenfalls; für einen normalen Leistungstest darf er nicht vorhanden sein.

## Welche Werte wichtig sind

Im Log erscheinen unter anderem:

- `GS timing` für `primitive_coverage`, `texture_minmax`, `texture_lookup`, `hw_draw`, `vk_render` und weitere Bereiche.
- `sampled_wall_ms`: aufsummierte gemessene Wandzeit der Stichproben im Intervall.
- `samples`: Anzahl tatsächlich zeitlich gemessener Stichproben.
- `calls`: Anzahl der Aufrufe, einschließlich nicht zeitlich gemessener Aufrufe.
- `sample_max_ms`: längste gemessene Einzelstichprobe.

Einige häufige GS-Arbeitsschritte werden ungefähr bei jedem 64. Aufruf zeitlich gemessen. Die Zeiten sind Stichproben und teils verschachtelt; **nicht addieren**, um eine Gesamt-CPU-Zeit zu berechnen. Ein einzelner Lauf beweist noch keinen Engpass. Für belastbare Vergleiche dieselbe Szene mehrfach messen.

## Interpretation

Die Werte dienen zunächst nur dazu, Kandidaten zu vergleichen. Erst nach einem reproduzierbaren BT3-Messlauf wird entschieden, welcher Codepfad optimiert wird. Es werden keine Grafikdetails oder Auflösung reduziert, nur um einen höheren Prozentwert zu erreichen.
