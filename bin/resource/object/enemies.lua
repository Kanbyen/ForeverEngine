function batBite(object, hitObject)
	if hitObject ~= nil then
		-- TODO: check if hitObject is a living being and hostile
		takeDamage(hitObject, 2, 0, object.id)
		wait(0.1)	-- don't kill the victim immediately with repeated collisions!
	end
end

function createBlueRingEffect(object)
	local pos = Vector3(0)
	getPosition(object, pos)

	createObject("ring emit blue", pos, object.id)
end
