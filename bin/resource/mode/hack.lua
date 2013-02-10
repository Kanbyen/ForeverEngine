function initMode()
	setSavePath("save_hack/")

	initLevelGenerator(1.0)
	enablePaging(true)
	moveCamera(3.14159 * 3.0 / 2.0, 0, Vector3(0, 14, -8))
	preloadGeometry(5*5*5)
	
	createPlayer()
end
