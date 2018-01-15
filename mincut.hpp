#ifndef MINCUT_HPP
#define MINCUT_HPP
#include "opencv2/opencv.hpp"
#include <opencv2/core/core.hpp>
#include "graph.h"

class minCut
{
public:
    minCut(cv::Mat _texture);
    minCut(cv::Mat _texture, int rows, int cols);

    void init();

    void update_mask(cv::Point2i t);

    /** Find the best position for the next patch */
    cv::Point2i patch_placement(int nb_pixels);

    void compute_minCut();

private:

    cv::Mat texture;

    cv::Mat old_synthese;
    cv::Mat new_synthese;
    cv::Mat mask; // Useful to know if a pixel has been filled or not with the new texture (0 = not filled ; 1 = filled)

    /** bordure_mask store the top,bottom,left,right borders of the mask*/
    cv::Vec4i border_mask;

    /** Define the max and the min number of overlapping pixels */
    int maxSuperposedPixel;
    int minSuperPosedPixel;

    /** Size of the global image */
    int imRows;
    int imCols;

    /** Size of the patch (input image) */
    int patchRows;
    int patchCols;

    /** Test if a translation is valid or not */
    bool isTranslationValid(cv::Point2i t);
    float compute_translation_cost(cv::Mat &update_synthese, int i, int j, int &nb_pixels);


    /** Every translation possible */
    std::vector<cv::Point2i> translations;
};

#endif // MINCUT_HPP
