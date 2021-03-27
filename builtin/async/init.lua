
if core.get_cache_path == nil then -- TODO

core.log("action", "Initializing Asynchronous environment for game")

function core.job_processor(func, serialized_params)
	local params = core.deserialize(serialized_params)

	print('will call ' .. tostring(func) .. ' with ' .. tostring(#params.arg) .. ' args')

	--core.set_last_run_mod(param.mod_origin) -- TODO
	local retval = {func(unpack(params.arg))}

	return core.serialize(retval)
end

else

core.log("info", "Initializing Asynchronous environment")

function core.job_processor(func, serialized_param)
	local param = core.deserialize(serialized_param)
	local retval = nil

	retval = core.serialize(func(param))

	return retval or core.serialize(nil)
end

end
