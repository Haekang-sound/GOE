#pragma once

struct BoneCache
{
	int scaleIndex = 0;
	int rotIndex = 0;
	int posIndex = 0;
};

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

	/// <summary>
	/// SRT키프레임마다 인덱스가 다르기 때문에 
	/// 올바른 캐시를 위해서 각각의 인덱스로 관리해줘야 한다.
	/// </summary>
	/// <param name="normaliedTime">정규화된 시간</param>
	/// <param name="lastIndex">가장 최근에 검색했던 인덱스</param>
	/// <returns></returns>
	GOE::Matrix4x4 InterpolateSRT(double normaliedTime, BoneCache& lastIndex)
	{
		// 보간함수로 정규화된 시간과 캐시할 인덱스값의 참조를 넘겨준다
		GOE::Matrix4x4 scaleMatrix = InterpolateScale(normaliedTime, lastIndex.scaleIndex);
		GOE::Matrix4x4 rotationMatrix = InterpolateQuatanion(normaliedTime, lastIndex.rotIndex);
		GOE::Matrix4x4 positionMatrix = InterpolatePosition(normaliedTime, lastIndex.posIndex);
		return scaleMatrix * rotationMatrix * positionMatrix;
	}

	GOE::Matrix4x4 InterpolateSR(double normaliedTime)
	{
		GOE::Matrix4x4 scaleMatrix = InterpolateScale(normaliedTime);
		GOE::Matrix4x4 rotationMatrix = InterpolateQuatanion(normaliedTime);
		return scaleMatrix * rotationMatrix;
	}

private:
	/// <summary>
	/// 스키닝에서 프레임을 선택하는 함수
	/// 마지막 키프레임을 기준으로 현재시간에 맞는 키프레임을 검사한다.
	/// 
	/// </summary>
	/// <typeparam name="T">키프레임 자료형 종류</typeparam>
	/// <param name="currentTime">현재 누적시간</param>
	/// <param name="keys">키프레임 벡터</param>
	/// <param name="lastIndex">마지막 인덱스</param>
	/// <returns>현재 누적시간에 적합한 키프레임인덱스</returns>
	template<typename T>
	int FindKeyIndex(double currentTime, const std::vector<T>& keys, int& lastIndex)
	{
		size_t numKeys = keys.size();
		if (numKeys <= 1) return 0;

		//1. 캐시가 유효한지 확인
		// currentTime이 현재 인덱스와 다음 인덱스 사이에 있는지 확인한다.
		if(lastIndex < numKeys -1)
		{
			if (currentTime >= keys[lastIndex].time 
				&& currentTime < keys[lastIndex + 1].time) return lastIndex;
		}

		// 2. 캐시가 빗나갔다면 바로 다음칸 확인
		if (++lastIndex < numKeys - 1)
		{
			if (currentTime >= keys[lastIndex].time
				&& currentTime < keys[lastIndex + 1].time) return lastIndex;
		}

		// 3. 예외상황에서는 이진탐색으로 빠르게 찾는다.
		// wait등으로 인해 프레임이 널뛸경우 index검색이 안될 수 있음
		lastIndex = FindKeyIndex<T>(currentTime, keys);
		return lastIndex;
	}

	/// <summary>
	///  이진탐색으로 키프레임을 찾는 함수
	/// </summary>
	/// </summary>
	/// <typeparam name="T">키프레임 자료형 종류</typeparam>
	/// <param name="currentTime">현재 누적시간</param>
	/// <param name="keys">키프레임 벡터</param>
	/// <returns>현재 누적시간에 적합한 키프레임인덱스</returns>
	template<typename T>
	int FindKeyIndex(double currentTime, const std::vector<T>& keys)
	{
		size_t numKeys = keys.size();
		if (numKeys <= 0) return 0;

		// 이진탐색을 구현합니다.
		int low = 0;
		int high = static_cast<int>(numKeys) - 1;

		while (low <= high)
		{
			int mid = low + (high - low) / 2;
			if (keys[mid].time > currentTime)high = mid - 1;
			else low = mid + 1;
		}

		// while문이 끝나면 low는 currentTime보다 큰 첫 번째 값의 인덱스가 됩니다.
		// 우리가 필요한 건 '현재 시간이 포함된 구간의 시작 키' 이므로 하나를 빼줍니다.
		int index = low - 1;

		// 인덱스 범위 안전 장치
		if (index < 0) return 0;
		if (index >= static_cast<int>(numKeys) - 1) return static_cast<int>(numKeys) - 2;

		return index;
	}
	
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