#pragma once

#include <string>
#include <iostream>
#include <vector>
#include <fstream>

using namespace std;

class Gnuplot
{
public:
    Gnuplot();
    ~Gnuplot();
    void operator ()(const string& command); // send any command to gnuplot

protected:
    FILE* gnuplotpipe;
};

//Tester Functions
void graphFromFile(std::string file_location, int data_sets);
void ellipsePlot(std::vector<double>& x_data, std::vector<double>& y_data, std::vector<double>& z_data);
void golfPlot(int axes_size, std::vector<double>& x_data, std::vector<double>& y_data, std::vector<double>& z_data, std::vector<double>& ox_data, std::vector<double>& oy_data, std::vector<double>& oz_data, std::vector<double>& gx, std::vector<double>& gy, std::vector<double>& gz);