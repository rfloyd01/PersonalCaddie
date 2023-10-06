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
	std::vector<float> m_timeStamps;
};