#pragma once

#include <map>
#include <vector>
#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "../Devices/PersonalCaddie.h"
#include "../Math/glm.h"
#include "../Modes/mode.h"

#include "model.h"
#include "shader.h"
#include "text.h"

//Classes, structs and enums defined in other headers
class Shader;
class Model;
class Mode;
class PersonalCaddie;

struct Text;
struct Character;

enum class ModeType;
enum class MessageType;
enum class DataType;
enum Axis;

//Main Graphics Class
class GL
{
public:
	//PUBLIC FUNCTIONS
	//Constructors
	GL(PersonalCaddie* personal_caddie);

	//Setup Functions
	//void LoadTexture(const char* name);
	GLFWwindow* GetWindow();

	//Rendering Functions
	void masterRender();

	//Text Based Functions
	Character* getCharacterInfo(char c);

	//Cleanup Functions
	bool ShouldClose();
	void setWindowShouldClose();
	void Terminate();

	//Key Press Functions
	bool GetCanPressKey();
	double GetKeyTime();
	void resetKeyTimer();

	//Utilization Functions
	void masterUpdate();

	//Graph Related Functions
	void DisplayGraph();
	void RecordData();
	void AddData();

	//Screen Size Functions
	float getScreenWidth();
	float getScreenHeight();

	//Sensor Functions
	//These functions only exist to pass variables directly from Sensor to mode classes
	float getDataPoint(DataType dt, Axis a, int sample_number);
	glm::quat getRotationQuaternion(int sample);
	glm::quat getOpenGLQuaternion(int sample);
	int getCurrentSample();
	float getCurrentTime();
	void resetTime();
	//void updateCalibrationNumbers();
	void setMagField();
	void setRotationQuaternion(glm::quat q, int sample);
	PersonalCaddie* getPersonalCaddie(); //returns a pointer to the currently paired Personal Caddie device

	//Mode Functions
	void addMode(Mode* m);
	Mode* getCurrentMode();
	void setCurrentMode(ModeType m);

	//Event Handlers
	void handlePersonalCaddieUpdate(int code);

	//PUBLIC VARIABLES
	bool display_readings = 0, record_data = 0; //these variables keep track of when to show sensor data and record data for creating graphs
	int current_display = 0; //keeps track of what sensor, if any, to display on screen

private:
	//PRIVATE FUNCTIONS
	//Setup Functions
	void Initialize();
	void InitializeText();
	void initializeModes();

	//Buffer and Texture Update Functions
	void setTextBuffers();
	void DeleteBuffers();
	void BindTexture(unsigned int tex);
	void Swap();

	//Rendering Functions
	void renderText();
	void renderModels();
	void drawCharacter(char character, float& x_location, float y_location, float scale);

	//Key Press Functions
	void setCanPressKey();
	
	//PRIVATE VARIABLES
	//Data type variable
	DataType data_type; //keeps track of the current data type (acceleration, magnetic, etc.) that is currently being displayed and graphed

	//OpenGL Variables
	float version = 3.3; //the version of OpenGL to be used
	GLFWwindow* window;
	unsigned int VBO, VAO, TVBO, TVAO, LVBO, LVAO;
	Shader clubShader, textShader, modelShader, lineShader;

	//Rendering Variables
	//glm::vec3 club_translate, club_scale;
	unsigned int club_model_location, club_view_location;
	std::vector<unsigned int> textures;

	//Viewscreen Variables
	int screen_width = 800, screen_height = 600;

	//Key Press Variables
	bool can_press_key; //can_press_key will disable keyboard presses momentarily after a key is pressed, to make sure the same key isn't pressed multiple times in one go
	double key_timer, key_time; //key timer keeps track of time elapsed since last time a key was pressed, key_time sets the limit on time allowable between key presses

	//Text Variables
	std::map<char, Character> Characters; //a map that holds rendering info on the first 128 ASCII characters, info includes shape, dimensions, etc.

	//Graph Variables
	std::vector<std::vector<float> > data_set; //records data to graph, graph y azis
	std::vector<float> time_set; //records time to graph, graph x axis

	//Mode Variables
	std::map<ModeType, Mode*> mode_map;

	//Class pointers
	Mode* p_current_mode;
	PersonalCaddie* p_pc;
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height);