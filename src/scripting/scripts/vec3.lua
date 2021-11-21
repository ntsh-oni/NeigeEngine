vec3 = {};
setmetatable(vec3, {
	__index = vec3;
	__add = function(a, b) return vec3:new(a.x + b.x, a.y + b.y, a.z + b.z); end;
	__sub = function(a, b) return vec3:new(a.x - b.x, a.y - b.y, a.z - b.z); end;
	__tostring = function(a) return "("..a.x..", "..a.y..", "..a.z..")"; end;
});

function vec3:new(x, y, z)
	return setmetatable({x=x or 0, y=y or 0, z=z or 0}, getmetatable(self));
end

function vec3:length(a)
	return math.sqrt((a.x * a.x) + (a.y * a.y) + (a.z * a.z));
end

function vec3:dot(a, b)
	return a.x * b.x + a.y * b.y + a.z * b.z; 
end

function vec3:cross(a, b)
	return vec3:new(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
end

function vec3:normalize(a)
	local length = vec3:length(a);
	return vec3:new(a.x / length, a.y / length, a.z / length);
end