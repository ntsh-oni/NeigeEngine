vec2 = {};
setmetatable(vec2, {
	__index = vec2;
	__add = function(a, b) return vec2:new(a.x + b.x, a.y + b.y); end;
	__sub = function(a, b) return vec2:new(a.x - b.x, a.y - b.y); end;
	__tostring = function(a) return "("..a.x..", "..a.y..")" end
});

function vec2:new(x, y)
	return setmetatable({x=x or 0, y=y or 0}, getmetatable(self));
end

function vec2:length(a)
	return math.sqrt((a.x * a.x) + (a.y * a.y));
end

function vec2:dot(a, b)
	return a.x * b.x + a.y * b.y; 
end

function vec2:normalize(a)
	local length = vec2:length(a);
	return vec2:new(a.x / length, a.y / length);
end