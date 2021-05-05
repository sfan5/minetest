// can't be bothered with templates

#define CAST_OBJ(obj) static_cast<SpatialIndex::id_type>(reinterpret_cast<intptr_t>(obj))
#define CAST_ID(id) reinterpret_cast<AO_TYPE*>(static_cast<intptr_t>(id))

class SpatialImpl : public SpatialHelper {
public:
	SpatialImpl();
	~SpatialImpl();

	virtual void add(const AO_TYPE *obj);
	virtual void remove(const AO_TYPE *obj);
	virtual void update(const AO_TYPE *obj);

	virtual void getInsideRadius(const v3f &pos, float radius,
			std::vector<AO_TYPE *> &result,
			std::function<bool(AO_TYPE *obj)> include_obj_cb);
	virtual void getInArea(const aabb3f &box,
			std::vector<AO_TYPE *> &result,
			std::function<bool(AO_TYPE *obj)> include_obj_cb);

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
		ResultVisitor(std::vector<AO_TYPE *> &result,
			std::function<bool(AO_TYPE *obj)> callback) :
			m_result(result), m_callback(callback)
		{}
		~ResultVisitor() {}

		virtual void visitNode(const SpatialIndex::INode &in) {}

		virtual void visitData(const SpatialIndex::IData &in)
		{
			SpatialIndex::id_type id = in.getIdentifier();
			AO_TYPE *obj = CAST_ID(id);
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
		std::vector<AO_TYPE *> &m_result;
		std::function<bool(AO_TYPE *obj)> m_callback;
	};

	SpatialIndex::ISpatialIndex *m_tree;
	SpatialIndex::IStorageManager *m_storagemanager;

	// ↓↓↓↓ this is bad
	std::unordered_map<const AO_TYPE*, v3f> pos_cache;
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

void SpatialImpl::add(const AO_TYPE *obj)
{
	m_tree->insertData(0, nullptr,
		spatial_point(obj->getBasePosition()), CAST_OBJ(obj));
	pos_cache[obj] = obj->getBasePosition();
}

void SpatialImpl::remove(const AO_TYPE *obj)
{
	auto it = pos_cache.find(obj);
	assert(it != pos_cache.end());
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

void SpatialImpl::update(const AO_TYPE *obj)
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
}

void SpatialImpl::getInsideRadius(const v3f &pos, float radius,
		std::vector<AO_TYPE *> &result,
		std::function<bool(AO_TYPE *obj)> include_obj_cb)
{
	// Emulate through area query
	float r2 = radius * radius;
	auto cb = [&] (AO_TYPE *obj) -> bool {
		if (obj->getBasePosition().getDistanceFromSQ(pos) > r2)
			return false;
		return !include_obj_cb || include_obj_cb(obj);
	};
	getInArea(aabb3f(pos - v3f(radius), pos + v3f(radius)), result, cb);
}

void SpatialImpl::getInArea(const aabb3f &box,
		std::vector<AO_TYPE *> &result,
		std::function<bool(AO_TYPE *obj)> include_obj_cb)
{
	ResultVisitor visitor(result, include_obj_cb);
	m_tree->containsWhatQuery(spatial_region(box.MinEdge, box.MaxEdge), visitor);
}
