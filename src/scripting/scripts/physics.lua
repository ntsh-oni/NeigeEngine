physics = {};

function physics:raycast(origin, direction)
	return raycast(origin.x, origin.y, origin.z, direction.x, direction.y, direction.z);
end