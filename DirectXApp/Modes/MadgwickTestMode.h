#pragma once

#include "Mode.h"

class MadgwickTestMode : public Mode
{
public:
	MadgwickTestMode();

	virtual uint32_t initializeMode(winrt::Windows::Foundation::Size windowSize, uint32_t initialState = 0) override;
	virtual void uninitializeMode() override;

	virtual uint32_t handleUIElementStateChange(int i) override;

	virtual void update() override;
	virtual void addQuaternions(std::vector<glm::quat> const& quaternions) override;
	void setODR(float odr);

private:
	void initializeTextOverlay(winrt::Windows::Foundation::Size windowSize);

	float m_currentRotation;
	float m_currentDegree;

	std::chrono::steady_clock::time_point current_time;

	volatile int m_currentQuaternion;
	DirectX::XMVECTOR m_renderQuaternion; //the current quaternion to be applied on screen
	std::vector<glm::quat> m_quaternions;
	glm::quat m_offsetQuaternion = { 0.372859f, 0.000669f, 0.007263f, 0.914294f };
	std::vector<float> m_timeStamps;

	//Swap and invert axes as normal to get from world coordinates to DirectX coordinates
	int axes_swap[3] = { 1, 2, 0 };
	int axes_invert[3] = { 1, -1, -1 }; //FXOS/FXAS Numbers
	//int axes_invert[3] = { 1, -1, -1 }; //LSM9DS1 Numbers
};