#ifndef MINCUT_HPP
#define MINCUT_HPP
#include "opencv2/opencv.hpp"
#include <opencv2/core/core.hpp>

class minCut
{
public:
    minCut();
    minCut(unsigned int rows, unsigned int cols);

private:
    cv::Mat old_synthese;
    cv::Mat new_synthese;
};

#endif // MINCUT_HPP
