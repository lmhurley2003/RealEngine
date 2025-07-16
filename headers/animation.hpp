#pragma once
#include "glm/glm.hpp"
#include <vector>

struct Driver {
	std::vector<float> times{};
	std::vector<float> values{};
	entitySize_t entityID = std::numeric_limits<entitySize_t>().max();
	//union {
	//	std::vector<glm::vec3> vec3Values;
	//	std::vector<glm::quat> quatValues;
	//};
	//entitySize_t entityIdx = std::numeric_limits<entitySize_t>().max();

	Driver() = default;
	//Driver(std::vector<uint32_t> _t, std::vector<glm::vec3> _v) : times(std::move(_t)), vec3Values(std::move(_v)) {};
	//Driver(std::vector<uint32_t> _t, std::vector<glm::quat> _v) : times(std::move(_t)), quatValues(std::move(_v)) {};

	inline bool isChannelTranslation() const {
		return (flags & CHANNEL_TRANSLATION_FLAG);
	};
	inline bool isChannelScale() const {
		return (flags & CHANNEL_SCALE_FLAG);
	};
	inline bool isChannelRotation() const {
		return (flags & CHANNEL_ROTATION_FLAG);
	};

	inline bool isInterpolationStep() const {
		return (flags & INTERPOLATION_STEP_FLAG);
	};
	inline bool isInterpolationLinear() const {
		return (flags & INTERPOLATION_LINEAR_FLAG);
	};
	inline bool isInterpolationSlerp() const {
		return (flags & INTERPOLATION_SLERP_FLAG);
	};

	inline void setChannelTranslation(const bool onOff) {
		if (onOff) (flags |= CHANNEL_TRANSLATION_FLAG);
		else (flags &= ~CHANNEL_TRANSLATION_FLAG);
	};
	inline void setChannelScale(const bool onOff) {
		if (onOff) (flags |= CHANNEL_SCALE_FLAG);
		else (flags &= ~CHANNEL_SCALE_FLAG);
	};
	inline void setChannelRotation(const bool onOff) {
		if (onOff) (flags |= CHANNEL_ROTATION_FLAG);
		else (flags &= ~CHANNEL_ROTATION_FLAG);
	};

	inline void setInterpolationStep(const bool onOff) {
		if (onOff) (flags |= INTERPOLATION_STEP_FLAG);
		else (flags &= ~INTERPOLATION_STEP_FLAG);
	};
	inline void setInterpolationLinear(const bool onOff) {
		if (onOff) (flags |= INTERPOLATION_LINEAR_FLAG);
		else (flags &= ~INTERPOLATION_LINEAR_FLAG);
	};
	inline void setInterpolationSlerp(const bool onOff) {
		if (onOff) (flags |= INTERPOLATION_SLERP_FLAG);
		else (flags &= ~INTERPOLATION_SLERP_FLAG);
	};

private:
	uint8_t flags = 0x0;
	static const uint8_t CHANNEL_TRANSLATION_FLAG = (0x1U << 0);
	static const uint8_t CHANNEL_SCALE_FLAG = (0x1U << 1);
	static const uint8_t CHANNEL_ROTATION_FLAG = (0x1U << 2);
	static const uint8_t INTERPOLATION_STEP_FLAG = (0x1U << 3);
	static const uint8_t INTERPOLATION_LINEAR_FLAG = (0x1U << 4);
	static const uint8_t INTERPOLATION_SLERP_FLAG = (0x1U << 5);
};