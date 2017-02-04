//
//  main.cpp
//  SerialMadlebrotA1
//
//  Created by Scott Obray on 1/12/17.
//  Copyright © 2017 Scott Obray. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <chrono>
#include <thread>
#include "ThreadPool2.hpp"
#include "TSQ.hpp"

auto imageSize = 600;
auto maxPixelVal = 256;
auto minReal = -1.5;
auto maxReal = 0.7;
auto minImag = -1.0;
auto maxImag = 1.0;
//std::vector<int> imageContents (imageSize * imageSize * 4);


struct Color {
    int red = 0;
    int green = 0;
    int blue = 0;
};

std::vector<std::vector<Color>> imageContents(imageSize, std::vector<Color>(imageSize));

//Global struct object to be used for each pixel
Color* color;

//Use to time any function
template<typename F>
auto timeFunc(F f, int numThreads)
{
    auto start = std::chrono::steady_clock::now();
    f(numThreads);
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> diff = end-start;
    return diff;
}
//Algorthim desgined from wikipedia article
//Scale to x
double mapToReal(int x, int imageSize, double minR, double maxR)
{
    double range = maxR - minR;
    return x * (range / imageSize) + minR;
}
//Scale to y
double mapToImaginary(int y, int imageSize, double minI, double maxI)
{
    double range = maxI - minI;
    return y * (range / imageSize) + minI;
}
//Find madelbrot value.....sourced from wikipedia
int findMandelbrot(double cr, double ci, int maxIterations)
{
    int i = 0;
    double zr = 0.0;
    double zi = 0.0;
    while (i < maxIterations && zr * zr + zi * zi < 4.0)
    {
        double temp = zr * zr - zi * zi + cr;
        zi = 2.0 * zr * zi + ci;
        zr = temp;
        i++;
    }
    return i;
}

//Determine rgb color value for each pixel
Color* computeColor(int n, Color* newColor)
{
    newColor->red = (n % 256);
    newColor->green = (n / 147) + 17;
    newColor->blue = (n*n) + 71;
    return newColor;
}

//Generate madelbrot without writing to a file
void generateMadlebrot(int beg, int end)
{

  // for (int j = 0; j < imageSize; j++)
  // {
    double cr = mapToReal(end, imageSize, minReal, maxReal);
    double ci = mapToImaginary(beg, imageSize, minImag, maxImag);
    int n = findMandelbrot(cr, ci, maxPixelVal);

    color = computeColor(n, color);

    imageContents[beg][end] = *color;
  //row by row
  // for (int j = 0; j < imageSize; j++)
  // {
  //   double cr = mapToReal(j, imageSize, minReal, maxReal);
  //   double ci = mapToImaginary(beg, imageSize, minImag, maxImag);
  //   int n = findMandelbrot(cr, ci, maxPixelVal);
  //
  //   color = computeColor(n, color);
  //
  //   imageContents[beg][j] = *color;
  // }

    //int index = beg;
    // for (int i = beg; i < end; i++)
    // {
    //     for (int j = 0; j < imageSize; j++)
    //     {
    //         double cr = mapToReal(j, imageSize, minReal, maxReal);
    //         double ci = mapToImaginary(i, imageSize, minImag, maxImag);
    //         int n = findMandelbrot(cr, ci, maxPixelVal);
    //
    //         color = computeColor(n, color);
    //
    //         imageContents[i][j] = *color;
    //         //std::cout << " 0 ";
    //     }
    // }
    //Even chunks
    // for (int i = beg; i < end; i++)
    // {
    //     for (int j = 0; j < imageSize; j++)
    //     {
    //         double cr = mapToReal(j, imageSize, minReal, maxReal);
    //         double ci = mapToImaginary(i, imageSize, minImag, maxImag);
    //         int n = findMandelbrot(cr, ci, maxPixelVal);
    //
    //         color = computeColor(n, color);
    //
    //         imageContents[i][j] = *color;
    //     }
    // }
}



void generateImage(int numThreads)
{
    ThreadPool2 pool(numThreads);

    //Row by row
    // for (int i = 0; i < imageSize; i++)
    // {
    //   pool.enqueue([=](){generateMadlebrot(i);});
    // }

    //Pixel by pixel
    for (int i = 0; i < imageSize; i++)
    {
      for (int j = 0; j < imageSize; j++)
      {
        pool.enqueue([=](){generateMadlebrot(i, j);});
        //std::cout << "Hello";
      }
    }

    // for (int i = 0; i < 8; i++)
    // {
    //     int beg = (imageSize / 8) * i;
    //     int end = (imageSize / 8) * (i+1);
    //
    //     //std::function<void(void)> task = std::bind(generateMadlebrot, beg, end);
    //
    //     pool.enqueue([=](){generateMadlebrot(beg, end);});

        //pool.enqueue(task);
    // }
}

   //Function calculate average
    double computeAverage(std::vector<double> &duration)
    {
      double mean = 0.0;
      for (auto i = 0; i < (int)duration.size(); i++)
      {
        mean += duration[i];
      }
      mean = mean / duration.size();
      return mean;
    }
    //Function to calculate standard deviation
    double computeSd(std::vector<double> &duration, double mean)
    {
      std::vector<double> differences;
      double sd = 0.0;
      for (int i = 0; i < (int)duration.size(); i++)
      {
        sd += pow(duration[i] - mean, 2);
      }        return sqrt(sd / duration.size());
    }

int main()
{
    color = new Color();
    int numTests = 5;
    std::vector<double> durations;

    int numThreads = 8;

    //generateImage(numThreads);

    //Run tests
    for (int i = 0; i < numTests; i++)
    {
        auto result = timeFunc(generateImage, numThreads);
        durations.push_back(result.count());
    }
    delete color;



    //Output Test Results
    double mean = computeAverage(durations);
    double sd = computeSd(durations, mean);
    std::cout << "Average time: "<< mean <<  "s\n";
    std::cout << "Standard Deviation: " << sd << "s\n";


    //Generate one mandelbrot image
    std::ofstream fout("mandelbrot.ppm");
    fout << "P3" << std::endl;
    fout << imageSize << " " << imageSize << std::endl;
    fout << "256" << std::endl;

    for (int i = 0; i < imageSize; i++)
    {
        for (int j = 0; j < imageSize; j++)
        {
            fout << imageContents[i][j].red << " " << imageContents[i][j].green << " " << imageContents[i][j].blue << " ";
        }
        fout << std::endl;
    }

}
