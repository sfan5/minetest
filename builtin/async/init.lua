
core.log("info", "Initializing Asynchronous environment")

function core.job_processor(func, serialized_param)
	local param = core.deserialize(serialized_param)
	local retval = nil

	retval = core.serialize(func(param))

	return retval or core.serialize(nil)
end

