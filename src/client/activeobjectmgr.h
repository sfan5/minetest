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

#pragma once

#include <functional>
#include <vector>
#include "../activeobjectmgr.h"
#include "clientobject.h"

namespace client
{
class SpatialHelper {
public:
	virtual ~SpatialHelper() = default;

	virtual void add(const ClientActiveObject *obj) = 0;
	virtual void remove(const ClientActiveObject *obj) = 0;
	// Called when object might have changed position
	virtual void update(const ClientActiveObject *obj) = 0;

	virtual void getInsideRadius(const v3f &pos, float radius,
			std::vector<ClientActiveObject *> &result,
			std::function<bool(ClientActiveObject *obj)> include_obj_cb) = 0;
	virtual void getInArea(const aabb3f &box,
			std::vector<ClientActiveObject *> &result,
			std::function<bool(ClientActiveObject *obj)> include_obj_cb) = 0;
};

class ActiveObjectMgr : public ::ActiveObjectMgr<ClientActiveObject>
{
public:
	ActiveObjectMgr();
	~ActiveObjectMgr();

	void clear();
	void step(const std::function<void(ClientActiveObject *)> &f) override;
	bool addObject(ClientActiveObject *obj) override;
	void removeObject(u16 id) override;

	void getActiveObjects(const v3f &origin, f32 max_d,
			std::vector<ClientActiveObject *> &dest);

protected:
	SpatialHelper *helper = nullptr;
};
} // namespace client
