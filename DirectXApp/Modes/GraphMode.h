#pragma once

#include "Mode.h"
#include "Devices/PersonalCaddie.h"

class GraphMode : public Mode
{
public:
	GraphMode();

	virtual uint32_t initializeMode(winrt::Windows::Foundation::Size windowSize, uint32_t initialState = 0) override;
	virtual void uninitializeMode() override;

	virtual void update() override;
	virtual void handleKeyPress(winrt::Windows::System::VirtualKey pressedKey) override;

	virtual void addData(std::vector<std::vector<std::vector<float> > > const& sensorData, float sensorODR, float timeStamp, int totalSamples) override;
	virtual void addQuaternions(std::vector<glm::quat> const& quaternions, int quaternion_number, float time_stamp, float delta_t) override;
	virtual void pc_ModeChange(PersonalCaddiePowerMode newMode) override;
	virtual void getIMUHeadingOffset(glm::quat heading) override;

private:
	void initializeTextOverlay();
	DataType getCurrentlySelectedDataType(std::wstring dropDownSelection);
	float testIntegrateData(float p1, float p2, float t);
	void convergenceCheck();
	void toggleCalculatedDataTypes();

	void loadModel();

	void resetData();
	bool dataTypeSelected(DataType t);
	std::wstring getDataTypeText(DataType t);

	//Handler Methods
	virtual void uiElementStateChangeHandler(std::shared_ptr<ManagedUIElement> element) override;

	std::chrono::steady_clock::time_point data_collection_start, data_receieved;
	bool m_recording;
	//DataType m_currentDataType; //deprecated
	uint32_t m_selectedDataTypes;

	//Variables for detecting Madgwick filter convergence
	bool m_converged = true;
	std::vector<glm::quat> m_convergenceQuaternions;

	//Rendering Variables
	std::chrono::steady_clock::time_point data_start_timer;
	volatile int m_currentQuaternion;
	volatile bool m_update_in_process;
	DirectX::XMVECTOR m_renderQuaternion; //the current quaternion to be applied on screen
	std::vector<glm::quat> m_quaternions;
	glm::quat m_headingOffset = { 1.0f, 0.0f, 0.0f, 0.0f };
	std::vector<float> m_timeStamps;
	int computer_axis_from_sensor_axis[3] = { 1, 2, 0 }; //Array used to swap real world coordinates to DirectX coordinates

	std::vector<DirectX::XMFLOAT2> m_graphDataX, m_graphDataY, m_graphDataZ; //deprecated
	std::vector<std::vector<std::vector<DirectX::XMFLOAT2> > > m_graphData = {}; //Holds data from the sensor for all possible data types
	//std::vector<std::vector<std::pair<float, float> > > m_graphDataExtremes = {}; //Holds the min/max values for each axes of each data type being recorded
	DirectX::XMFLOAT2 m_minimalPoint, m_maximalPoint; //used for scaling of the graph
	std::vector<UIColor> m_lineColors; //holds multiple colors to be graphed
	int m_currentLineColor; //used to select a graph line color from the above vector
	//int m_dataPoints = 1000;
	//int m_sinePeaks = 1;
};