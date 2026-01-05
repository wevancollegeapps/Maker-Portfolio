wait()
repeat wait() until game.Loaded or game:IsLoaded()
local event = game:GetService("ReplicatedStorage").RayEvent
local scripts = -1
for _,v in pairs(script.Parent:GetChildren()) do
	if v:IsA("LocalScript") then
		scripts = scripts + 1
	end
end
local maxlidarparticles = game.Workspace:WaitForChild(game.Players.LocalPlayer.Name .. " Settings"):WaitForChild("Maximum Lidar Particles").Value / scripts
local SaveMult = 1
local maxlidarscans = math.ceil(game.Workspace:WaitForChild(game.Players.LocalPlayer.Name .. " Settings"):WaitForChild("Maximum Lidar Scan Saves").Value * SaveMult)
local psize = game.Workspace:WaitForChild(game.Players.LocalPlayer.Name .. " Settings"):WaitForChild("Particle Size").Value
local spread = game.Workspace:WaitForChild(game.Players.LocalPlayer.Name .. " Settings"):WaitForChild("Particle Spread").Value
local YSize = game.Workspace:WaitForChild(game.Players.LocalPlayer.Name .. " Settings"):WaitForChild("ScanCast Veiw Size X").Value
local XSize = game.Workspace:WaitForChild(game.Players.LocalPlayer.Name .. " Settings"):WaitForChild("ScanCast Veiw Size Y").Value
local max = math.floor(math.sqrt(maxlidarparticles / game.Workspace:WaitForChild(game.Players.LocalPlayer.Name .. " Settings"):WaitForChild("Maximum Lidar Scan Saves").Value))
warn(max)
warn(maxlidarparticles)
warn(scripts)
local ignorelist = {
	game.Players.LocalPlayer.Character:WaitForChild("Head"),
	game.Players.LocalPlayer.Character:WaitForChild("HumanoidRootPart"),
	game.Players.LocalPlayer.Character:WaitForChild("Left Arm"),
	game.Players.LocalPlayer.Character:WaitForChild("Left Leg"),
	game.Players.LocalPlayer.Character:WaitForChild("Right Arm"),
	game.Players.LocalPlayer.Character:WaitForChild("Right Leg"),
	game.Players.LocalPlayer.Character:WaitForChild("Torso"),
	workspace:FindFirstChild("AttachmentPart0"),
	workspace:FindFirstChild("AttachmentPart1")
}
local blacklist = {
	"hitpart",
	"Baseplate",
	"beamattachment0",
	"beamattachment1",
	"AttachmentPart0",
	"AttachmentPart1"
}
local blacklistfunction = function(hit)
	for i = 1, #blacklist do
		if blacklist[i] == hit.Name then
			--warn("blocked")
			return false
		end
	end
	return true
end

for o=1, maxlidarscans do
	if workspace:FindFirstChild("hitpartfolder " .. script.Name .. " " .. tostring(o)) == nil then
		local folder = Instance.new("Folder")
		folder.Parent = workspace
		folder.Name = "hitpartfolder " .. script.Name .. " " .. tostring(o)
		for i=1, max*max, 1 do
			if math.random(1,1000) == 500 then
				wait()
			end
			local part = Instance.new("Part")
			part.Position = Vector3.new(0,1000,0)
			part.Material = Enum.Material.Neon
			part.Size = Vector3.new(psize/1000, psize/1000, psize/1000)
			part.Anchored = true
			part.CanCollide = false
			part.Parent = folder
			part.Name = "hitpart"
			part.Shape = Enum.PartType.Ball
			part.CanQuery = false
			part.CanTouch = false
			part.CastShadow = false
		end
	else
		local children = workspace:FindFirstChild("hitpartfolder " .. script.Name .. " " .. tostring(o)):GetChildren()
		if #children > max*max then
			warn("too many parts")
			for i=1, (#children - max*max), 1 do
				children[i]:Destroy()
			end
		elseif #children < max*max then
			warn("too little parts")
			for i=1, max*max, 1 do
				if math.random(1,1000) == 500 then
					wait()
				end
				local part = Instance.new("Part")
				part.Position = Vector3.new(0,1000,0)
				part.Material = Enum.Material.Neon
				part.Size = Vector3.new(psize/1000, psize/1000, psize/1000)
				part.Anchored = true
				part.CanCollide = false
				part.Parent = workspace:FindFirstChild("hitpartfolder " .. script.Name .. " " .. tostring(o))
				part.Name = "hitpart"
				part.Shape = Enum.PartType.Ball
				part.CanQuery = false
				part.CanTouch = false
				part.CastShadow = false
			end
		end
	end
end

local folderon = script.foldernumber.Value

local function parttween(greaterthan1)
	local moveparts = workspace:FindFirstChild("hitpartfolder " .. script.Name .. " " .. tostring(folderon)):GetChildren()
	for i=1, #moveparts, 1 do
		if moveparts[i]:IsA("Part") then
			if greaterthan1 == true then
				moveparts[i].Position = Vector3.new(0,1000,0)
			end
		end
	end
end

local counted = 0
local debounce = false
local function scan()
	if debounce == false then
		debounce = true
		if folderon > 1 then
			parttween(true)
		else
			parttween(false)
		end
		
		for x=1, max, 1 do
			for y=1, max, 1 do
				if y == math.floor(max/2) and x%2==0 then
					wait()
				end
				
				local rayOrigin = workspace.CurrentCamera.CFrame * CFrame.new(math.random(-100,100)/spread,math.random(-100,100)/spread,0)
				local rayDirection = workspace.CurrentCamera.CFrame * CFrame.fromEulerAngles(math.rad(x/(max)*(-1*(YSize)))+math.rad(YSize/2), math.rad(y/(max)*(-1*(XSize)))+math.rad(XSize/2), 0)
				local ray = Ray.new(rayOrigin.Position, rayDirection.LookVector * game.ReplicatedStorage.Range.Value)

				local hit, position = game.Workspace:FindPartOnRayWithIgnoreList(ray, ignorelist)
				if hit then
					if blacklistfunction(hit) == true then
						event:FireServer(position, 	game.Players.LocalPlayer.Character:WaitForChild("Head"), hit.Color.R, hit.Color.G, hit.Color.B, script.Name, false)
						--
						local part = workspace:FindFirstChild("hitpartfolder " .. script.Name .. " " .. tostring(folderon)):FindFirstChild("hitpart")
						if part.Position ~= nil or part.Color ~= nil then
							part.Position = position
							part.Color = Color3.new(tonumber(hit.Color.R)/2, tonumber(hit.Color.G)/2, tonumber(hit.Color.B)/2)
						end
						part.Name = "hitpartused"
						part.Size = Vector3.new(psize/1000, psize/1000, psize/1000)
						if hit.Parent:FindFirstChild("Humanoid") ~= nil then
							if hit.Parent:FindFirstChild("BillboardGui") == nil then
								if (game.Players.LocalPlayer.Character:WaitForChild("Torso").Position - part.Position).Magnitude <= 30 then
									local players = game.Players:GetChildren()
									local found = false
									for p = 1, #players do
										if players[p].Name == hit.Parent.Name then
											game.ReplicatedStorage.FindPlayerEvent:FireServer(hit.Parent.Name, hit.Parent:FindFirstChild("Torso").Color)
											print("fired server")
											local BillboardGui = Instance.new("BillboardGui")
											local TextLabel = Instance.new("TextLabel")
											BillboardGui.Size = UDim2.new(2,0,2,0)
											TextLabel.Size = UDim2.new(1,0,1,0)
											BillboardGui.Parent = hit.Parent
											TextLabel.Parent = BillboardGui
											TextLabel.Text = "▼"
											TextLabel.BackgroundTransparency = 1
											TextLabel.TextScaled = true
											TextLabel.TextColor3 = hit.Parent:FindFirstChild("Torso").Color
											script["SCP:SL Looked at SCP-096 - SCP"]:Play()
											found = true
										end
									end
									if found == false then
										local BillboardGui = Instance.new("BillboardGui")
										local TextLabel = Instance.new("TextLabel")
										BillboardGui.Size = UDim2.new(2,0,2,0)
										TextLabel.Size = UDim2.new(1,0,1,0)
										BillboardGui.Parent = hit.Parent
										TextLabel.Parent = BillboardGui
										TextLabel.Text = "◈"
										TextLabel.BackgroundTransparency = 1
										TextLabel.TextScaled = true
										TextLabel.TextColor3 = hit.Parent:FindFirstChild("Torso").Color
										script["SCP:SL Looked at SCP-096 - SCP"]:Play()
									end
								end
							end
						end
						counted = counted + 1
					end
				end
			end
		end
		event:FireServer(nil, game.Players.LocalPlayer.Character:WaitForChild("Head"), nil, nil, nil, script.Name, true)
		for i=1, counted, 1 do
			if i%max==0 then
				wait()
			end
			workspace:FindFirstChild("hitpartfolder " .. script.Name .. " " .. tostring(folderon)):FindFirstChild("hitpartused").Name = "hitpart"
		end
		print(folderon)
		counted = 0
		folderon = folderon + 1
		if folderon == maxlidarscans then
			folderon = 1
		end
		script.foldernumber.Value = folderon
		debounce = false
	end	
end

local UserInputService = game:GetService("UserInputService")
local mobileevent = game.ReplicatedStorage.MobileScanEvent
if UserInputService.TouchEnabled and not UserInputService.KeyboardEnabled and not UserInputService.MouseEnabled then
	--mobile
	mobileevent.OnClientEvent:Connect(function(state)
		if state == "Idle" then
			
		end
		if state == "Press" then
			scan()
		end
	end)
elseif not UserInputService.TouchEnabled and UserInputService.KeyboardEnabled and UserInputService.MouseEnabled then
	--computer
	game.Players.LocalPlayer:GetMouse().Button1Down:Connect(function()
		scan()
	end)
end
