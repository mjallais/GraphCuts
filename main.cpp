#include <stdio.h>
#include "graph.h"
#include "mincut.hpp"
#include "opencv2/opencv.hpp"
#include <opencv2/core/core.hpp>


float norm(cv::Vec3b a)
{
    return std::sqrt(a[0]*a[0] + a[1]*a[1] + a[2]*a[2]);
}




int main()
{
    // READ IMAGE
    cv::Mat image = cv::imread("../GraphCuts/images/nuts_128.jpg");
    if (!image.data)
    {
        std::cout << "Error reading file 1 "<< "image/" << std::endl;
        exit(0);
    }

    minCut mincut(image,image.size[0]*3,image.size[1]*3);

    int method = 0; //0 = entire 1=random
    cv::Mat result = mincut.compute_minCut(method);

    cv::imwrite("../GraphCuts/résultats/test.jpg",result);
    return 0;
}
