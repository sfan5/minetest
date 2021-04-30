/*
Minetest
Copyright (C) 2010-2018 nerzhul, Loic BLOT <loic.blot@unix-experience.fr>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#undef NDEBUG

#include "log.h"
#include "profiler.h"
#include "activeobjectmgr.h"
#include "config.h"
#if USE_SPATIAL
	#include <spatialindex/SpatialIndex.h>
#endif

namespace client
{

/*** SpatialImpl ***/

#if USE_SPATIAL

#define AO_TYPE ClientActiveObject
#define getBasePosition getPosition // hmm yes
#include "../spatialimpl.h"

#endif

/*** ActiveObjectMgr ***/

ActiveObjectMgr::ActiveObjectMgr()
{
#if USE_SPATIAL
	verbosestream << "ActiveObjectMgr: using SpatialIndex to speed up queries" << std::endl;
	helper = new SpatialImpl();
#endif
}

ActiveObjectMgr::~ActiveObjectMgr()
{
	delete helper;
}

void ActiveObjectMgr::clear()
{
	// delete active objects
	for (auto &active_object : m_active_objects) {
		delete active_object.second;
		// Object must be marked as gone when children try to detach
		active_object.second = nullptr;
	}
	m_active_objects.clear();

#if USE_SPATIAL
	if (helper) {
		delete helper;
		helper = new SpatialImpl();
	}
#endif
}

void ActiveObjectMgr::step(const std::function<void(ClientActiveObject *)> &f)
{
	g_profiler->avg("ActiveObjectMgr: CAO count [#]", m_active_objects.size());
	for (auto &ao_it : m_active_objects) {
		f(ao_it.second);
		if (helper) // TODO: probably incomplete here too
			helper->update(ao_it.second);
	}
}

bool ActiveObjectMgr::addObject(ClientActiveObject *obj)
{
	assert(obj); // Pre-condition
	if (obj->getId() == 0) {
		u16 new_id = getFreeId();
		if (new_id == 0) {
			infostream << "Client::ActiveObjectMgr::addObject(): "
					<< "no free id available" << std::endl;

			delete obj;
			return false;
		}
		obj->setId(new_id);
	}

	if (!isFreeId(obj->getId())) {
		infostream << "Client::ActiveObjectMgr::addObject(): "
				<< "id is not free (" << obj->getId() << ")" << std::endl;
		delete obj;
		return false;
	}
	infostream << "Client::ActiveObjectMgr::addObject(): "
			<< "added (id=" << obj->getId() << ")" << std::endl;

	m_active_objects[obj->getId()] = obj;
	if (helper)
		helper->add(obj);

	return true;
}

void ActiveObjectMgr::removeObject(u16 id)
{
	verbosestream << "Client::ActiveObjectMgr::removeObject(): "
			<< "id=" << id << std::endl;
	ClientActiveObject *obj = getActiveObject(id);
	if (!obj) {
		infostream << "Client::ActiveObjectMgr::removeObject(): "
				<< "id=" << id << " not found" << std::endl;
		return;
	}

	m_active_objects.erase(id);
	if (helper)
		helper->remove(obj);

	obj->removeFromScene(true);
	delete obj;
}

void ActiveObjectMgr::getActiveObjects(const v3f &origin, f32 max_d,
		std::vector<ClientActiveObject *> &dest)
{
	if (helper) {
		const auto cb = [] (ClientActiveObject *obj) -> bool {
			return true;
		};
		return helper->getInsideRadius(origin, max_d, dest, cb);
	}

	f32 max_d2 = max_d * max_d;
	for (auto &ao_it : m_active_objects) {
		ClientActiveObject *obj = ao_it.second;

		f32 d2 = (obj->getPosition() - origin).getLengthSQ();

		if (d2 > max_d2)
			continue;

		dest.emplace_back(obj);
	}
}

} // namespace client
