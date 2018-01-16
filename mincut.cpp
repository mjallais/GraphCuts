#include "mincut.hpp"


minCut::minCut(cv::Mat _texture) : imRows(100), imCols(100), overlapCols(0), overlapRows(0)
{
    texture = _texture;
    patchRows = texture.rows;
    patchCols = texture.cols;

    old_synthese =  cv::Mat::zeros(imRows, imCols, CV_8UC3);
    new_synthese =  cv::Mat::zeros(imRows, imCols, CV_8UC3);
    mask =  cv::Mat::zeros(imRows, imCols, CV_8UC1);
    overlap_zone = cv::Mat::zeros(imRows,imCols,CV_8UC1);


    maxSuperposedPixel = 256;
    minSuperPosedPixel = 256;

    border_mask = cv::Vec4i(0,imRows,0,imCols);
}


minCut::minCut(cv::Mat _texture, int rows, int cols) : overlapCols(0), overlapRows(0)
{
    texture = _texture;
    patchRows = texture.rows;
    patchCols = texture.cols;

    imRows = rows;
    imCols = cols;
    old_synthese =  cv::Mat::zeros(imRows, imCols, CV_8UC3);
    new_synthese =  cv::Mat::zeros(imRows, imCols, CV_8UC3);
    mask =  cv::Mat::zeros(imRows, imCols, CV_8UC1);
    overlap_zone = cv::Mat::zeros(imRows,imCols,CV_8UC1);

    maxSuperposedPixel = 256;
    minSuperPosedPixel = 256;

    border_mask = cv::Vec4i(imRows,0,imCols,0);
}


void minCut::init()
{
    // Copie de l'image originale dans l'image de synthèse que l'on cherche à créer
    cv::Point2i t (floor(imRows/4),floor(imCols/4));
    texture(cv::Range(0,patchRows),cv::Range(0,patchCols)).copyTo(old_synthese(cv::Rect(t.y, t.x, patchCols, patchRows)));
    old_synthese.copyTo(new_synthese);

    update_mask(t);
}


void minCut::update_mask(cv::Point2i t)
{

    for(int u=0; u<patchRows; ++u)
        for(int v=0; v<patchCols; ++v)
        {
            mask.at<uchar>(t.x+u, t.y+v) = 1;
        }

    if(t.x < border_mask[0])
        border_mask[0]=t.x;
    if(t.x+patchRows > border_mask[1])
        border_mask[1]=t.x+patchRows;
    if(t.y < border_mask[2])
        border_mask[2]=t.y;
    if(t.y+patchCols > border_mask[3])
        border_mask[3]=t.y+patchCols;

    std::cout<<"border_mask = "<<border_mask[0]<<" "<<border_mask[1]<<" "<<border_mask[2]<<" "<<border_mask[3]<<std::endl;
}


cv::Point2i minCut::update_overlap_zone(cv::Point2i t)
{
    // 1 = zone d'overlap
    // 2 = appartient au nouveau patch
    // 3 = appartient à l'ancienne synthèse

    overlap_zone = cv::Mat::zeros(imRows,imCols,CV_8UC1);
    cv::Point2i corner;
    overlapRows = 0;
    overlapCols = 0;
    bool first=true;
    int n=0;
    for(int u=0; u<patchRows; ++u)
    {
        for(int v=0; v<patchCols; ++v)
        {
            if(mask.at<uchar>(t.x+u, t.y+v) == 1)
            {
                overlap_zone.at<uchar>(t.x+u, t.y+v) = 1;
                if(first)
                {
                    corner.x = t.x+u;
                    corner.y = t.y+v;
                    first = false;
                }
                if(n == 0)
                    overlapRows+=1;
                n+=1;
            }
        }
        if(n!=0)
            overlapCols=n;
        n=0;
    }
    std::cout<<"update_overlap_zone : overlapCols = "<<overlapCols << " overlapRows = "<<overlapRows<<std::endl;

    // On cherche à définir si les points appartiennent au nouveau patch ou à l'ancienne synthèse
    for(int u=0; u<overlapRows; ++u)
    {
        // colonne de gauche
        if(mask.at<uchar>(corner.x+u, corner.y-1) == 0 || mask.at<uchar>(corner.x+u, corner.y+1) == 0)
        {
            // Un des voisins n'appartenait pas à l'ancienne synthèse -> le point appartient forcément au nouveau patch
            overlap_zone.at<uchar>(corner.x+u,corner.y) = 2;
        }
        else
            overlap_zone.at<uchar>(corner.x+u,corner.y) = 3;

        // colonne de droite
        if(mask.at<uchar>(corner.x+u, corner.y+(overlapCols-1)-1) == 0 || mask.at<uchar>(corner.x+u, corner.y+(overlapCols-1)+1) == 0)
            overlap_zone.at<uchar>(corner.x+u,corner.y+overlapCols) = 2;
        else
            overlap_zone.at<uchar>(corner.x+u,corner.y+overlapCols) = 3;
    }

    for(int v=0; v<overlapCols; ++v)
    {
        // ligne du haut
        if(mask.at<uchar>(corner.x-1, corner.y+v) == 0 || mask.at<uchar>(corner.x+1, corner.y+v) == 0)
            overlap_zone.at<uchar>(corner.x,corner.y+v) = 2;
        else
            overlap_zone.at<uchar>(corner.x,corner.y+v) = 3;

        // ligne du bas
        if(mask.at<uchar>(corner.x+(overlapRows-1)-1, corner.y+v) == 0 || mask.at<uchar>(corner.x+(overlapRows-1)+1, corner.y+v) == 0)
            overlap_zone.at<uchar>(corner.x+overlapRows,corner.y+v) = 2;
        else
            overlap_zone.at<uchar>(corner.x+overlapRows,corner.y+v) = 3;
    }

    return corner;
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


float minCut::compute_translation_cost(cv::Mat &update_synthese, int i, int j, int &nb_pixels)
{
    nb_pixels =0;
    float cost=0.0f;
    if(i >0 && i<imRows-patchRows && j >0 && j<imCols-patchCols)
    {
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
    }
    return cost;
}


cv::Point2i minCut::patch_placement(int &nb_pixels)
{
    float cost_min = 1000.0f;

    cv::Point2i t;
    int min_i =0, max_i=imRows, min_j=0, max_j=imCols;
    if(border_mask[0]-patchRows > 0)
        min_i=border_mask[0]-patchRows ;
    if(border_mask[1]+patchRows < imRows)
        max_i=border_mask[1]+patchRows ;

    if(border_mask[2]-patchCols > 0)
        min_j=border_mask[2]-patchCols ;
    if(border_mask[3]+patchCols < imCols)
        max_j=border_mask[3]+patchCols ;

    std::cout<<"min_i = "<<min_i<< "   max_i = "<<max_i<<"   min_j = "<<min_j<<"   max_j = "<<max_j<<std::endl;
    for(int i=min_i; i<=max_i; ++i)
        for(int j=min_j; j<=max_j; ++j)
        {
            if(isTranslationValid(cv::Point2i(i,j)))
            {
                // Création de notre nouvelle synthese
                cv::Mat update_synthese;
                old_synthese.copyTo(update_synthese);
                texture(cv::Range(0,patchRows),cv::Range(0,patchCols)).copyTo(update_synthese(cv::Rect(j,i,patchCols,patchRows)));
                float cost = compute_translation_cost(update_synthese, i, j, nb_pixels);
                if(cost < cost_min)
                {
                    cost_min=cost;
                    t = cv::Point2i(i,j);
                }
            }
        }
    return t;
}


bool minCut::allImageFilled()
{

}


void minCut::mat_affichage(cv::Mat mat)
{
    for(int u=0; u<imRows; ++u){
        for(int v=0; v<imCols; ++v)
        {
            std::cout<<(int)mat.at<uchar>(u,v)<<" ";
        }
        std::cout<<std::endl;
    }
}


void minCut::compute_minCut()
{
    init();
    int nb_pixels = 0;
    cv::Point2i t = patch_placement(nb_pixels);
    std::cout<<"Apres patch placement t = "<<t.x<<" "<<t.y<<std::endl;

    texture(cv::Range(0,patchRows),cv::Range(0,patchCols)).copyTo(new_synthese(cv::Rect(t.y,t.x,patchCols,patchRows)));

    imshow("Image old", old_synthese); cv::waitKey(0);
    imshow("Image new", new_synthese); cv::waitKey(0);

    std::cout<<"nb_pixels = "<<nb_pixels<<std::endl;
    std::cout<<"overlapCols = "<<overlapCols<<" overlapRows = "<<overlapRows<<std::endl;

    typedef Graph<int,int,int> GraphType;
    GraphType *g = new GraphType(/*estimated # of nodes*/ nb_pixels, /*estimated # of edges*/ (overlapCols-1)*(overlapRows-1));

    g->add_node(nb_pixels);

    cv::Point2i overlap_corner = update_overlap_zone(t);
    std::cout<<"overlapCols = "<<overlapCols<<" overlapRows = "<<overlapRows<<std::endl;

    cv::Mat overlap;
    new_synthese.copyTo(overlap,overlap_zone);
    imshow("overlap", overlap);cv::waitKey(0);

    mat_affichage(overlap_zone);

    int num = 0;
    for(int i=0; i<overlapRows; ++i) // lignes
        for(int j=0; j<overlapCols; ++j) // colonnes
        {
            // Calcul edge à droite et en bas pour chaque point dans les deux sens
            int x_crt = overlap_corner.x + i;
            int y_crt = overlap_corner.y + j;

            if(i<overlapRows-1) // sauf bordure bas - calcul du cout avec le point en dessous et la réciproque
            {
                // liaison avec le point en dessous
                int x_adj = x_crt;
                int y_adj = y_crt + 1;
                float cost_direct = norm(old_synthese.at<cv::Vec3b>(x_crt,y_crt) - new_synthese.at<cv::Vec3b>(x_crt,y_crt)) + norm(old_synthese.at<cv::Vec3b>(x_adj,y_adj) - new_synthese.at<cv::Vec3b>(x_adj,y_adj));
                float cost_indirect = norm(old_synthese.at<cv::Vec3b>(x_adj,y_adj) - new_synthese.at<cv::Vec3b>(x_adj,y_adj)) + norm(old_synthese.at<cv::Vec3b>(x_crt,y_crt) - new_synthese.at<cv::Vec3b>(x_crt,y_crt));
                g->add_edge(num, num+overlapRows, cost_direct, cost_indirect);
            }

            if(j<overlapCols-1) // sauf bordure droite - calcul du cout avec le point à droite et la réciproque
            {
                // liaison avec le point à droite
                int x_adj = x_crt + 1;
                int y_adj = y_crt;
                float cost_direct = norm(old_synthese.at<cv::Vec3b>(x_crt,y_crt) - new_synthese.at<cv::Vec3b>(x_crt,y_crt)) + norm(old_synthese.at<cv::Vec3b>(x_adj,y_adj) - new_synthese.at<cv::Vec3b>(x_adj,y_adj));
                float cost_indirect = norm(old_synthese.at<cv::Vec3b>(x_adj,y_adj) - new_synthese.at<cv::Vec3b>(x_adj,y_adj)) + norm(old_synthese.at<cv::Vec3b>(x_crt,y_crt) - new_synthese.at<cv::Vec3b>(x_crt,y_crt));
                g->add_edge(num, num+1, cost_direct, cost_indirect);
            }

            if(mask.at<uchar>(x_crt,y_crt) == 2) // appartient au nouveau patch
                g->add_tweights( num,   /* capacities old=source new=sink */ 0, 1000 );

            if(mask.at<uchar>(x_crt,y_crt) == 3) // appartient à l'image de base
                g->add_tweights( num,   /* capacities old=source new=sink*/ 1000, 0 );

            ++num;
        }

    std::cout<<"num = "<<num<<std::endl;

    //int flow = g -> maxflow();

//    printf("Flow = %d\n", flow);
//    printf("Minimum cut:\n");

//    num=0;
//    for(int i=0; i<overlapRows; ++i) // lignes
//        for(int j=0; j<overlapCols; ++j) // colonnes
//        {
//            int x_crt = overlap_corner.x + i;
//            int y_crt = overlap_corner.y + j;
//            if(g->what_segment(num) == GraphType::SOURCE)
//            {
//                std::cout<<"old ";
//                new_synthese.at<cv::Vec3b>(x_crt, y_crt) = old_synthese.at<cv::Vec3b>(x_crt, y_crt);
//            }
//            else if(g->what_segment(num) == GraphType::SINK)
//            {
//                std::cout<<"new ";
//            }
//            // Sinon appartient au SINK donc garde la valeur du new_synthese
//            ++num;
//        }

//    delete g;

    //imshow("Image", new_synthese); cv::waitKey(0);

    //std::cout<<"size image : cols = "<<image.cols<<" rows = "<<image.rows<<std::endl;

    return;
}




