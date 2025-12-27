-- Quantize Selection to 4th notes (Red)
-- This script iterates through the selected notes and snaps them to the nearest 4th note (48 rows).

local sel = Vortex.getSelection()
local notes = Vortex.getSelectedNotes()

if #notes == 0 then
    Vortex.log("No notes selected.")
else
    Vortex.log("Quantizing " .. #notes .. " notes to 4th...")
    
    local count = 0
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
            -- Delete old note
            Vortex.deleteNote(note.row, note.col)
            -- Add new note (preserving type, endrow length, player)
            local len = note.endrow - note.row
            Vortex.addNote(newRow, note.col, note.type, newRow + len, note.player)
            count = count + 1
        end
    end
    
    Vortex.log("Quantized " .. count .. " notes.")
end
