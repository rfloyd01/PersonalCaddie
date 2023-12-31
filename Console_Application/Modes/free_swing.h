#pragma once

#include "../Devices/PersonalCaddie.h"
#include "../Modes/mode.h"

class GL;
class Text;
class Mode;

enum class DataType;

class FreeSwing : public Mode
{
public:
	//PUBLIC FUNCTIONS
	//Constructors
	FreeSwing(GL* graphics);

	//Updating and Advancement Functions
	void update(); //virtual allows a sub-class to overwrite the base class' implementation of the function
	void processInput();
	void modeStart();
	void modeEnd();

private:
	//PRIVATE FUNCTIONS
	//Graph Related Functions
	void displayGraph();
	void addGraphData();

	//Data Functions
	void liveUpdate();

	//Utilization Functions
	void makeVec(std::vector<std::vector<float> >& vec, int size);

	//PRIVATE VARIABLES
	//Bool Variables
	bool record_data = 0, display_data = 0; //keeps track of when to show live data on screen and when to record values to graph
	float record_timer = 0; //shows the time stamp of the current piece of data

	//Graph Variables
	std::vector<std::vector<float> > data_set; //records data to graph, graph y azis
	std::vector<float> time_set; //records time to graph, graph x axis
	int last_sample = -1; //the index of the last data sample read from the Personal Caddie

	//Data Variables
	DataType current_data_type; //keeps track of which data type to display and record

	//pointers to sensor data
	/*std::vector<float>* p_data_x;
	std::vector<float>* p_data_y;
	std::vector<float>* p_data_z;*/
};