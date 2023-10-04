#pragma once

#include "Mode.h"
#include "Devices/PersonalCaddie.h"

enum GraphModeState
{
	RECORDING = 1, //overwrites the active Mode State, turns the Personal Caddie into Sensor Active Mode and enables data notifications
};

class GraphMode : public Mode
{
public:
	GraphMode();

	virtual uint32_t initializeMode(winrt::Windows::Foundation::Size windowSize, uint32_t initialState = 0) override;
	virtual void uninitializeMode() override;

	virtual uint32_t handleUIElementStateChange(int i) override;

	virtual void addData(std::vector<std::vector<std::vector<float> > > const& sensorData, float sensorODR) override;

private:
	void initializeTextOverlay(winrt::Windows::Foundation::Size windowSize);
	DataType getCurrentlySelectedDataType(std::wstring dropDownSelection);

	std::vector<DirectX::XMFLOAT2> m_graphDataX, m_graphDataY, m_graphDataZ;
	DirectX::XMFLOAT2 m_minimalPoint, m_maximalPoint; //used for scaling of the graph
	int m_dataPoints = 1000;
	int m_sinePeaks = 1;
};