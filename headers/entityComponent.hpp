#pragma once
#include <unordered_map>
#include <cassert>

//TODO make macro to choose uint64_t for entity size instead ?
typedef uint32_t entitySize_t;

struct Entity {
private:
	entitySize_t id;
	inline static uint32_t currentEntities = 0; //current number of entities in scene, incremented on entity addition, decremented on entity deletetion
	inline static uint32_t totalEntities = 0; //total number of entities instantiated in runtime of scene, incremented on entity addition, NOT decremented on entity deletion
	
	static const uint32_t ENTITY_IS_ENABLED = (0x1U << 0);
	static const uint32_t ENTITY_IS_STATIC = (0x1U << 1); //if not static, is dynamc
	static const uint32_t ENTITY_IS_DRIVER_ANIMATED = (0x1U << 2); //TODO implement driver animations, also maybe don't need static flag, just determine from whether it has animation components
	static const uint32_t ENTITY_HAS_MESH = (0x1U << 3);
	static const uint32_t ENTITY_IS_BONE_ANIMATED = (0x1U << 4);
	static const uint32_t ENTITY_HAS_LIGHT = (0x1U << 5);
	static const uint32_t ENTITY_HAS_CAMERA = (0x1U << 6);
	static const uint32_t ENTITY_HAS_ORBIT_CONTROL = (0x1U << 7);
	static const uint32_t ENTITY_HAS_ENVIRONMENT_NODE = (0x1U << 8);

	entitySize_t flags = (0x0U | ENTITY_IS_ENABLED | ENTITY_IS_STATIC); //TODO is flag paramter necessary ?

public:
	bool isEnabled() const;
	bool isStatic() const;
	bool isDriverAnimated() const;
	bool hasMesh() const;
	bool isBoneAnimated() const;
	bool hasLight() const;
	bool hasCamera() const;
	bool hasOrbitControl() const;
	bool hasEnvironmentNode() const;

	void setIsEnabled(bool onOff);
	void setIsStatic(bool onOff);
	void setIsDriverAnimated(bool onOff);
	void setHasMesh(bool onOff);
	void setIsBoneAnimation(bool onOff);
	void setHasLight(bool onOff);
	void setHasCamera(bool onOff);
	void setHasOrbitControl(bool onOff);
	void setHasEnvironmentNode(bool onOff);

	entitySize_t getID() const { return id; };

	Entity() {
		if (!totalEntities) totalEntities = 0;
		if (!currentEntities) currentEntities = 0;
		id = totalEntities;
		currentEntities++;
		totalEntities++;
	};
};

//TODO make _data switchback vector so idxs are consitent when removing entity
template<typename T>
class EnitityComponents {
	// key : entity id, value : idxs into _data array
	std::unordered_map<entitySize_t, uint32_t> _idxs{};
	std::vector<T> _data{};
public:
	
	T& get(entitySize_t id) {
		assert(_idxs.count(id) && (_idxs[id] < _data.size()));
		return _data.at(_idxs[id]);
	}

	T& get(const Entity& entity) {
		entitySize_t id = entity.getID();
		assert(_idxs.count(id) && _idxs[id] < _data.size());
		return _data.at(_idxs[id]);
	}

	template<typename U>
	uint32_t insert(entitySize_t key, U&& val) {
		static_assert(std::is_same<std::decay_t<U>, T>::value, "Inserted type must be exactly T");
		uint32_t idx = static_cast<uint32_t>(_data.size());
		_idxs.insert({ key, idx });
		_data.emplace_back(std::forward<U>(val));
		return idx;
	}

	template<typename U>
	uint32_t insert(const Entity& key, U&& val) {
		static_assert(std::is_same<std::decay_t<U>, T>::value, "Inserted type must be exactly T");
		uint32_t idx = static_cast<uint32_t>(_data.size());
		_idxs.insert({key.getID(), idx });
		_data.emplace_back(std::forward<U>(val));
		return idx;
	}

	//set entity has component from component that already exists in EntiyComponent
	void insertExisting(const Entity& key, uint32_t idx) {
		_idxs.insert({ key.getID(), idx });
	}

	void insertExisting(entitySize_t id, uint32_t idx) {
		_idxs.insert({ id, idx });
	}

	bool contains(const Entity& key) {
		return _idxs.count(key.getID());
	}

	bool contains(entitySize_t ID) {
		return _idxs.count(ID);
	}

	std::unordered_map<entitySize_t, uint32_t>::const_iterator mapBegin() {
		return _idxs.cbegin();
	}

	std::unordered_map<entitySize_t, uint32_t>::const_iterator mapIterator(entitySize_t entityID) {
		return _idxs.find(entityID);
	}

	std::unordered_map<entitySize_t, uint32_t>::const_iterator mapIterator(const Entity& entity) {
		return _idxs.find(entity.getID());
	}

	std::unordered_map<entitySize_t, uint32_t>::const_iterator mapEnd() {
		return _idxs.cend();
	}

	std::vector<T>::iterator dataBegin() {
		return _data.begin();
	}

	std::vector<T>::iterator dataEnd() {
		return _data.end();
	}
};


