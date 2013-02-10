function initMode()
	loadScriptsFrom("cave", "FileSystem")
	initCave()								-- this must be implemented by a script in the cave directory

	initDigiSpe("cave.xml", "", false)
	
	setFog(0, 0, 0, 0.7, 0.9, false)		-- override fog settings of initCave() to see more
	setRunSimulation(false)					-- pause the "game", better for editing objects
	setEditObjects(true)					-- enable object editing
end
