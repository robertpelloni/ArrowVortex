# ArrowVortex Lua API Reference

ArrowVortex exposes a global `Vortex` table to Lua scripts, allowing access to simfile metadata, chart data, and editor functions.

## General Functions

### `Vortex.log(message)`
Prints a message to the editor's HUD (Heads-Up Display).
*   **message**: The string to print.

## Metadata Access

### `Vortex.getSongTitle()`
Returns the title of the current simfile.
*   **Returns**: String (Title)

### `Vortex.getSongArtist()`
Returns the artist of the current simfile.
*   **Returns**: String (Artist)

### `Vortex.getSongDir()`
Returns the directory path of the current simfile.
*   **Returns**: String (Path)

### `Vortex.getChartDifficulty()`
Returns the difficulty name of the active chart (e.g., "Challenge", "Hard").
*   **Returns**: String (Difficulty)

### `Vortex.getChartMeter()`
Returns the difficulty meter (level) of the active chart.
*   **Returns**: Integer (Meter)

## Note Data Access

### `Vortex.getNotes()`
Returns all notes in the active chart as an array of tables.
*   **Returns**: Array of Note Objects.
*   **Note Object Structure**:
    *   `row`: Integer (Row index, 192nd notes)
    *   `col`: Integer (Column index, 0-based)
    *   `type`: Integer (Note Type: 0=Tap, 1=Mine, 2=Roll, 3=Lift, 4=Fake)
    *   `endrow`: Integer (End row for holds/rolls)
    *   `player`: Integer (Player index)

### `Vortex.addNote(row, col, type, [endrow], [player])`
Adds a note to the chart.
*   **row**: Integer (Row index)
*   **col**: Integer (Column index)
*   **type**: Integer (Note Type)
*   **endrow**: (Optional) Integer (End row for holds/rolls, defaults to `row`)
*   **player**: (Optional) Integer (Player index, defaults to 0)

### `Vortex.deleteNote(row, col)`
Deletes the note at the specified position.
*   **row**: Integer (Row index)
*   **col**: Integer (Column index)

### `Vortex.clearChart()`
Removes ALL notes from the active chart. Use with caution.

## Selection Access

### `Vortex.getSelection()`
Returns the current selection range.
*   **Returns**: Table `{ beginRow, endRow }`

### `Vortex.setSelection(startRow, endRow)`
Sets the editor's selection range.
*   **startRow**: Integer
*   **endRow**: Integer

### `Vortex.getSelectedNotes()`
Returns an array of notes that are currently selected.
*   **Returns**: Array of Note Objects (same structure as `getNotes`).

## Tempo & Timing

### `Vortex.getBpmAt(row)`
Returns the BPM active at the specified row.
*   **row**: Integer
*   **Returns**: Number (BPM)

### `Vortex.addBpm(row, bpm)`
Adds a BPM change at the specified row.
*   **row**: Integer
*   **bpm**: Number

### `Vortex.addStop(row, seconds)`
Adds a Stop segment at the specified row.
*   **row**: Integer
*   **seconds**: Number (Duration)

### `Vortex.getOffset()`
Returns the global song offset.
*   **Returns**: Number (Seconds)

### `Vortex.setOffset(seconds)`
Sets the global song offset.
*   **seconds**: Number

### `Vortex.rowToTime(row)`
Converts a row index to time in seconds.
*   **row**: Number
*   **Returns**: Number (Time)

### `Vortex.timeToRow(time)`
Converts time in seconds to a row index.
*   **time**: Number
*   **Returns**: Number (Row)

## Example Script

```lua
-- Quantize Selection Example
local sel = Vortex.getSelection()
local notes = Vortex.getSelectedNotes()

Vortex.log("Quantizing " .. #notes .. " notes...")

for i, note in ipairs(notes) do
    -- Snap to nearest 4th note (48 rows)
    local snap = 48
    local remainder = note.row % snap
    local newRow = note.row
    
    if remainder < snap / 2 then
        newRow = note.row - remainder
    else
        newRow = note.row + (snap - remainder)
    end
    
    if newRow ~= note.row then
        Vortex.deleteNote(note.row, note.col)
        Vortex.addNote(newRow, note.col, note.type, newRow + (note.endrow - note.row), note.player)
    end
end

Vortex.log("Done.")
```
