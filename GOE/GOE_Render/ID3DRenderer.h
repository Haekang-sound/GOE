#pragma once

/// <summary>
/// 사용자가 다루게될 최전방의 인터페이스 
/// 
/// ohk 2025.06.21
/// </summary>
class ID3DRenderer
{
public:
	virtual ~ID3DRenderer() {}

	virtual void OnInit() = 0;
	virtual void OnUpdate() = 0;
	virtual void BeginRender() = 0;
	virtual void OnRender() = 0;
	virtual void EndRender() = 0;
	virtual void OnDestroy() = 0;

};
