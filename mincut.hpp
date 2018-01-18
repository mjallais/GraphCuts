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
    /** Update the mask of the image */
    void update_mask(cv::Point2i t);

    /** Update the zone where the Image and the new patch met */
    cv::Point2i update_overlap_zone(cv::Point2i t);

    void update_seams(cv::Point2i corner, cv::Mat mask_seam, int index_patch);

    void update_init_value_seams(cv::Point2i corner);

    /** Find the best position for the next patch */
    cv::Point2i random_patch_placement(int &nb_pixels);
    cv::Point2i entire_patch_matching_placement(int &nb_pixels);

    float compute_cost_edge(int x_crt, int y_crt, int x_adj, int y_adj, cv::Mat A, cv::Mat B);

    /** Debug fonction to show an matrix */
    void mat_affichage(cv::Mat mat);
    void mat_affichage_vec(cv::Mat mat); // matrix composed of cv::Vec3f

    /** Test is we have filled all the image or not */
    bool allImageFilled();

    /** Run fonction that execute the algorithm */
    void compute_minCut();

private:
    /** Matrix of the texture that we want to reproduce */
    cv::Mat texture;
    /** Matrix which stores the avancement of the algorithm */
    cv::Mat old_synthese;
    cv::Mat new_synthese;

    /** Is a mask of old_synthess */
    cv::Mat mask; // Useful to know if a pixel has been filled or not with the new texture (0 = not filled ; 1 = filled)

    /** Mask of the overlap zone where the image and the new texture met*/
    cv::Mat overlap_zone;

    /** Contain the seams of previous cuts and their cost 1: patch the pixel belongs to, 2: point with which we compute the cost, 3: cost*/
    cv::Mat seams;

    /** Contain the initial position in the pach of the seams (from their original patch)*/
    cv::Mat init_value_seams;

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


    /** Size of the overlap zone */
    int overlapRows;
    int overlapCols;


    /** Test if a translation is valid or not */
    bool isTranslationValid(cv::Point2i t);
    float compute_translation_cost(cv::Mat &update_synthese, int i, int j, int &nb_pixels);


    /** Every translation possible */
    std::vector<cv::Point2i> translations;
};

#endif // MINCUT_HPP
