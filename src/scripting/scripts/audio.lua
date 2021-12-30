audio = {};

function audio:load(filePath)
	return loadAudio(filePath);
end

function audio:play(sound)
	playSound(sound);
end

function audio:stop(sound)
	stopSound(sound);
end

function audio:pause(sound)
	pauseSound(sound);
end

function audio:isPlaying(sound)
	return isPlaying(sound);
end

function audio:setGain(sound, newGain)
	setGain(sound, newGain);
end

function audio:setPitch(sound, newPitch)
	setPitch(sound, newPitch);
end

function audio:getSoundCount()
	return getSoundCount();
end