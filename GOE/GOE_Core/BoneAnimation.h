#pragma once

struct VectorKeyFrame
{
	double time = 0.0f;
	GOE::FLoatVector3 value = {};
	VectorKeyFrame(double time, GOE::FLoatVector3 value) : time(time), value(value) {}
};

struct QuatKeyFrame
{
	double time = 0.0f;
	GOE::FLoatVector4 value = {};
	QuatKeyFrame(double time, GOE::FLoatVector4 value) : time(time), value(value) {}
};


class BoneAnimation
{
public:
	BoneAnimation(std::string name, size_t hash) : m_name(name), m_id(hash) {}
	BoneAnimation(size_t hash) : m_id(hash) {}
	~BoneAnimation();

public:
	inline const std::string& GetName() const { return m_name; }
	inline const std::size_t GetID() const { return m_id; }
	inline const std::vector<VectorKeyFrame>& GetScales() { return m_scales; }
	inline const std::vector<VectorKeyFrame>& GetPositions() { return m_positions; }
	inline const std::vector<QuatKeyFrame>& GetQuatanions() { return m_quatanions; }
	inline const int GetScaleCount() const { return static_cast<int>(m_scales.size()); }
	inline const int GetPositionCount() const { return static_cast<int>(m_positions.size()); }
	inline const int GetRotationCount() const { return static_cast<int>(m_quatanions.size()); }


public:
	inline void SetName(const std::string& name) { m_name = name; }

	inline void AddScales(VectorKeyFrame scale) { m_scales.emplace_back(scale); }
	inline void AddQuatanions(QuatKeyFrame quat) { m_quatanions.emplace_back(quat); }
	inline void AddPositions(VectorKeyFrame position) { m_positions.emplace_back(position); }

public:
	GOE::Matrix4x4 InterpolateSRT(double normaliedTime)
	{
		GOE::Matrix4x4 scaleMatrix = InterpolateScale(normaliedTime);
		GOE::Matrix4x4 rotationMatrix = InterpolateQuatanion(normaliedTime);
		GOE::Matrix4x4 positionMatrix = InterpolatePosition(normaliedTime);
		return scaleMatrix * rotationMatrix * positionMatrix;
	}

	GOE::Matrix4x4 InterpolateSRT(double normaliedTime,  int& lastIndex)
	{
		GOE::Matrix4x4 scaleMatrix = InterpolateScale(normaliedTime, lastIndex);
		GOE::Matrix4x4 rotationMatrix = InterpolateQuatanion(normaliedTime, lastIndex);
		GOE::Matrix4x4 positionMatrix = InterpolatePosition(normaliedTime, lastIndex);
		return scaleMatrix * rotationMatrix * positionMatrix;
	}

	GOE::Matrix4x4 InterpolateSR(double normaliedTime)
	{
		GOE::Matrix4x4 scaleMatrix = InterpolateScale(normaliedTime);
		GOE::Matrix4x4 rotationMatrix = InterpolateQuatanion(normaliedTime);
		return scaleMatrix * rotationMatrix;
	}

private:
	template<typename T>
	int FindKeyIndex(double currentTime, const std::vector<T>& keys, int& lastIndex)
	{
		size_t numKeys = keys.size();
		if (numKeys <= 0) return 0;

		//1. 캐시가 유효한지 확인
		if(lastIndex < numKeys -1)
		{
			if (currentTime >= keys[lastIndex].time && currentTime < keys[lastIndex + 1].time)
			{
				return lastIndex;
			}
		}

		// 2. 캐시가 빗나갔다면? 바로 다음칸 부터 확인해본다
		if (lastIndex + 1 < numKeys - 1)
		{
			if (currentTime >= keys[lastIndex + 1].time
				&& currentTime < keys[lastIndex + 2].time)
			{
				lastIndex++;
				return lastIndex;
			}
		}

		// 3. 예외상황: 역재생, 점프같은경우 이진탐색으로 찾아서 lastindex갱신

		int low = 0;
		int high = static_cast<int>(numKeys) - 1;
		while (low <= high)
		{
			int mid = low + (high - low) / 2;
			if (keys[mid].time > currentTime) high = mid - 1;
			else low = mid + 1;
		}

		int foundIndex = low - 1;
		if (foundIndex < 0) foundIndex = 0;
		if (foundIndex >= static_cast<int>(numKeys)-1) foundIndex = static_cast<int>(numKeys) -2;

		lastIndex = foundIndex;
		return foundIndex;
	}


	template<typename T>
	int FindKeyIndex(double currentTime, const std::vector<T>& keys)
	{
		size_t numKeys = keys.size();
		if (numKeys <= 0) return 0;

		// 이진탐색을 구현합니다.
		// 목표 currnetTime 보다 큰 첫 번째 키를 찾아 그 직전 인덱스를 구함
		int low = 0;
		int high = static_cast<int>(numKeys) - 1;

		while (low <= high)
		{
			int mid = low + (high - low) / 2;

			if (keys[mid].time > currentTime)
			{
				high = mid - 1;
			}
			else
			{
				low = mid + 1;
			}
		}

		// while문이 끝나면 low는 currentTime보다 큰 첫 번째 값의 인덱스가 됩니다.
		// 우리가 필요한 건 '현재 시간이 포함된 구간의 시작 키' 이므로 하나를 빼줍니다.
		int index = low - 1;

		// 인덱스 범위 안전 장치(보정)
		if (index < 0) return 0;
		if (index >= static_cast<int>(numKeys) - 1) return static_cast<int>(numKeys) - 2;

		return index;

	}
	//template <typename T> // T = VectorKeyFrame 또는 QuatKeyFrame
	//int FindKeyIndex(double currentTime, const std::vector<T>& keys)
	//{
	//	size_t numKeys = keys.size();

	//	// 방어 코드 1: 키가 하나뿐이면 항상 0번 인덱스
	//	if (numKeys <= 1)
	//	{
	//		return 0;
	//	}

	//	// 선형 탐색 (Linear Search)
	//	for (int i = 0; i < numKeys - 1; i++)
	//	{
	//		// "현재 시간(currentTime)"이 "다음 키프레임(keys[i+1])의 시간"보다
	//		// 작은 순간을 찾습니다.
	//		if (currentTime < keys[i + 1].time)
	//		{
	//			// 찾았습니다. 현재 시간은 [i]와 [i+1] 사이입니다.
	//			// KeyA의 인덱스인 'i'를 반환합니다.
	//			return i;
	//		}
	//	}

	//	// 방어 코드 2: 루프를 다 돌았다면 (현재 시간이 마지막 키보다도 뒤)
	//	// 마지막 유효 구간인 [마지막-2, 마지막-1]의
	//	// KeyA 인덱스(numKeys - 2)를 반환합니다.
	//	return static_cast<int>(numKeys) - 2;
	//}
	
	// 캐싱테스트용 함수
	GOE::Matrix4x4 InterpolateScale(double normaliedTime, int& cacheIndex);
	GOE::Matrix4x4 InterpolatePosition(double normaliedTime, int& cacheIndex);
	GOE::Matrix4x4 InterpolateQuatanion(double normaliedTime, int& cacheIndex);
	GOE::Matrix4x4 InterpolateScale(double normaliedTime);
	GOE::Matrix4x4 InterpolatePosition(double normaliedTime);
	GOE::Matrix4x4 InterpolateQuatanion(double normaliedTime);

private:
	std::string m_name = "";
	size_t m_id = 0;

	std::vector<VectorKeyFrame> m_scales;
	std::vector<QuatKeyFrame> m_quatanions;
	std::vector<VectorKeyFrame> m_positions;
};