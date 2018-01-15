#include "mincut.hpp"

minCut::minCut()
{
    cv::Mat old_synthese =  cv::Mat::zeros(700, 1000, CV_8UC3);
    cv::Mat new_synthese =  cv::Mat::zeros(700, 1000, CV_8UC3);
}
minCut::minCut(unsigned int rows, unsigned int cols)
{
    cv::Mat old_synthese =  cv::Mat::zeros(rows, cols, CV_8UC3);
    cv::Mat new_synthese =  cv::Mat::zeros(rows, cols, CV_8UC3);
}
