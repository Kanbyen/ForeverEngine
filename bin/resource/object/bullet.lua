function energyBulletHit(object, hitObject)
	local pos = Vector3(0)
	getPosition(object, pos)
	-- go back a bit
	local dir = Vector3(0)
	getAim(object, dir)
	pos = pos - dir * 1.5
	createObject("orange streak small", pos, object.id)
	
	if hitObject ~= nil then
		takeDamage(hitObject, 3, 3, object.creatorID)
	end
end

function rocketHit(object, hitObject)
	local pos = Vector3(0)
	getPosition(object, pos)
	-- go back a bit
	local dir = Vector3(0)
	getAim(object, dir)
	pos = pos - dir * 1.5
	
	createExplosionMedium(pos, object.id)
	--createObject("orange streak small", pos, object.id)
	
	--if hitObject ~= nil then
	--	takeDamage(hitObject, 3, 3, object.creatorID)
	--end
end

function waterDropHit(object, hitObject)
	local pos = Vector3(0)
	getPosition(object, pos)
	-- go back a bit
	local dir = Vector3(0)
	getAim(object, dir)
	pos = pos - dir * 0.1
	createObject("water splash small", pos, object.id)
end

function waterDropGenerate(object)
	local pos = Vector3(0)
	getPosition(object, pos)
	
	local drop = createObject("water drop", pos, object.id)
	setAim(drop, Vector3(0, -1, 0))
end
