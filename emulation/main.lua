-- You need Love2D installed to run this script
-- After installing it, run "love ." in the terminal to start the emulation
-- If you dont have a match_data file, generate one using emulate.cpp

local json = require "json"
local match_data = {}
local match_data_file = arg[2] or "game.json"

local bot = {}

function love.load()
    love.window.setMode(800, 600, {resizable = false})
    love.graphics.setBackgroundColor(0, 0, 0)
    love.graphics.setColor(255, 255, 255)
    love.graphics.setFont(love.graphics.newFont(12))

    -- Load match data
    local file = io.open(match_data_file, "r")
    if file then
        match_data = json.parse(file:read("*all"))
        file:close()
    end

    print("Loaded match data:")
    for k, v in pairs(match_data) do
        print(k, v)
    end

    -- Initialize bot at position of match_data.pods[1]
    bot.x = match_data.pods[1].x
    bot.y = match_data.pods[1].y
    bot.sqr_area = match_data.player.sqr_area

    --[[
        No need to add velocity because it was already calculated in the emulator
    ]]
end

local curr_point = 1
local FPS = 40
local nextTime = love.timer.getTime() + 1/FPS

function love.update(dt)
    -- Do this at start
    local timedif = nextTime - love.timer.getTime()
    if timedif > 0 then love.timer.sleep(timedif) end
    nextTime = nextTime + 1/FPS

    -- Update bot position
    print(curr_point, #match_data.points)
    if (not match_data.points[curr_point]) then
        print("Finished")
        return
    end

    bot.x = bot.x + match_data.points[curr_point].x
    bot.y = bot.y + match_data.points[curr_point].y

    -- Increase current point
    curr_point = curr_point + 1
end

function love.draw()
    -- Draw white circles for each pod
    love.graphics.setColor(255, 255, 255)
    for i = 1, #match_data.pods do
        local pod = match_data.pods[i]
        love.graphics.circle("fill", pod.x, pod.y, pod.radius)
    end

    -- Set color to yellow
    love.graphics.setColor(255, 255, 0)
    love.graphics.circle("fill", bot.x, bot.y, bot.sqr_area)
end