function explosionMediumObjectCallback(center, squaredDistance, object, objectPos)
	local impulse = (objectPos - center)
	impulse:normalise()
	impulse = impulse * 2 * (150 - squaredDistance)
	applyCentralImpulse(object, impulse)
	if takeDamage(object, 20 - (squaredDistance * 0.2), 0, -1) then
		takeDamage(object, 20 - (squaredDistance * 0.2), 1, -1)
	end
end
function createExplosionMedium(pos, creatorID)
	createObject("smoke big", pos, creatorID)
	createObject("orange streak explosion", pos, creatorID)
	createObject("explosion fire ball", pos, creatorID)
	createObject("explosion light flash", pos, creatorID)
	wait(0.05)
	editVoxelDataHard(pos, false, 8)
	objectsInSphereCallback(pos, 10, "explosionMediumObjectCallback")
end

function explodeOnDeath(object, oldHealth, newHealth)
	if newHealth <= 0 then
		local pos = Vector3(0)
		getPosition(object, pos)
	
		createExplosionMedium(pos, object.id)
	end
end

function ShortWaitWhenDamageTaken(object, oldHealth, newHealth)
	wait(0.05)
end
