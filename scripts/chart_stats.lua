-- Chart Statistics
-- Calculates breakdown of note types and stream analysis.

local notes = Vortex.getNotes()

if not notes then
    Vortex.log("No chart loaded.")
else
    local counts = {
        taps = 0,
        mines = 0,
        holds = 0,
        rolls = 0,
        lifts = 0,
        fakes = 0
    }
    
    for i, note in ipairs(notes) do
        if note.type == 0 then counts.taps = counts.taps + 1 end
        if note.type == 1 then counts.mines = counts.mines + 1 end
        if note.type == 2 then counts.rolls = counts.rolls + 1 end -- Note: Type 2 is usually Lift/Roll depending on engine, assuming Roll here based on standard SM
        if note.type == 3 then counts.lifts = counts.lifts + 1 end
        if note.type == 4 then counts.fakes = counts.fakes + 1 end
        
        if note.endrow > note.row then
            if note.type == 0 then counts.holds = counts.holds + 1 end -- Tap with length is hold
        end
    end
    
    Vortex.log("--- Chart Statistics ---")
    Vortex.log("Total Objects: " .. #notes)
    Vortex.log("Taps: " .. counts.taps)
    Vortex.log("Holds: " .. counts.holds)
    Vortex.log("Mines: " .. counts.mines)
    Vortex.log("Rolls: " .. counts.rolls)
    
    -- Simple Stream Analysis (16th stream)
    -- Stream = 16th notes (12 rows) without breaks
    local streamCount = 0
    local maxStream = 0
    local currentStream = 0
    local lastRow = -100
    
    -- Sort notes by row (getNotes returns sorted, but good to be safe if logic changes)
    -- Assuming sorted for this script.
    
    for i, note in ipairs(notes) do
        if note.type == 0 then -- Taps only
            if note.row - lastRow <= 12 then
                currentStream = currentStream + 1
            else
                if currentStream > 4 then -- Minimum 4 notes to call it a stream
                    streamCount = streamCount + 1
                    if currentStream > maxStream then maxStream = currentStream end
                end
                currentStream = 1
            end
            lastRow = note.row
        end
    end
    
    Vortex.log("Streams (16th): " .. streamCount)
    Vortex.log("Longest Stream: " .. maxStream .. " notes")
end
