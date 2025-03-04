
-- THIS IS SOME SAMPLE CODE
-- Currently completely reworking lua to sol2 for ease in the future, none of this code will work, not now nor in the futrue because its gonna be better

--[[
local pong = Instance.new("Light")
local paddle1 = Instance.new("Object")
local paddle2 = Instance.new("Object")


local paddle1Y = 5
local pongY = 5
local pongX = 0
local pongSpeed = 0.03
local goingUp = true
local goingLeft = true

while true do wait(0.01)
    
    local currentKeys = getKeys()
    for i = 1, #currentKeys do
        if currentKeys[i] == "W" then
            paddle1Y = paddle1Y + 0.1
        elseif currentKeys[i] == "S" then
            paddle1Y = paddle1Y - 0.1
        end
    end

    if goingLeft == true then
        pongX = pongX - pongSpeed
    else
        pongX = pongX + pongSpeed
    end

    paddle1.pos = {-5, paddle1Y, 0}
    paddle2.pos = {5, pongY, 0}
    pong.pos = {pongX, pongY, 0}

    print(pongX)
    print(pong.pos[1])
end

]]--

--[[
print("test")

local vec = vec3.new()

vec.x = 2.0

print(vec.x .. " hi guys yep thats me on the big screen hello everybody my name is welcome")
]]--


print("Before wait")
--wait(3)  -- Works because C++ yields the coroutine
print("After wait")

--[[




local obj = object.new("test", getObj("cube"), objectTypes.REGULAR)
local trans = transform.new() 
trans.pos = vec3.new(0, 5, 0)
trans.rot = vec3.new(0, 0, 0)
trans.scale = vec3.new(1, 1, 1)
print("VAO: "..obj.VAO)
print("VBO: "..obj.VBO)
obj.transform = trans
--print(object)
]]--