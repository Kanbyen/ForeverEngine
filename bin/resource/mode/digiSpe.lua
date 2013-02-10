function initMode()
	loadScriptsFrom("cave", "FileSystem")
	initCave()									-- this must be implemented by a script in the cave directory

	initDigiSpe("cave.xml", "cave/tour", true)
	loadAll()
	--preloadGeometry(5*5*5)					-- TODO: this causes a bug with the luabind objects which are created in the loaded blocks ... ?!
end
