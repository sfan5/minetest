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
#include "mapblock.h"
#include "profiler.h"
#include "activeobjectmgr.h"
#include "config.h"
#if USE_SPATIAL
	#include <spatialindex/SpatialIndex.h>
#endif

/*** SpatialImpl ***/

#if USE_SPATIAL

#define CAST_OBJ(obj) static_cast<SpatialIndex::id_type>(reinterpret_cast<intptr_t>(obj))
#define CAST_ID(id) reinterpret_cast<ServerActiveObject*>(static_cast<intptr_t>(id))

class SpatialImpl : public SpatialHelper {
public:
	SpatialImpl();
	~SpatialImpl();

	virtual void add(const ServerActiveObject *obj);
	virtual void remove(const ServerActiveObject *obj);
	virtual void update(const ServerActiveObject *obj);

	virtual void getInsideRadius(const v3f &pos, float radius,
			std::vector<ServerActiveObject *> &result,
			std::function<bool(ServerActiveObject *obj)> include_obj_cb);
	virtual void getInArea(const aabb3f &box,
			std::vector<ServerActiveObject *> &result,
			std::function<bool(ServerActiveObject *obj)> include_obj_cb);

private:
	static inline SpatialIndex::Region spatial_region(v3f minedge, v3f maxedge)
	{
		return SpatialIndex::Region(
			(double[]) {minedge.X, minedge.Y, minedge.Z},
			(double[]) {maxedge.X, maxedge.Y, maxedge.Z}, 3);
	}
	static inline SpatialIndex::Point spatial_point(v3f pos)
	{
		return SpatialIndex::Point((double[]) {pos.X, pos.Y, pos.Z}, 3);
	}

	class ResultVisitor : public SpatialIndex::IVisitor {
	public:
		ResultVisitor(std::vector<ServerActiveObject *> &result,
			std::function<bool(ServerActiveObject *obj)> callback) :
			m_result(result), m_callback(callback)
		{}
		~ResultVisitor() {}

		virtual void visitNode(const SpatialIndex::INode &in) {}

		virtual void visitData(const SpatialIndex::IData &in)
		{
			SpatialIndex::id_type id = in.getIdentifier();
			ServerActiveObject *obj = CAST_ID(id);
			if (!m_callback || m_callback(obj))
				m_result.emplace_back(obj);
		}

		virtual void visitData(std::vector<const SpatialIndex::IData *> &v)
		{
			m_result.reserve(m_result.size() + v.size());
			for (const SpatialIndex::IData *in : v)
				visitData(*in);
		}

	private:
		std::vector<ServerActiveObject *> &m_result;
		std::function<bool(ServerActiveObject *obj)> m_callback;
	};

	SpatialIndex::ISpatialIndex *m_tree;
	SpatialIndex::IStorageManager *m_storagemanager;

	// ↓↓↓↓ this is bad
	std::unordered_map<const ServerActiveObject*, v3f> pos_cache;
	// ↑↑↑↑ this is bad
};

SpatialImpl::SpatialImpl()
{
	static_assert(sizeof(SpatialIndex::id_type) >= sizeof(void*), "Type size mismatch");

	m_storagemanager = SpatialIndex::StorageManager::createNewMemoryStorageManager();
	SpatialIndex::id_type id;
	m_tree = SpatialIndex::RTree::createNewRTree(
		*m_storagemanager,
		.7, // Fill factor
		100, // Index capacity
		100, // Leaf capacity
		3, // Dimension
		SpatialIndex::RTree::RV_RSTAR,
		id);
}

SpatialImpl::~SpatialImpl()
{
	delete m_tree;
}

void SpatialImpl::add(const ServerActiveObject *obj)
{
	m_tree->insertData(0, nullptr,
		spatial_point(obj->getBasePosition()), CAST_OBJ(obj));
	pos_cache[obj] = obj->getBasePosition();
}

void SpatialImpl::remove(const ServerActiveObject *obj)
{
	auto it = pos_cache.find(obj);
	if (it != pos_cache.end())
		pos_cache.erase(it);

	bool ok = m_tree->deleteData(
		spatial_point(obj->getBasePosition()), CAST_OBJ(obj));
	if (ok)
		return;
	warningstream << "SpatialImpl::remove(): Couldn't find object at expected "
		"position, THIS IS A BUG." << std::endl;
	ok = m_tree->deleteData(
		spatial_region({S16_MIN, S16_MIN, S16_MIN}, {S16_MAX, S16_MAX, S16_MAX}),
		CAST_OBJ(obj));
	assert(ok);
}

void SpatialImpl::update(const ServerActiveObject *obj)
{
	// ↓↓↓↓ this is bad
	auto it = pos_cache.find(obj);
	assert(it != pos_cache.end());
	if (it->second == obj->getBasePosition())
		return;
	// ↑↑↑↑ this is bad

	bool ok = m_tree->deleteData(
		spatial_point(it->second), CAST_OBJ(obj));
	if (!ok) {
		warningstream << "SpatialImpl::update(): Couldn't find object at "
			"expected position, THIS IS A BUG." << std::endl;
		ok = m_tree->deleteData(
			spatial_region({S16_MIN, S16_MIN, S16_MIN}, {S16_MAX, S16_MAX, S16_MAX}),
			CAST_OBJ(obj));
		assert(ok);
	}
	add(obj);

	it->second = obj->getBasePosition();
}

void SpatialImpl::getInsideRadius(const v3f &pos, float radius,
		std::vector<ServerActiveObject *> &result,
		std::function<bool(ServerActiveObject *obj)> include_obj_cb)
{
	// Emulate through area query
	float r2 = radius * radius;
	auto cb = [&] (ServerActiveObject *obj) -> bool {
		if (obj->getBasePosition().getDistanceFromSQ(pos) > r2)
			return false;
		return !include_obj_cb || include_obj_cb(obj);
	};
	v3f r(radius, radius, radius);
	getInArea(aabb3f(pos - r, pos + r), result, cb);
}

void SpatialImpl::getInArea(const aabb3f &box,
		std::vector<ServerActiveObject *> &result,
		std::function<bool(ServerActiveObject *obj)> include_obj_cb)
{
	ResultVisitor visitor(result, include_obj_cb);
	m_tree->containsWhatQuery(spatial_region(box.MinEdge, box.MaxEdge), visitor);
}
#endif

/*** ActiveObjectMgr ***/

namespace server
{

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

void ActiveObjectMgr::clear(const std::function<bool(ServerActiveObject *, u16)> &cb)
{
	std::vector<u16> objects_to_remove;
	for (auto &it : m_active_objects) {
		if (cb(it.second, it.first)) {
			// Id to be removed from m_active_objects
			objects_to_remove.push_back(it.first);
			if (helper)
				helper->remove(it.second);
		}
	}

	// Remove references from m_active_objects
	for (u16 i : objects_to_remove) {
		m_active_objects.erase(i);
	}
}

void ActiveObjectMgr::step(const std::function<void(ServerActiveObject *)> &f)
{
	g_profiler->avg("ActiveObjectMgr: SAO count [#]", m_active_objects.size());
	for (auto &ao_it : m_active_objects) {
		f(ao_it.second);
		// TODO: This isn't the only place where entities move so this is very much incomplete
		if (helper)
			helper->update(ao_it.second);
	}
}

bool ActiveObjectMgr::addObject(ServerActiveObject *obj)
{
	assert(obj); // Pre-condition
	if (obj->getId() == 0) {
		u16 new_id = getFreeId();
		if (new_id == 0) {
			errorstream << "Server::ActiveObjectMgr::addObject(): "
					<< "no free id available" << std::endl;
			if (obj->environmentDeletes())
				delete obj;
			return false;
		}
		obj->setId(new_id);
	} else {
		verbosestream << "Server::ActiveObjectMgr::addObject(): "
				<< "supplied with id " << obj->getId() << std::endl;
	}

	if (!isFreeId(obj->getId())) {
		errorstream << "Server::ActiveObjectMgr::addObject(): "
				<< "id is not free (" << obj->getId() << ")" << std::endl;
		if (obj->environmentDeletes())
			delete obj;
		return false;
	}

	if (objectpos_over_limit(obj->getBasePosition())) {
		v3f p = obj->getBasePosition();
		warningstream << "Server::ActiveObjectMgr::addObject(): "
				<< "object position (" << p.X << "," << p.Y << "," << p.Z
				<< ") outside maximum range" << std::endl;
		if (obj->environmentDeletes())
			delete obj;
		return false;
	}

	m_active_objects[obj->getId()] = obj;
	if (helper)
		helper->add(obj);

	verbosestream << "Server::ActiveObjectMgr::addObject(): "
			<< "Added id=" << obj->getId() << "; there are now "
			<< m_active_objects.size() << " active objects." << std::endl;
	return true;
}

void ActiveObjectMgr::removeObject(u16 id)
{
	verbosestream << "Server::ActiveObjectMgr::removeObject(): "
			<< "id=" << id << std::endl;
	ServerActiveObject *obj = getActiveObject(id);
	if (!obj) {
		infostream << "Server::ActiveObjectMgr::removeObject(): "
				<< "id=" << id << " not found" << std::endl;
		return;
	}

	m_active_objects.erase(id);
	if (helper)
		helper->remove(obj);

	delete obj;
}

void ActiveObjectMgr::getObjectsInsideRadius(const v3f &pos, float radius,
		std::vector<ServerActiveObject *> &result,
		std::function<bool(ServerActiveObject *obj)> include_obj_cb)
{
	if (helper)
		return helper->getInsideRadius(pos, radius, result, include_obj_cb);

	float r2 = radius * radius;
	for (auto &activeObject : m_active_objects) {
		ServerActiveObject *obj = activeObject.second;
		const v3f &objectpos = obj->getBasePosition();
		if (objectpos.getDistanceFromSQ(pos) > r2)
			continue;

		if (!include_obj_cb || include_obj_cb(obj))
			result.push_back(obj);
	}
}

void ActiveObjectMgr::getObjectsInArea(const aabb3f &box,
		std::vector<ServerActiveObject *> &result,
		std::function<bool(ServerActiveObject *obj)> include_obj_cb)
{
	if (helper)
		return helper->getInArea(box, result, include_obj_cb);

	for (auto &activeObject : m_active_objects) {
		ServerActiveObject *obj = activeObject.second;
		const v3f &objectpos = obj->getBasePosition();
		if (!box.isPointInside(objectpos))
			continue;

		if (!include_obj_cb || include_obj_cb(obj))
			result.push_back(obj);
	}
}

void ActiveObjectMgr::getAddedActiveObjectsAroundPos(const v3f &player_pos, f32 radius,
		f32 player_radius, std::set<u16> &current_objects,
		std::queue<u16> &added_objects)
{
	/*
		Go through the object list,
		- discard removed/deactivated objects,
		- discard objects that are too far away,
		- discard objects that are found in current_objects.
		- add remaining objects to added_objects
	*/
	for (auto &ao_it : m_active_objects) {
		u16 id = ao_it.first;

		// Get object
		ServerActiveObject *object = ao_it.second;
		if (!object)
			continue;

		if (object->isGone())
			continue;

		f32 distance_f = object->getBasePosition().getDistanceFrom(player_pos);
		if (object->getType() == ACTIVEOBJECT_TYPE_PLAYER) {
			// Discard if too far
			if (distance_f > player_radius && player_radius != 0)
				continue;
		} else if (distance_f > radius)
			continue;

		// Discard if already on current_objects
		auto n = current_objects.find(id);
		if (n != current_objects.end())
			continue;
		// Add to added_objects
		added_objects.push(id);
	}
}

} // namespace server
