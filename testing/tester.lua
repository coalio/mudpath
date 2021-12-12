local json = require("json")
local behavior = {}

local game_data = json.parse(io.open(arg[1] or "game.json", "r"):read("*a"))