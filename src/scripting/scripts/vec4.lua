vec4 = {};
setmetatable(vec4, {
	__index = vec4;
	__add = function(a, b) return vec4:new(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w); end;
	__sub = function(a, b) return vec4:new(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w); end;
	__tostring = function(a) return "("..a.x..", "..a.y..", "..a.z..", "..a.w..")"; end;
});

function vec4:new(x, y, z, w)
	return setmetatable({x=x or 0, y=y or 0, z=z or 0, w=w or 0}, getmetatable(self));
end

function vec4:length(a)
	return math.sqrt((a.x * a.x) + (a.y * a.y) + (a.z * a.z) + (a.w * a.w));
end

function vec4:dot(a, b)
	return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w; 
end

function vec4:normalize(a)
	local length = vec4:length(a);
	return vec4:new(a.x / length, a.y / length, a.z / length, a.w / length);
end