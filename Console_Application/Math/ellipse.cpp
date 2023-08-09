#include "pch.h"

#include <iostream>
#include <Math/ellipse.h>

void eTest()
{
    std::srand(std::rand());
    std::vector<double> test_x, test_y, test_z, point;

    //First define an ellipsoid by setting the values for roll, pitch, yaw, rx, ry, rz, a, b and c
    double roll = 3.14159 / 2.0;
    double pitch = 3.14159 / 2.0;
    double yaw = 3.14159 / 5.613;
    double rx = 30;
    double ry = 20;
    double rz = 10;
    double a = 20;
    double b = 0;
    double c = 0;

    for (float u = 0; u < 2 * 3.14159; u += 3.14159 / 5.0)
    {
        for (float v = -3.14159; v < 3.14159; v += 3.14159 / 10.0)
        {
            point = getEllipsePoint(roll, pitch, yaw, rx, ry, rz, a, b, c, u, v);
            test_x.push_back(point[0]);
            test_y.push_back(point[1]);
            test_z.push_back(point[2]);
        }
    }

    //plot ellipse to make sure it matches the given parameters
    ellipsePlot(test_x, test_y, test_z);

    //Calculate Ellipsoid Parameters
    std::vector<Eigen::MatrixXf> matrixParameters = getEllipseMatrix(roll, pitch, yaw, rx, ry, rz, a, b, c);

    //If ellipoid parameters are correct, then every point (x, y, z) on the ellipsoid can be fed into the quadratic equation described by matrixParameters and return a value of 0
    for (int i = 0; i < test_x.size(); i++)
    {
        std::cout << "Point = [" << test_x[i] << ", " << test_y[i] << ", " << test_z[i] << "]  ---> " << testEllipseMatrix(test_x[i], test_y[i], test_z[i], matrixParameters) << std::endl;

        double xa = test_x[i] - a, yb = test_y[i] - b, zc = test_z[i] - c;
        double s1 = sin(yaw), s2 = sin(pitch), s3 = sin(roll);
        double c1 = cos(yaw), c2 = cos(pitch), c3 = cos(roll);
        double rx2 = rx * rx, ry2 = ry * ry, rz2 = rz * rz;
        double M = c1 * c2;
        double N = s1 * c3 - c1 * s2 * s3; //
        double O = s1 * s3 + c1 * s2 * c3;
        double P = s1 * c2;//
        double Q = c1 * c3 + s1 * s2 * s3;
        double R = c1 * s3 - s1 * s2 * c3;
        double S = s2;//
        double T = c2 * s3;
        double U = c2 * c3;

        double first = xa * M + yb * N + zc * O;
        double second = -xa * P + yb * Q + zc * R;
        double third = -xa * S - yb * T + zc * U;

        std::cout << "                        Other formula ---> " << 1.0 / rx2 * first * first + 1.0 / ry2 * second * second + 1.0 / rz2 * third * third << std::endl << std::endl;
    }

}

void circleTest()
{
    std::vector<double> test_x, other_x, test_y, other_y, test_z, other_z, point;
    float swing_radius = 60.0; //in inches
    float shallow_angle = 68.5; //in degrees
    float minimum_angle_of_attack = 5.0;
    float minimum_angle_off_target = 5.0;
    float angle_off_of_target = 180.0; //club starts downswing by moving away from target
    float iterations = 10000.0;
    float current_height, x, y, z, old_x, old_y, old_z;

    float strike_window_length = 0;
    int count = 0;

    for (float t = 3.14159; t >= -3.14159; t -= (2 * 3.14159 / iterations))
    {
        //make a parametric circle with 50 data points, going from negative pi to positive pi
        current_height = -cosf(t) * swing_radius + swing_radius;
        old_x = x; old_y = y; old_z = z;
        x = sinf(t) * swing_radius;
        y = current_height * cosf(shallow_angle * 3.14159 / 180.0);
        z = current_height * sinf(shallow_angle * 3.14159 / 180.0);

        if (t < 3.14159) //this line just makes sure it isn't the first iteration of the loop
        {
            //find angle between current change vector in x-y plane (ignore change in z direction) and vector from last point along taget line
            //target line is straight down x axis. Accomplish this using formula A * B = mag(A) x mag(B) x cos(theta) where * is the dot product
            //A vector is just unit vector down the -x axis so use A = (-1, 0) and B = (x - x_old, y - yold)
            //A * B just equals x_old - x since Y component of A is 0
            //std::cout << "X-Y Change Vector is (" << x - old_x << ", " << y - old_y << ")" << std::endl;
            angle_off_of_target = acos((old_x - x) / sqrt((x - old_x) * (x - old_x) + (y - old_y) * (y - old_y)));
            angle_off_of_target *= (180.0 / 3.14159); //convert from radians to degrees
        }
        //only graph the point if it meets the threshold for minimum angle of attack
        /*
        if ((atan(z / sqrt(x * x + y * y)) * 180.0 / 3.14159) <= minimum_angle_of_attack)
        {
            //this if statement was used to test when angle between clubhead and ground was 5 degrees or less
            test_x.push_back(sinf(t) * swing_radius);
            test_y.push_back(current_height * cosf(shallow_angle * 3.14159 / 180.0));
            test_z.push_back(current_height * sinf(shallow_angle * 3.14159 / 180.0));

            //if (count > 0) strike_window_length += sqrt(pow((test_x[count] - test_x[count - 1]), 2) + pow((test_y[count] - test_y[count - 1]), 2) + pow((test_z[count] - test_z[count - 1]), 2));
            if (count == 0) strike_window_length = -x; //x-distance from club when angle of attack becomes shallow enough to ball in inches
            count++;
        }
        */
        if ((angle_off_of_target <= minimum_angle_off_target))
        {
            //this if statement checks when the current direction of the club is within a reasonable vector to the target line (off by no more than 5 degrees)
            //it does this by drawing a vector between the current point of the swing and the last point and checking the angle between it and the target line in the X-Y plane
            test_x.push_back(sinf(t) * swing_radius);
            test_y.push_back(current_height * cosf(shallow_angle * 3.14159 / 180.0));
            test_z.push_back(current_height * sinf(shallow_angle * 3.14159 / 180.0));

            //if (count > 0) strike_window_length += sqrt(pow((test_x[count] - test_x[count - 1]), 2) + pow((test_y[count] - test_y[count - 1]), 2) + pow((test_z[count] - test_z[count - 1]), 2));
            if (count == 0) strike_window_length = x; //x-distance from club when angle of attack becomes shallow enough to ball in inches
            count++;
        }
        else
        {
            other_x.push_back(sinf(t) * swing_radius);
            other_y.push_back(current_height * cosf(shallow_angle * 3.14159 / 180.0));
            other_z.push_back(current_height * sinf(shallow_angle * 3.14159 / 180.0));
        }
    }

    //plot a golf ball on the same graph
    std::vector<double> golf_x, golf_y, golf_z;
    for (float u = 0; u < 2 * 3.14159; u += 3.14159 / 5.0)
    {
        for (float v = -3.14159; v < 3.14159; v += 3.14159 / 10.0)
        {
            point = getEllipsePoint(0, 0, 0, 0.84, 0.84, 0.84, 0, 0, 0, u, v);
            golf_x.push_back(point[0]);
            golf_y.push_back(point[1]);
            golf_z.push_back(point[2]);
        }
    }
    //plot ellipse to make sure it matches the given parameters
    std::cout << "Length of strike window is " << strike_window_length << " inches." << std::endl;
    golfPlot(2 * swing_radius + 10, test_x, test_y, test_z, other_x, other_y, other_z, golf_x, golf_y, golf_z);
}

std::vector<double> getEllipsePoint(float roll, float pitch, float yaw, float xr, float yr, float zr, float x_off, float y_off, float z_off, float u, float v)
{
    //This function takes an ellipse defined by the first 9 parrameters, applies the parametric values u and v and then updates x, y and z with a point on the ellipse
    float R1[3][3] = { {cosf(yaw), sinf(yaw), 0}, {-sinf(yaw), cosf(yaw), 0}, {0, 0, 1} }; //rotation in z-axis
    float R2[3][3] = { {cosf(pitch), 0, sinf(pitch)}, {0, 1, 0} , {-sinf(pitch), 0, cosf(pitch)} }; //rotation in y-axis
    float R3[3][3] = { {1, 0, 0}, {0, cosf(roll), sinf(roll)}, {0, -sinf(roll), cosf(roll)} }; //rotation in x-axis
    float V[3][1] = { {xr * cosf(u) * cosf(v)}, {yr * sinf(u) * cosf(v)}, {zr * sinf(v)} };

    //Multiply R1 and R2 to get R12
    float R12[3][3] = {
        {R1[0][0] * R2[0][0] + R1[0][1] * R2[1][0] + R1[0][2] * R2[2][0], R1[0][0] * R2[0][1] + R1[0][1] * R2[1][1] + R1[0][2] * R2[2][1], R1[0][0] * R2[0][2] + R1[0][1] * R2[1][2] + R1[0][2] * R2[2][2]},
        {R1[1][0] * R2[0][0] + R1[1][1] * R2[1][0] + R1[1][2] * R2[2][0], R1[1][0] * R2[0][1] + R1[1][1] * R2[1][1] + R1[1][2] * R2[2][1], R1[1][0] * R2[0][2] + R1[1][1] * R2[1][2] + R1[1][2] * R2[2][2]},
        {R1[2][0] * R2[0][0] + R1[2][1] * R2[1][0] + R1[2][2] * R2[2][0], R1[2][0] * R2[0][1] + R1[2][1] * R2[1][1] + R1[2][2] * R2[2][1], R1[2][0] * R2[0][2] + R1[2][1] * R2[1][2] + R1[2][2] * R2[2][2]}
    };

    //multiply R12 by R3 to get R123
    float R123[3][3] = {
        {R12[0][0] * R3[0][0] + R12[0][1] * R3[1][0] + R12[0][2] * R3[2][0], R12[0][0] * R3[0][1] + R12[0][1] * R3[1][1] + R12[0][2] * R3[2][1], R12[0][0] * R3[0][2] + R12[0][1] * R3[1][2] + R12[0][2] * R3[2][2]},
        {R12[1][0] * R3[0][0] + R12[1][1] * R3[1][0] + R12[1][2] * R3[2][0], R12[1][0] * R3[0][1] + R12[1][1] * R3[1][1] + R12[1][2] * R3[2][1], R12[1][0] * R3[0][2] + R12[1][1] * R3[1][2] + R12[1][2] * R3[2][2]},
        {R12[2][0] * R3[0][0] + R12[2][1] * R3[1][0] + R12[2][2] * R3[2][0], R12[2][0] * R3[0][1] + R12[2][1] * R3[1][1] + R12[2][2] * R3[2][1], R12[2][0] * R3[0][2] + R12[2][1] * R3[1][2] + R12[2][2] * R3[2][2]}
    };

    double x = R123[0][0] * V[0][0] + R123[0][1] * V[1][0] + R123[0][2] * V[2][0];
    double y = R123[1][0] * V[0][0] + R123[1][1] * V[1][0] + R123[1][2] * V[2][0];
    double z = R123[2][0] * V[0][0] + R123[2][1] * V[1][0] + R123[2][2] * V[2][0];

    x += x_off;
    y += y_off;
    z += z_off;

    return { x, y, z };
}
std::vector<Eigen::MatrixXf> getEllipseMatrix(float roll, float pitch, float yaw, float xr, float yr, float zr, float a, float b, float c)
{
    //Algebraically a general ellipsoid can be described with the following equation: X^t*Abar*X + bbar^t*X + cbar = 0
    //Abar is a 3x3 positive definte matrix of the form [[A, D/2, E/2], [D/2, B, F/2], [E/2, F/2, C]], bbar is a column vector of the form [[G], [H], [I]] amd cbar is a scalar
    //which can be represented with a J. The above equation when multiplied out yields the following quadratic equation: Ax^2 + By^2 + Cz^2 + Dxy + Exz + Fyz + Gx + Hy + Iz + J = 0 
    //If the features for an ellipsoid are known (i.e. minor radii, center and angles of rotation) then the parameters A-J can be defined in terms of those features.
    //This function calculates A-J for a known ellipsoid and returns a vector containing Abar, bbar and cbar.

    //first calculate sine and cosine values for angles of rotation and r^2 values to avoid repeat math
    double s1 = sin(yaw), s2 = sin(pitch), s3 = sin(roll);
    double c1 = cos(yaw), c2 = cos(pitch), c3 = cos(roll);
    double rx2 = xr * xr, ry2 = yr * yr, rz2 = zr * zr;

    //store sine and cosine equations into variables named M through U to get rid of repeated math and keep code cleaner
    double M = c1 * c2;
    double N = s1 * c3 - c1 * s2 * s3; //
    double O = s1 * s3 + c1 * s2 * c3;
    double P = s1 * c2;//
    double Q = c1 * c3 + s1 * s2 * s3; 
    double R = c1 * s3 - s1 * s2 * c3;
    double S = s2;//
    double T = c2 * s3;
    double U = c2 * c3;

    //angle 1 and 2 are 0, angle 3 is arbitrary
    //M = 1, N = 0, O = 0, P = 0, Q = c3, R = s3, S = 0, T = s3, U = c3
    //angle 1 is 0, angle 2 is 90, angle 3 is arbitrary
    //M = 0, N = -s3, O = c3, P = 0, Q = c3, R = s3, S = 1, T = 0, U = 0

    //calculate A - J by using the variables M - U
    double A = (M * M / rx2) + (P * P / ry2) + (S * S / rz2);
    double B = (N * N / rx2) + (Q * Q / ry2) + (T * T / rz2);
    double C = (O * O / rx2) + (R * R / ry2) + (U * U / rz2);
    double D = 2 * ((M * N / rx2) - (P * Q / ry2) + (S * T / rz2)); //seems to be something wrong with this line
    double E = 2 * ((M * O / rx2) - (P * R / ry2) - (S * U / rz2)); //seems to be something wrong with this line
    double F = 2 * ((N * O / rx2) + (Q * R / ry2) - (T * U / rz2)); //seems to be something wrong with this line
    double G = 2 * ((M * (-a * M - b * N - c * O) / rx2) + (P * (-a * P + b * Q + c * R) / ry2) + (S * (-a * S - b * T + c * U) / rz2));
    double H = 2 * ((N * (-a * M - b * N - c * O) / rx2) + (Q * (a * P - b * Q - c * R) / ry2) + (T * (-a * S - b * T + c * U) / rz2));
    double I = 2 * ((O * (-a * M - b * N - c * O) / rx2) + (R * (a * P - b * Q - c * R) / ry2) + (U * (a * S + b * T - c * U) / rz2));
    double J = ((a*a*M*M + 2*a*b*M*N + 2*a*c*M*O + b*b*N*N + 2*b*c*N*O + c*c*O*O) / rx2) + ((a*a*P*P - 2*a*b*P*Q - 2*a*c*P*R + b*b*Q*Q + 2*b*c*Q*R + c*c*R*R) / ry2)
        + ((a*a*S*S + 2*a*b*S*T - 2*a*c*S*U + b*b*T*T - 2*b*c*T*U + c*c*U*U) / rz2);

    //angle 1 and 2 are 0, angle 3 is arbitrary
    //A = 1/rx2, B = c3c3/ry2 + s3s3/rz2, C = s3s3/ry2 + c3c3/rz2, D = 0, E = 0, F = 2 * ((c3s3/ry2) - (s3c3/rz2)), G = -2a/rx2, H = 2 * ((c3*(-b*c3 - c*s3)/ry2) + (s3*(-b*s3 + c*c3)/rz2)), I = 2 * ((s3*(-b*c3 - c*s3)/ry2) + (c3*(b*s3 - c*c3)/rz2)), J = (aa/rx2) + ((bb*c3c3 + 2bc*c3s3 + cc*s3s3)/ry2) + ((bb*s3s3 -2bc*s3c3 + cc*c3c3)/rz2)
    //Works when every parameter has a value other than D and E, test to see what happens when D or E isn't zero

    Eigen::MatrixXf Abar(3, 3);
    Abar(0, 0) = A; Abar(0, 1) = D / 2.0; Abar(0, 2) = E / 2.0;
    Abar(1, 0) = D / 2.0; Abar(1, 1) = B; Abar(1, 2) = F / 2.0;
    Abar(2, 0) = E / 2.0; Abar(2, 1) = F / 2.0; Abar(2, 2) = C;

    Eigen::MatrixXf bbar(1, 3);
    bbar(0, 0) = G; bbar(0, 1) = H; bbar(0, 2) = I;

    Eigen::MatrixXf cbar(1, 1);
    cbar(0, 0) = J;

    std::cout << "A = " << A << ", B = " << B << ", C = " << C << ", D = " << D << ", E = " << E << ", F = " << F << ", G = " << G << ", H = " << H << ", I = " << I << ", J = " << J << std::endl;
    
    return { Abar, bbar, cbar };
}
double testEllipseMatrix(double x, double y, double z, std::vector<Eigen::MatrixXf>& p)
{
    //takes the point (x, y, z) and runs it through the quadratic equation described by the parameters stored in p, returns the value.
    //if (x, y, z) is a point on the ellipsoid described by p then the value returned should be 1 (or very close to it because of floating point precision)

    //Ax^2 + By^2 + Cz^2 + Dxy + Exz + Fyz + Gx + Hy + Iz + J = 0 is the quadratic equation described by p. P is of the form {Abar, bbar, cbar}
    //where Abar is a 3x3 matrix containing parameters A-F, bbar is a vector containing paramters G-I and c is a scalar representing J
    double A = p[0](0, 0);
    double B = p[0](1, 1);
    double C = p[0](2, 2);
    double D = 2 * p[0](0, 1);
    double E = 2 * p[0](0, 2);
    double F = 2 * p[0](1, 2);
    double G = p[1](0, 0);
    double H = p[1](0, 1);
    double I = p[1](0, 2);
    double J = p[2](0, 0);

    return (A * x * x + B * y * y + C * z * z + D * x * y + E * x * z + F * y * z + G * x + H * y + I * z + J);
}
void ellipseBestFit(std::vector<float>& x_data, std::vector<float>& y_data, std::vector<float>& z_data, std::vector<std::vector<double> >& RUV, float* mag_off, float* mag_gain)
{
    //This function takes magnetometer data that has been moved by hard iron offsets so that it's centered on origin

    //First gets a best fit solution S to the set of linear equations D * S = e
    //D is an m x 9 matrix where m is the amount of data points in the set and row i of the matrix is: [xi^2 + yi^2 - 2zi^2, xi^2 - 2yi^2 + zi^2, 4xiyi, 2xizi, 2yizi, xi, yi, zi, 1]
    //e is an m x 1 matrix where m is the amount of data points in the set and row i of the matrix is: [xi^2 + yi^2 + zi^2]
    //S is the initial estimate for best fit ellipse of the form S = [U, V, M, N, P, Q, R, S, T]transpose
    //more info can be found here: https://www.researchgate.net/publication/2239930_An_Algorithm_for_Fitting_an_Ellipsoid_to_Data

    Eigen::MatrixXf D(x_data.size(), 9), UV(x_data.size(), 2); //stores u and v components of data set converted to spherical coordinates
    Eigen::VectorXf e(x_data.size());
    float xi, yi, zi;

    //Set Matrices
    //also create a matrix for all points converted to spherical coordinates which will be used later
    for (int i = 0; i < x_data.size(); i++)
    {
        //get current x, y, z
        xi = x_data[i];
        yi = y_data[i];
        zi = z_data[i];

        D(i, 0) = xi * xi + yi * yi - 2 * zi * zi;
        D(i, 1) = xi * xi - 2 * yi * yi + zi * zi;
        D(i, 2) = 4 * xi * yi;
        D(i, 3) = 2 * xi * zi;
        D(i, 4) = 2 * yi * zi;
        D(i, 5) = xi;
        D(i, 6) = yi;
        D(i, 7) = zi;
        D(i, 8) = 1;

        e(i) = xi * xi + yi * yi + zi * zi; //this is equivalent to r^2, these values will be used to calculate residuals later
        UV(i, 0) = atan2(yi, xi);
        UV(i, 1) = atan2(sqrt(xi * xi + yi * yi), zi);
    }

    //Eigen::VectorXf S = D.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(e);
    Eigen::MatrixXf S = D.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(e);

    //Initial estimate for parameters for Newton-Gaussian algorithm S* = [a, b, c, M, N, P, T, U, V]transpose
    //M, N, P, T, U and V were calculated above and correspond to S[2], S[3], S[4], S[8], S[0] and S[1] resspectively
    //a, b and c are the estimates for the center of the best fit ellipse and are initialized to 0

    Eigen::MatrixXf S_star(9, 1);
    S_star(0, 0) = 0;
    S_star(1, 0) = 0;
    S_star(2, 0) = 0;
    S_star(3, 0) = S(2, 0);
    S_star(4, 0) = S(3, 0);
    S_star(5, 0) = S(4, 0);
    S_star(6, 0) = S(8, 0);
    S_star(7, 0) = S(0, 0);
    S_star(8, 0) = S(1, 0);

    //std::cout << "Initial estimate for Parameters is\n" << S_star << std::endl;

    //Now that original parameters have been set, can start the Newton-Euler algorithm
    //set ending condition
    double error_tolerance = 0.0001;

    //create an array to hold residual values
    Eigen::MatrixXf res(x_data.size(), 1);
    std::vector<double> xyz;
    double error, last_error = 100;

    //Newton-Euler algorithm
    while (true)
    {
        //calculate residuals
        for (int i = 0; i < x_data.size(); i++)
        {
            //double r_squared = calculateRSquared(UV(i, 0), UV(i, 1), S_star);
            //res(i) = r_squared - e(i);

            //xyz = calculateXYZ(UV(i, 0), UV(i, 1), S_star);
            //std::cout << "Calculated Point is [" << xyz[0] << ", " << xyz[1] << ", " << xyz[2] << "]\n" << std::endl;
            //res(i) = sqrt((x_data[i] - xyz[0]) * (x_data[i] - xyz[0]) + (y_data[i] - xyz[1]) * (y_data[i] - xyz[1]) + (z_data[i] - xyz[2]) * (z_data[i] - xyz[2])); //residual is distance between the two points in space
            //res(i, 0) = calculateResidual(x_data[i], y_data[i], z_data[i], xyz[0], xyz[1], xyz[2]);

            //each residual is the squared distance from point [x, y, z] and the nearest location on the ellipsoid
            res(i, 0) = calculateGeometricDistance(x_data[i], y_data[i], z_data[i], S_star);
        }

        //if residuals are low enough then break out of algorithm
        error = residualError(res);
        if ((last_error - error) / last_error <= error_tolerance) //wait for error to converge on a solution, aka, change in error is less than .01% since last iteration
        {
            std::cout << "Best fit ellipsoid has been found! With following parameters b:\n" << std::endl;
            std::cout << S_star << std::endl << std::endl;
            std::cout << "Here's a graph of the best fit ellipsoid vs. the original data (close graph to continue):\n" << std::endl;
            graphCompare(x_data, y_data, z_data, S_star);
            break;
        }
        else
        {
            //std::cout << "Current error difference is " << 100 * (last_error - error) / last_error << " percent." << std::endl;
            last_error = error;
        }

        Eigen::MatrixXf jacobian = createJacobian(S_star, UV, x_data, y_data, z_data); //To Do: I think that all data points need to be looked at again to properly make J matrix

        //std::cout << "Jacobian Matrix is:\n" << jacobian << std::endl;
        updateParameters(S_star, res, jacobian);

        //std::cout << "Paramaters have been changed to \n" << S_star << std::endl;
    }

    //final ellipse parameters are of the form (a, b, c, M, N, P, T, U, V)
    //a, b and c are the center of the best fit ellipsoid from when data was transfered to origin, therefore, a, b anc c need to be added to mag_offset values
    //T is just a function of Earth's Magnetic field (should be roughly equal to the field strength squared)
    //M, N, P, U and V formulate the best fit ellipse matrix A as follows
    //A[3][3] = [[(1 - U - V), -2M,  -N],
    //           [-2M, (1 - U + 2V), -P],
    //           [-N, -P, (1 + 2U - V)]]

    *mag_off += S_star(0, 0); *(mag_off + 1) += S_star(1, 0); *(mag_off + 2) += S_star(2, 0); //updated mag_off

    Eigen::Matrix3f A;
    A(0, 0) = 1 - S_star(7, 0) - S_star(8, 0); A(0, 1) = -2 * S_star(3, 0); A(0, 2) = -S_star(4, 0);
    A(1, 0) = -2 * S_star(3, 0); A(1, 1) = 1 - S_star(7, 0) + 2 * S_star(8, 0); A(1, 2) = -S_star(5, 0);
    A(2, 0) = -S_star(4, 0); A(2, 1) = -S_star(5, 0); A(2, 2) = 1 + 2 * S_star(7, 0) - S_star(8, 0);

    Eigen::EigenSolver<Eigen::Matrix3f> eigensolver(A);
    auto eValues = eigensolver.eigenvalues();
    auto eVectors = eigensolver.eigenvectors();

    Eigen::MatrixXf D_prime(3, 3);
    D_prime(0, 0) = sqrt(eValues(0).real()); D_prime(0, 1) = 0; D_prime(0, 2) = 0;
    D_prime(1, 0) = 0; D_prime(1, 1) = sqrt(eValues(1).real()); D_prime(1, 2) = 0;
    D_prime(2, 0) = 0; D_prime(2, 1) = 0;  D_prime(2, 2) = sqrt(eValues(2).real());

    Eigen::MatrixXf eigenVectors(3, 3);
    eigenVectors(0, 0) = eVectors(0).real(); eigenVectors(0, 1) = eVectors(3).real(); eigenVectors(0, 2) = eVectors(6).real();
    eigenVectors(1, 0) = eVectors(1).real(); eigenVectors(1, 1) = eVectors(4).real(); eigenVectors(1, 2) = eVectors(7).real();
    eigenVectors(2, 0) = eVectors(2).real(); eigenVectors(2, 1) = eVectors(5).real(); eigenVectors(2, 2) = eVectors(8).real();

    Eigen::MatrixXf W_inverse = eigenVectors * D_prime * eigenVectors.inverse(); //this is the matrix needed for soft-iron calibration purposes!

    //std::cout << "Ellipse Matrix is: \n" << A << std::endl;
    std::cout << "Matrix Parameters are:\n" << S_star << std::endl;
    std::cout << "\nSqrt of Matrix is: \n" << W_inverse << std::endl;

    for (int i = 0; i < 9; i++)
    {
        *(mag_gain + i) = W_inverse(i); //TODO: need to double check that mag_gain is appropriately copied from W_inverse
    }
}

//Functions Used for Newton_Gauss best fit method
void convertCartesianToSpherical(std::vector<float>& x, std::vector<float>& y, std::vector<float>& z, Eigen::MatrixXf& uv_values)
{
    //trying to calculate the radius of an ellipsoid in polar coordinates given two angles u and v
    //convert the cartesian coordinates [x, y, z] to the angles u and v which will then be used to calculate an r value later
    for (int i = 0; i < x.size(); i++)
    {
        double u = atan(y[i] / x[i]);
        double v = atan(sqrt(x[i] * x[i] + y[i] * y[i]) / z[i]);
        uv_values(i, 0) = u;
        uv_values(i, 1) = v;
    }
}
std::vector<double> convertSphericalToCartesian(std::vector<double>& spherical)
{
    //takes spherical coordinates of the form [r, u, v] and converts them to cartesian coordinates [x, y, z]
    std::vector<double> cart;
    cart[0] = spherical[0] * cos(spherical[1]) * sin(spherical[2]);
    cart[1] = spherical[0] * sin(spherical[1]) * sin(spherical[2]);
    cart[2] = spherical[0] * cos(spherical[2]);

    return cart;
}
double calculateRSquared(double u, double v, Eigen::MatrixXf& b)
{
    //returns the squared distance from center of ellipse defined by parameters in b to the point on ellipse defined by parametric parameters u and v

    double numerator = b(6, 0);
    double term1 = b(3, 0) * (1 + cos(2 * v)) * sin(2 * u);
    double term2 = b(4, 0) * cos(u) * sin(2 * v);
    double term3 = b(5, 0) * sin(u) * sin(2 * v);
    double term4 = b(7, 0) * (3 * cos(2 * v) - 1) / 2.0;
    double term5 = 3 * b(8, 0) * (cos(2 * u) * cos(2 * v) + cos(2 * u) - cos(2 * v) + 1.0 / 3.0) / 4.0;
    double denominator = 1 - term1 - term2 - term3 - term4 - term5;

    return numerator / denominator;
}
std::vector<double> calculateXYZ(double u, double v, Eigen::MatrixXf& b)
{
    //Ellipsoid function
    //b_vector = [a, b, c, M, N, P, T, U, V]
    //r^2 = T / [1 - M(1 + cos2v)sin2u - Ncosusin2vPsinusin2v - U(3cos2v - 1) / 2 - 3V(cos2ucos2v + cos2u - cos2v + 1/3) / 4]
    //x = a + r * cos(u)cos(v)
    //y = b + r * sin(u)cos(v)
    //z = c + r * sin(v)

    double numerator = b(6, 0);
    double term1 = b(3, 0) * (1 + cos(2 * v)) * sin(2 * u);
    double term2 = b(4, 0) * cos(u) * sin(2 * v);
    double term3 = b(5, 0) * sin(u) * sin(2 * v);
    double term4 = b(7, 0) * (3 * cos(2 * v) - 1) / 2.0;
    double term5 = 3 * b(8, 0) * (cos(2 * u) * cos(2 * v) + cos(2 * u) - cos(2 * v) + 1.0 / 3.0) / 4.0;
    double denominator = 1 - term1 - term2 - term3 - term4 - term5;
    double r_squared = numerator / denominator;

    //TODO: Don't take the square root of r^2, just substitute the r^2 value for r in the below equations so negative values don't throw things off
    //std::cout << "Tansform Point: [" << ruv[0] * cos(ruv[1]) * sin(ruv[2]) << ", " << ruv[0] * sin(ruv[1]) * sin(ruv[2]) << ", " << ruv[0] * cos(ruv[2]) << "]" << std::endl;
    double x = b(0, 0) + sqrt(r_squared) * cos(u) * cos(v);
    double y = b(1, 0) + sqrt(r_squared) * sin(u) * cos(v);
    double z = b(2, 0) + sqrt(r_squared) * sin(v);
    
    //std::cout << "r^2 = " << numerator << " / " << denominator << std::endl;

    return {x, y, z};
}
Eigen::MatrixXf createJacobian(Eigen::MatrixXf b, Eigen::MatrixXf& uv, std::vector<float>& x, std::vector<float>& y, std::vector<float>& z)
{
    Eigen::MatrixXf jc(uv.rows(), b.size());

    for (int i = 0; i < x.size(); i++)
    {
        for (int j = 0; j < b.size(); j++)
        {
            double heyhey = derivative(uv(i, 0), uv(i, 1), x[i], y[i], z[i], b, j); //most likely can remove uv portion from derivative function
            //std::cout << "Partial Derivative returned by derivative function = " << heyhey << std::endl << std::endl;
            jc(i, j) = heyhey;
        }
    }
    return jc;
}
double derivative(double u, double v, double x, double y, double z, Eigen::MatrixXf b, int bIndex)
{
    //gets the derivative of the residual with respect to the parameters
    //x, y and z represent the current point being compared against
    double alpha = .001; //define alpha as some arbitrary small value, test different values to see what works best

    b(bIndex, 0) += alpha;
    //double r1 = calculateRSquared(u, v, b);
    //std::vector<double> xyz1 = calculateXYZ(u, v, b); //change one of the b values slightly and calculate new residual
    double distance1 = calculateGeometricDistance(x, y, z, b);

    b(bIndex, 0) -= (2 * alpha); //checked b + alpha, now check b - alpha
    //double r2 = calculateRSquared(u, v, b);
    //std::vector<double> xyz2 = calculateXYZ(u, v, b);
    double distance2 = calculateGeometricDistance(x, y, z, b);

    //return (r1 - r2) / (2 * alpha);
    //double yo = (calculateResidual(x, y, z, xyz1[0], xyz1[1], xyz1[2]) - calculateResidual(x, y, z, xyz2[0], xyz2[1], xyz2[2])) / (2 * alpha);
    //std::cout << "Current magnetic reading is [" << x << ", " << y << ", " << z << "]" << std::endl;
    //std::cout << "Point 1 is [" << xyz1[0] << ", " << xyz1[1] << ", " << xyz1[2] << "]" << std::endl;
    //std::cout << "Point 2 is [" << xyz2[0] << ", " << xyz2[1] << ", " << xyz2[2] << "]" << std::endl;
    //std::cout << "Partial Derivative calculated by derivative function = " << yo << std::endl;
    //return yo;

    return (distance1 - distance2) / (2 * alpha);
}
double residualError(Eigen::MatrixXf& res)
{
    //Equation for error found here: https://www.investopedia.com/ask/answers/042415/what-difference-between-standard-error-means-and-standard-deviation.asp#:~:text=SEM%20is%20calculated%20by%20taking,variability%20of%20the%20sample%20means.
    double sum_of_squares = 0;
    double average = 0;
    double standard_deviation = 0;

    for (int i = 0; i < res.rows(); i++)
    {
        sum_of_squares += res(i, 0); //add up squared distances
        average += sqrt(res(i, 0)); //add distance to get average distance
    }

    average = average / res.rows();
    //std::cout << "Average = " << average << std::endl;

    for (int i = 0; i < res.rows(); i++)  standard_deviation += (sqrt(res(i, 0)) - average) * (sqrt(res(i, 0)) - average);
    standard_deviation = sqrt(standard_deviation / (res.rows() - 1));

    return (standard_deviation / sqrt(res.rows()));
}
void updateParameters(Eigen::MatrixXf& b, Eigen::MatrixXf& res, Eigen::MatrixXf& J)
{
    //std::cout << "Jacobian Matrix is:\n" << J << std::endl;
    double gamma = 0.01; //TODO: not sure if these is actually needed, makes the process take longer. Mess around with it to see what happens

    Eigen::MatrixXf new_b = (J.transpose() * J).inverse() * J.transpose() * res;
    
    b = b - gamma * new_b;
}
double calculateResidual(double x, double y, double z, double x_new, double y_new, double z_new)
{
    //residual is distance between the two points in space
    //x, y and z represent the original point in space and x_new, y_new and z_new represebt the calculated point in space
    double res = sqrt((x - x_new) * (x - x_new) + (y - y_new) * (y - y_new) + (z - z_new) * (z - z_new));
    return res;
}
double calculateGeometricDistance(double x, double y, double z, Eigen::MatrixXf& b)
{
    //attempts to estimate the distance from a point in space to the surface of the ellipse defined by parameters in b
    //draws a line from the point [x, y, z] to the center of the ellipse [b(0, 0), b(1, 0), b(2, 0)] to figure out the parametric values u and v for the point
    //the value is left as the distance squared as this is need to find the residual error

    //the distance from surface of ellipsoid to the point isn't necessarily the shortest distance so this function could be improved upon at some future point

    double xi = x - b(0, 0);
    double yi = y - b(1, 0);
    double zi = z - b(2, 0);

    //calculate angles and squared distance from ellipsoid center to point
    double u = atan2(yi, xi);
    double v = atan2(sqrt(xi * xi + yi * yi), zi);
    double rsquared = xi * xi + yi * yi + zi * zi;
    
    //return the square of differnce
    double rs = calculateRSquared(u, v, b);
    return (sqrt(rsquared) - sqrt(rs)) * (sqrt(rsquared) - sqrt(rs));
}

//Graphing Functions
void graphCompare(std::vector<float>& x, std::vector<float>& y, std::vector<float>& z, Eigen::MatrixXf& b)
{
    float pi = 3.14159;

    //write all data to file
    ofstream myFile;
    myFile.open("ellipse.dat");
    int i = 0;
    for (float u = -pi; u < pi; u += 2 * pi / x.size())
    {
        for (float v = -pi / 2.0; v < pi / 2.0; v += pi / 25.0)
        {
            //std::cout << double(v) << std::endl;
            std::vector<double> new_point = calculateXYZ((double)u, (double)v, b);
            myFile << x[i] << "    " << y[i] << "    " << z[i] << "    " << new_point[0] << "    " << new_point[1] << "    " << new_point[2] << '\n';
        }
        i++;
    }
    myFile.close();

    std::string function = "splot 'ellipse.dat' using 1:2:3, 'ellipse.dat' using 4:5:6";

    Gnuplot plot;
    plot("set terminal wxt size 1000,1000"); //make the gnuplot window a square so that all axes tics have the same physical distance between them, otherwise will get ellipse instead of sphere
    plot("set xrange [" + to_string(-100) + ":" + to_string(100) + "]");
    plot("set yrange [" + to_string(-100) + ":" + to_string(100) + "]");
    plot("set zrange [" + to_string(-100) + ":" + to_string(100) + "]");
    plot("set ticslevel 0"); //make sure that axes aren't offset at all
    plot(function);
}