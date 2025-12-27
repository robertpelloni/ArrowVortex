print("Hello from Lua!")

if Vortex then
    Vortex.log("Vortex API is available.")
    
    local title = Vortex.getSongTitle()
    local artist = Vortex.getSongArtist()
    local diff = Vortex.getChartDifficulty()
    local meter = Vortex.getChartMeter()
    
    Vortex.log("Song: " .. title .. " - " .. artist)
    Vortex.log("Chart: " .. diff .. " " .. meter)
else
    print("Vortex API is NOT available.")
end