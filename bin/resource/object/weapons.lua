function knifeLMB(object)
	-- move the knife forward and wait
	setTargetPosition(object, ComponentPosition(0.086, -0.094, -3.453,		-115.46, 8.672, -96.464))
	wait(0.15)
	
	-- calculate hit
	local start = Vector3(0)
	getHead(object, start)
	local aim = Vector3(0)
	getAim(object, aim)
	
	local hitPos = Vector3(0)
	local hit = rayTestCombined(start, aim, 4.2, hitPos, object, 23)
	if hit then
		-- which object was hit?
		local hitObject = getLastHitObject()
		if hitObject == nil then
			-- we hit the ground; dig a bit and create some smoke
			editVoxelDataSmooth2(hitPos, false, 1.4)
			createObject("smoke small", hitPos, object.id)
		else
			-- we hit an object, get its position ...
			local hitObjectCenter = Vector3(0)
			getPosition(hitObject, hitObjectCenter)
			
			-- ... apply an impulse ...
			applyImpulse(hitObject, aim * 14, hitPos - hitObjectCenter)
			damageEffect(hitObject, hitPos, 0)
			
			-- and apply damage
			local holderID = getHolderID(object)
			takeDamage(hitObject, 4.5, 0, holderID)
		end
	end
	
	-- move the knife back to the original position and wait
	setTargetPosition(object, ComponentPosition(0.584, -0.110, -1.660,		0, 42.903, 5.391))
	wait(0.25)
end

function pistolLMB(object)
	setTargetPosition(object, ComponentPosition(0.479, -0.4, -1.262,		80, 0, 0))
	
	local start = Vector3(0)
	getHead(object, start)
	local aim = Vector3(0)
	getAim(object, aim)
	
	local hitPos = Vector3(0)
	local hit = rayTestCombined(start, aim, 800, hitPos, object, 23)
	if hit then
		local hitObject = getLastHitObject()
		if hitObject == nil then
			editVoxelDataHard(hitPos, false, 0.8)
			createObject("smoke small", hitPos, object.id)
		else
			hitObjectCenter = Vector3(0)
			getPosition(hitObject, hitObjectCenter)
			
			applyImpulse(hitObject, aim * 6, hitPos - hitObjectCenter)
			damageEffect(hitObject, hitPos, 0)
			
			-- apply 2 damage types
			local holderID = getHolderID(object)
			takeDamage(hitObject, 4.5, 0, holderID)
			takeDamage(hitObject, 0.3, 1, holderID)
		end
	end
	
	local fireSplashPos = Vector3(0)
	getMuzzleRel(object, fireSplashPos)
	local fireSplash = createObject("fire splash small", fireSplashPos, object.id)
	attachObject(object, fireSplash)
	
	wait(0.02)
	
	setTargetPosition(object, ComponentPosition(0.479, -0.4, -1.406,		0, 0, 0))
	wait(0.33)
end

function plasmapistolLMB(object)
	setTargetPosition(object, ComponentPosition(0.479, -0.4, -1.262,		80, 0, 0))
	
	local start = Vector3(0)
	getHead(object, start)
	local aim = Vector3(0)
	getAim(object, aim)
	start = start + aim * 4.6
	
	local bullet = createObject("energy bullet", start, object.id)
	setAim(bullet, aim)
	
	local fireSplashPos = Vector3(0)
	getMuzzleRel(object, fireSplashPos)
	local fireSplash = createObject("fire splash small", fireSplashPos, object.id)
	attachObject(object, fireSplash)
	
	wait(0.02)
	
	setTargetPosition(object, ComponentPosition(0.479, -0.4, -1.406,		0, 0, 0))
	wait(0.1)
end

function machinegunLMB(object)
	setTargetPosition(object, ComponentPosition(0.479, -0.4, -1.262,		10, 0, 0))
	
	local start = Vector3(0)
	getHead(object, start)
	local aim = Vector3(0)
	getAim(object, aim)
	
	local hitPos = Vector3(0)
	local hit = rayTestCombined(start, aim, 800, hitPos, object, 23)
	if hit then
		local hitObject = getLastHitObject()
		if hitObject == nil then
			editVoxelDataHard(hitPos, false, 0.8)
			createObject("smoke small", hitPos, object.id)
		else
			hitObjectCenter = Vector3(0)
			getPosition(hitObject, hitObjectCenter)
			
			applyImpulse(hitObject, aim * 6, hitPos - hitObjectCenter)
			damageEffect(hitObject, hitPos, 0)
			
			-- apply 2 damage types
			local holderID = getHolderID(object)
			takeDamage(hitObject, 4.5, 0, holderID)
			takeDamage(hitObject, 0.3, 1, holderID)
		end
	end
	
	local fireSplashPos = Vector3(0)
	getMuzzleRel(object, fireSplashPos)
	local fireSplash = createObject("fire splash small", fireSplashPos, object.id)
	attachObject(object, fireSplash)
	
	wait(0.02)
	
	setTargetPosition(object, ComponentPosition(0.479, -0.4, -1.406,		0, 0, 0))
	wait(0.05)
end

function rocketlauncherLMB(object)
	setTargetPosition(object, ComponentPosition(0.699, -0.523, -1.06,		0, 0, 0))
	
	local start = Vector3(0)
	getHead(object, start)
	local aim = Vector3(0)
	getAim(object, aim)
	start = start + aim * 4.6
	
	local bullet = createObject("rocket", start, object.id)
	setAim(bullet, aim)
	
	--local fireSplashPos = Vector3(0)
	--getMuzzleRel(object, fireSplashPos)
	--local fireSplash = createObject("fire splash small", fireSplashPos, object.id)
	--attachObject(object, fireSplash)
	
	wait(0.60)
	
	setTargetPosition(object, ComponentPosition(0.699, -0.523, -1.513,		0, 0, 0))
	wait(0.40)
end
