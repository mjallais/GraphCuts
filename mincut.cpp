#include "mincut.hpp"

minCut::minCut(cv::Mat _texture) : imRows(500), imCols(500)
{
    texture = _texture;
    patchRows = texture.rows;
    patchCols = texture.cols;

    old_synthese =  cv::Mat::zeros(imRows, imCols, CV_8UC3);
    new_synthese =  cv::Mat::zeros(imRows, imCols, CV_8UC3);
    mask =  cv::Mat::zeros(imRows, imCols, CV_8UC1);

    maxSuperposedPixel = 256;
    minSuperPosedPixel = 256;
}
minCut::minCut(cv::Mat _texture, int rows, int cols)
{
    texture = _texture;
    patchRows = texture.rows;
    patchCols = texture.cols;

    imRows = rows;
    imCols = cols;
    old_synthese =  cv::Mat::zeros(imRows, imCols, CV_8UC3);
    new_synthese =  cv::Mat::zeros(imRows, imCols, CV_8UC3);
    mask =  cv::Mat::zeros(imRows, imCols, CV_8UC1);

    maxSuperposedPixel = 256;
    minSuperPosedPixel = 256;
}


void minCut::update_mask(cv::Point2i t)
{
    for(int u=0; u<patchRows; ++u)
        for(int v=0; v<patchCols; ++v)
        {
            mask.at<uchar>(t.x+u, t.y+v) = 1;
        }
}


// t : translation = position du coin en haut à gauche
bool minCut::isTranslationValid(cv::Point2i t)
{
    int nb_pixels =0;
    // On vérifie que le patch est dans l'image globale
    if(t.x >0 && t.x<imRows-patchRows && t.y >0 && t.y<imCols-patchCols)
    {
        for(int u=0; u<patchRows; ++u)
            for(int v=0; v<patchCols; ++v)
            {
                if(mask.at<uchar>(t.x+u, t.y+v) == 1)
                    nb_pixels+=1;
            }
    }
    if(nb_pixels <= maxSuperposedPixel && nb_pixels >= minSuperPosedPixel)
        return true;
    else
        return false;
}


float minCut::compute_translation_cost(cv::Mat &update_synthese, int i, int j)
{
    int nb_pixels =0;
    float cost=0.0f;
    for(int u=0; u<patchRows; ++u)
        for(int v=0; v<patchCols; ++v)
        {
            if(mask.at<uchar>(i+u, j+v) == 1)
            {
                nb_pixels+=1;
                cv::Vec3b old = old_synthese.at<cv::Vec3b>(i+u,j+v);
                cv::Vec3b update = update_synthese.at<cv::Vec3b>(i+u,j+v);
                cost += std::pow(abs(old[0]-update[0]),2) + std::pow(abs(old[1]-update[1]),2) + std::pow(abs(old[2]-update[2]),2);
            }
        }
    cost = cost/nb_pixels;
    return cost;
}


cv::Point2i minCut::patch_placement()
{
    float cost_min = 1000.0f;
    cv::Point2i t;
    for(int i=0;i<imRows;i++)
        for(int j=0;j<imCols;j++)
        {
            if(isTranslationValid(cv::Point2i(i,j)))
            {
                // Création de notre nouvelle synthese
                cv::Mat update_synthese = old_synthese;
                texture(cv::Range(0,patchRows),cv::Range(0,patchCols)).copyTo(update_synthese(cv::Rect(i,j,patchCols,patchRows)));
                float cost = compute_translation_cost(update_synthese, i, j);
                if(cost < cost_min)
                {
                    cost_min=cost;
                    t = cv::Point2i(i,j);
                }
            }
        }
    return t;
}


