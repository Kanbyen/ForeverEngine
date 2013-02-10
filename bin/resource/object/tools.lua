function geometryGunLMB(object)
	setTargetPosition(object, ComponentPosition(0.699, -0.523, -1.46,		0, 0, 0))
	
	local muzzleFirePos = Vector3(0)
	getMuzzleRel(object, muzzleFirePos)
	local muzzleFire = createObject("glow ring red", muzzleFirePos, object.id)
	attachObject(object, muzzleFire)
	
	wait(0.07)

	local start = Vector3(0)
	getHead(object, start)
	local aim = Vector3(0)
	getAim(object, aim)
	
	local hitPos = Vector3(0)
	local hit = rayTestVoxel(start, aim, 26, hitPos)
	if hit then
		local lengthVec = hitPos - start
		if lengthVec:length() > 6 then
			editVoxelDataSmooth2(hitPos, true, 10)
		end
	end

	setTargetPosition(object, ComponentPosition(0.699, -0.523, -1.513,		0, 0, 0))
end

function geometryGunRMB(object)
	setTargetPosition(object, ComponentPosition(0.699, -0.523, -1.56,		0, 0, 0))
	
	local muzzleFirePos = Vector3(0)
	getMuzzleRel(object, muzzleFirePos)
	local muzzleFire = createObject("glow ring blue", muzzleFirePos, object.id)
	attachObject(object, muzzleFire)
	
	wait(0.07)

	local start = Vector3(0)
	getHead(object, start)
	local aim = Vector3(0)
	getAim(object, aim)
	
	local hitPos = Vector3(0)
	local hit = rayTestVoxel(start, aim, 26, hitPos)
	if hit then
		editVoxelDataSmooth2(hitPos, false, 10)
	end

	setTargetPosition(object, ComponentPosition(0.699, -0.523, -1.513,		0, 0, 0))
end
