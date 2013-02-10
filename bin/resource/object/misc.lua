function explosiveBarrel1DamageTaken(object, oldHealth, newHealth)
	-- TODO: start burning if health <= 5

	if newHealth <= 0 then
		local pos = Vector3(0)
		getPosition(object, pos)
	
		createExplosionMedium(pos, object.id)
	end
end

function selfDestructSlow(object)
	takeDamage(object, 4, 0, -1)
end
