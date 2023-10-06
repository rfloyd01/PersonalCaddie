#pragma once

#include <vector>

#include <Math/glm.h>

glm::quat Madgwick(glm::quat q, float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz, float delta_t, float beta);
glm::quat MadgwickModified(glm::quat q, float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz, glm::quat h, float delta_t, float beta);
glm::quat MadgwickVerticalY(glm::quat q, float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz, float delta_t, float beta);
glm::quat MadgwickIMU(glm::quat q, float gx, float gy, float gz, float ax, float ay, float az, float delta_t, float beta);
glm::quat Floyd(glm::quat q, float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz, float bx, float by, float bz, float delta_t, float beta); //attempt at making my own algorithm

//Original Algorithms (only minimal changes to work with glm::quat)
void MadgwickAHRSupdate(glm::quat &q_first, glm::quat& q_second, float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz, float sampleFreq, float beta);
void MadgwickAHRSupdateIMU(glm::quat& q_first, glm::quat& q_second, float gx, float gy, float gz, float ax, float ay, float az, float sampleFreq, float beta);

float invSqrt(float x);