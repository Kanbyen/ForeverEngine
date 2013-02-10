function initMode()
	setSavePath("dm_map/")

	enablePaging(false)
	--setVolumeOpen(false) -- not implemented yet, but set by default (?)
	
	-- We don't set a block load func, all blocks which are not included in the loaded data will be empty
	--loadVolume()
	
	setBlockFunc("InvCube")
	setTextureFunc("AutoTex")
	setAutotexSettings(2, 3, 1, 3)
	
	-- Set camera position
	local volumeSize = Vector3(0)
	volumeSize = getVolumeSize()
	moveCamera(3.14159 * 3.0 / 2.0, 0, Vector3(0.5 * volumeSize.x, volumeSize.y - 32, -0.5 * volumeSize.z))
	
	--preloadGeometry(5*5*5)
end
