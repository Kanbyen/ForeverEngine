function explodeGrenade(object, hitObject)
	local pos = Vector3(0)
	getPosition(object, pos)
	
	createExplosionMedium(pos, object.id)
end

function explodeGeometryGrenade(object, hitObject)
	local pos = Vector3(0)
	getPosition(object, pos)
	
	createObject("orange streak explosion", pos, object.id)
	wait(0.02)
	editVoxelDataHard(pos, true, 1)
	wait(0.04)
	editVoxelDataHard(pos, true, 3)
	wait(0.04)
	editVoxelDataHard(pos, true, 5)
	wait(0.04)
	editVoxelDataHard(pos, true, 7)
	wait(0.04)
	editVoxelDataHard(pos, true, 9)
	createObject("smoke ring big", pos, object.id)
	wait(0.05)
end
