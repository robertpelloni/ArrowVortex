print("Hello from Lua!")

if Vortex then
    Vortex.log("Vortex API is available.")
    
    local title = Vortex.getSongTitle()
    local artist = Vortex.getSongArtist()
    local diff = Vortex.getChartDifficulty()
    local meter = Vortex.getChartMeter()
    
    Vortex.log("Song: " .. title .. " - " .. artist)
    Vortex.log("Chart: " .. diff .. " " .. meter)

    -- Test Note Access
    local notes = Vortex.getNotes()
    if notes then
        Vortex.log("Total Notes: " .. #notes)
        if #notes > 0 then
            local firstNote = notes[1]
            Vortex.log("First Note - Row: " .. firstNote.row .. ", Col: " .. firstNote.col .. ", Type: " .. firstNote.type)
            
            -- Test Timing Conversion
            local time = Vortex.rowToTime(firstNote.row)
            Vortex.log("Time at First Note: " .. time)
            
            local rowBack = Vortex.timeToRow(time)
            Vortex.log("Row converted back: " .. rowBack)
        end

        -- Test Write Access
        Vortex.log("Testing Write Access...")
        local testRow = 192
        local testCol = 0
        
        -- Add Note
        Vortex.addNote(testRow, testCol, 0) -- Row 192, Col 0, Type 0 (Tap)
        Vortex.log("Added note at row " .. testRow)
        
        -- Verify Addition
        local notesAfterAdd = Vortex.getNotes()
        Vortex.log("Notes after add: " .. #notesAfterAdd)
        
        -- Delete Note
        Vortex.deleteNote(testRow, testCol)
        Vortex.log("Deleted note at row " .. testRow)
        
        -- Verify Deletion
        local notesAfterDelete = Vortex.getNotes()
        Vortex.log("Notes after delete: " .. #notesAfterDelete)

        -- Test Selection
        Vortex.log("Testing Selection...")
        Vortex.setSelection(0, 192) -- Select first measure
        local sel = Vortex.getSelection()
        Vortex.log("Selection: " .. sel.beginRow .. " to " .. sel.endRow)
        
        -- Add a note in selection to test getSelectedNotes
        Vortex.addNote(0, 0, 0)
        Vortex.setSelection(0, 192) -- Re-select to ensure update (though not strictly needed if logic is right)
        local selNotes = Vortex.getSelectedNotes()
        Vortex.log("Selected Notes: " .. #selNotes)
        
        -- Cleanup
        Vortex.deleteNote(0, 0)

        -- Test Tempo
        Vortex.log("Testing Tempo...")
        local bpm = Vortex.getBpmAt(0)
        Vortex.log("BPM at row 0: " .. bpm)
        
        local offset = Vortex.getOffset()
        Vortex.log("Offset: " .. offset)
        
        -- Add BPM change
        Vortex.addBpm(192, 200)
        Vortex.log("Added BPM 200 at row 192")
        Vortex.log("BPM at row 192: " .. Vortex.getBpmAt(192))
        
        -- Add Stop
        Vortex.addStop(384, 1.0)
        Vortex.log("Added 1s Stop at row 384")

    else
        Vortex.log("No notes found or chart not loaded.")
    end
else
    print("Vortex API is NOT available.")
end