#include "mincut.hpp"
#include <time.h>


minCut::minCut(cv::Mat _texture) : imRows(100), imCols(100), overlapCols(0), overlapRows(0)
{
    texture = _texture;
    patchRows = texture.rows;
    patchCols = texture.cols;

    old_synthese =  cv::Mat::zeros(imRows, imCols, CV_8UC3);
    new_synthese =  cv::Mat::zeros(imRows, imCols, CV_8UC3);
    mask =  cv::Mat::zeros(imRows, imCols, CV_8UC1);
    overlap_zone = cv::Mat::zeros(imRows,imCols,CV_8UC1);
    seams = cv::Mat::zeros(imRows,imCols,CV_8UC2);
    init_value_seams = cv::Mat(imRows, imCols, CV_8UC2, cv::Scalar(255,255));

    maxSuperposedPixel = 256;
    minSuperPosedPixel = 150;

    border_mask = cv::Vec4i(imRows,0,imCols,0);
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
    seams = cv::Mat::zeros(imRows,imCols,CV_8UC2);
    init_value_seams = cv::Mat(imRows, imCols, CV_8UC2, cv::Scalar(255,255));

    maxSuperposedPixel = 256;
    minSuperPosedPixel = 10;

    border_mask = cv::Vec4i(imRows,0,imCols,0);
}


void minCut::init()
{
    // Copie de l'image originale dans l'image de synthèse que l'on cherche à créer
    cv::Point2i t (floor(imRows/5),floor(imCols/5));
    texture(cv::Range(0,patchRows),cv::Range(0,patchCols)).copyTo(old_synthese(cv::Rect(t.y, t.x, patchCols, patchRows)));
    old_synthese.copyTo(new_synthese);

    update_mask(t);
    update_init_value_seams(t,t, cv::Mat::ones(patchRows,patchCols,CV_8UC1));
    //    std::cout<<"init_value_seams à l'initialisation"<<std::endl;
    //    for(int u=-1; u<=patchRows; ++u){
    //        for(int v=-1; v<=patchCols; ++v)
    //        {
    //            //std::cout<<"("<<(int)init_value_seams.at<cv::Vec2b>(overlap_corner.x+u,overlap_corner.y+v)[0]<<","<<(int)init_value_seams.at<cv::Vec2b>(overlap_corner.x+u,overlap_corner.y+v)[1]<<") ";
    //            std::cout<<(int)init_value_seams.at<cv::Vec2b>(t.x+u,t.y+v)[0]<<" ";
    //        }
    //        std::cout<<std::endl;
    //    }
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
//    std::cout<<"mask :"<<std::endl;
//    for(int u=-1; u<=patchRows; ++u)
//    {
//        for(int v=-1; v<=patchCols; ++v)
//        {
//            std::cout<<(int)mask.at<uchar>(t.x+u, t.y+v);
//        }
//        std::cout<<std::endl;
//    }

}

int minCut::num_neigbors_in_overlap(cv::Point2i p)
{
    int num_neigh = 0;
    for(int i=-1;i<=1;++i)
        for(int j=-1;j<=1;++j)
        {
            if(p.x+i>=0 && p.x+i<imRows && p.y+j>=0 && p.y+j<imCols) //if in image
            {
                if(overlap_zone.at<uchar>(p.x+i,p.y+j) == 0)
                    num_neigh++;
            }
        }
    return num_neigh;
}

int minCut::num_neigbors_in_mask(cv::Point2i p)
{
    int num_neigh = 0;
    for(int i=-1;i<=1;++i)
        for(int j=-1;j<=1;++j)
        {
            if(p.x+i>=0 && p.x+i<imRows && p.y+j>=0 && p.y+j<imCols) //if in image
            {
                if(mask.at<uchar>(p.x+i,p.y+j) == 0)
                    num_neigh++;
            }
        }
    return num_neigh;
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
    std::cout<<"patchRows = "<< patchRows<<" patchCols = "<<patchCols<<std::endl;
    std::cout<<"update_overlpa_zone : t = "<<t.x<<" "<<t.y<<std::endl;

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
        if(n!=0 && n>overlapCols) //Seul truc pas evident
            overlapCols=n;
        n=0;
    }
    for(int u=corner.x-1; u<=corner.x+overlapRows; ++u)
        for(int v=corner.y-1; v<=corner.y+overlapCols; ++v)
        {
            if(overlap_zone.at<uchar>(u,v) == 1) //In overlap between patch
            {
                if(num_neigbors_in_overlap(cv::Point2i(u,v)) >= 1) //On the border of the overlap
                {
//                    std::cout << "on the border" << std::endl;
                    if(num_neigbors_in_mask(cv::Point2i(u,v))>=1) //Bordure with new patch
                    {
//                        std::cout << "on new border " << std::endl;
                        overlap_zone.at<uchar>(u,v) = 2;
                    }
                    else //border with old patch
                    {
//                        std::cout << "on old border" << std::endl;
                        overlap_zone.at<uchar>(u,v) = 3;
                    }
                }


            }
        }
/*

    //    bool limits = false; // on veut que les pixels source/sink soient en haut/bas OU gauche/droite
    //    // On cherche à définir si les points appartiennent au nouveau patch ou à l'ancienne synthèse

    //    for(int u=0; u<overlapRows; ++u)
    //        for(int v=0; v<overlapCols; ++v)
    //        {
    //            if(corner.y-1 >= t.y && corner.y-1 <= t.y+patchCols-1) // la colonne à droite de la colonne la plus à gauche de notre overlap se trouve dans le patch -> appartient au patch
    //            {
    //                if(mask.at<uchar>(corner.x+u, corner.y) == 1)
    //                {
    //                    overlap_zone.at<uchar>(corner.x+u,corner.y) = 2;
    //                    limits = true;
    //                }
    //            }
    //            else if(mask.at<uchar>(corner.x+u, corner.y-1) == 1) // si cette colonne est à coté d'une colonne appartenant au mask -> appartient à l'ancienne synthèse
    //            {
    //                if(mask.at<uchar>(corner.x+u, corner.y) == 1)
    //                {
    //                    overlap_zone.at<uchar>(corner.x+u,corner.y) = 3;
    //                    limits = true;
    //                }
    //            }
    //            //sinon appartient à ni l'un ni l'autre

    //            // colonne de droite = corner.y+(overlapCols-1)
    //            if(corner.y+(overlapCols-1)+1 >= t.y && corner.y+overlapCols <= t.y+patchCols-1) // appartient au patch
    //            {
    //                if(mask.at<uchar>(corner.x+u, corner.y+overlapCols-1) == 1)
    //                {
    //                    overlap_zone.at<uchar>(corner.x+u,corner.y+overlapCols-1) = 2;
    //                }
    //            }
    //            else if(mask.at<uchar>(corner.x+u, corner.y+overlapCols) == 1)
    //            {
    //                if(mask.at<uchar>(corner.x+u, corner.y+overlapCols-1) == 1)
    //                {
    //                    overlap_zone.at<uchar>(corner.x+u,corner.y+overlapCols-1) = 3;
    //                }
    //            }

    //            if(mask.at<uchar>(corner.x, corner.y+v) == 0)
    //            {
    //                // ligne du haut = corner.x
    //                if(corner.x-1 >= t.x && corner.x-1 < t.x+patchRows)
    //                    overlap_zone.at<uchar>(corner.x,corner.y+v) = 2;
    //                else if(mask.at<uchar>(corner.x-1, corner.y+v) == 1)
    //                    overlap_zone.at<uchar>(corner.x,corner.y+v) = 3;

    //                // ligne du bas = corner.x+(overlapRows-1)
    //                if(corner.x+(overlapRows-1)+1 >= t.x && corner.x+overlapRows < t.x+patchRows-1)
    //                    overlap_zone.at<uchar>(corner.x+overlapRows-1,corner.y+v) = 2;
    //                else if(mask.at<uchar>(corner.x+overlapRows, corner.y+v) == 1)
    //                    overlap_zone.at<uchar>(corner.x+overlapRows-1,corner.y+v) = 3;
    //            }
    //        }


    //    for(int u=0; u<overlapRows; ++u)
    //    {
    //        // colonne de gauche = corner.y
    //        if(corner.y-1 >= t.y && corner.y-1 <= t.y+patchCols-1) // la colonne à droite de la colonne la plus à gauche de notre overlap se trouve dans le patch -> appartient au patch
    //        {
    //            if(mask.at<uchar>(corner.x+u, corner.y) == 0)
    //            {
    //                overlap_zone.at<uchar>(corner.x+u,corner.y) = 2;
    //                limits = true;
    //            }
    //        }
    //        else if(mask.at<uchar>(corner.x+u, corner.y-1) == 1) // si cette colonne est à coté d'une colonne appartenant au mask -> appartient à l'ancienne synthèse
    //        {
    //            if(mask.at<uchar>(corner.x+u, corner.y) <= 1)
    //            {
    //                overlap_zone.at<uchar>(corner.x+u,corner.y) = 3;
    //                limits = true;
    //            }
    //        }
    //        //sinon appartient à ni l'un ni l'autre

    //        // colonne de droite = corner.y+(overlapCols-1)
    //        if(corner.y+(overlapCols-1)+1 >= t.y && corner.y+overlapCols <= t.y+patchCols-1) // appartient au patch
    //        {
    //            if(mask.at<uchar>(corner.x+u, corner.y+overlapCols-1) <= 1)
    //            {
    //                overlap_zone.at<uchar>(corner.x+u,corner.y+overlapCols-1) = 2;
    //            }
    //        }
    //        else if(mask.at<uchar>(corner.x+u, corner.y+overlapCols) >= 1)
    //        {
    //            if(mask.at<uchar>(corner.x+u, corner.y+overlapCols-1) <= 1)
    //            {
    //                overlap_zone.at<uchar>(corner.x+u,corner.y+overlapCols-1) = 3;
    //            }
    //        }
    //    }


    //    for(int v=0; v<overlapCols; ++v)
    //    {
    //        if(mask.at<uchar>(corner.x, corner.y+v) == 0)
    //        {
    //            // ligne du haut = corner.x
    //            if(corner.x-1 >= t.x && corner.x-1 < t.x+patchRows)
    //                overlap_zone.at<uchar>(corner.x,corner.y+v) = 2;
    //            else if(mask.at<uchar>(corner.x-1, corner.y+v) >= 1)
    //                overlap_zone.at<uchar>(corner.x,corner.y+v) = 3;

    //            // ligne du bas = corner.x+(overlapRows-1)
    //            if(corner.x+(overlapRows-1)+1 >= t.x && corner.x+overlapRows < t.x+patchRows-1)
    //                overlap_zone.at<uchar>(corner.x+overlapRows-1,corner.y+v) = 2;
    //            else if(mask.at<uchar>(corner.x+overlapRows, corner.y+v) >= 1)
    //                overlap_zone.at<uchar>(corner.x+overlapRows-1,corner.y+v) = 3;
    //        }
    //    }
*/
    return corner;
}


void minCut::update_seams(cv::Point2i corner, cv::Mat mask_seam, int index_patch)
// mask_seam = 0 si old et 1 si new - fait la taille du patch (uchar)
// index_patch = numéro du nouveau patch
// seams : 1: numéro du patch; 2: pixel avec lequel on calcul le cout (uchar)
// pixel avec lequel on calcul le cout : 1=en haut, 2=en bas, 3=à gauche, 4=à droite
// possibilité d'optimiser
{
    bool found = false;
    for(int i=0; i<overlapRows; ++i)
    {
        for(int j=0; j<overlapCols; ++j)
        {
            // On enlève l'ancienne barrière, déja prise en compte pour le calcul de cut précédent

            seams.at<cv::Vec2b>(corner.x+i,corner.y+j) = cv::Vec2b(0,0);

            found = false;
            //Parcours les 4 voisins pour voir si frontière (un 1 et un 0 à coté)
            int val_mask = mask_seam.at<uchar>(i,j);
            // faire conditions haut/bas
//            if(i>0)
//            {
//                if(mask_seam.at<uchar>(i-1,j) != val_mask) //haut
//                {
//                    seams.at<cv::Vec2b>(corner.x+i, corner.y+j)[1] = 1;
//                    if(val_mask == 1) // nouveau patch donc pas encore présent dans seams
//                        seams.at<cv::Vec2b>(corner.x+i, corner.y+j)[0] = index_patch;
//                    //else garde le meme index, mais cout change car la frontiere a évoluée
//                    else if (val_mask == 0 && seams.at<cv::Vec2b>(corner.x+i, corner.y+j)[0] == 0)
//                        seams.at<cv::Vec2b>(corner.x+i, corner.y+j)[0] = 1;
//                    found = true;
//                }
//            }
            if(i<mask_seam.size[0]-1 && found == false)
            {
                if(mask_seam.at<uchar>(i+1,j) != val_mask && mask_seam.at<uchar>(i+1,j) != 0) //bas
                {
                    seams.at<cv::Vec2b>(corner.x+i, corner.y+j)[1] = 2;
                    if(val_mask == 2)
                        seams.at<cv::Vec2b>(corner.x+i, corner.y+j)[0] = index_patch;
                    else if (val_mask == 1 && seams.at<cv::Vec2b>(corner.x+i, corner.y+j)[0] == 0)
                        seams.at<cv::Vec2b>(corner.x+i, corner.y+j)[0] = 1;
                    found = true;
                }
            }
//            if(j>0 && found == false)
//            {
//                if(mask_seam.at<uchar>(i,j-1) != val_mask) //gauche
//                {
//                    seams.at<cv::Vec2b>(corner.x+i, corner.y+j)[1] = 3;
//                    if(val_mask == 1)
//                        seams.at<cv::Vec2b>(corner.x+i, corner.y+j)[0] = index_patch;
//                    else if (val_mask == 0 && seams.at<cv::Vec2b>(corner.x+i, corner.y+j)[0] == 0)
//                        seams.at<cv::Vec2b>(corner.x+i, corner.y+j)[0] = 1;
//                    found = true;
//                }
//            }
            if(j<mask_seam.size[1]-1 && found == false)
            {
                if(mask_seam.at<uchar>(i,j+1) != val_mask && mask_seam.at<uchar>(i,j+1) != 0) //droite
                {
                    seams.at<cv::Vec2b>(corner.x+i, corner.y+j)[1] = 4;
                    if(val_mask == 2)
                        seams.at<cv::Vec2b>(corner.x+i, corner.y+j)[0] = index_patch;
                    else if (val_mask == 1 && seams.at<cv::Vec2b>(corner.x+i, corner.y+j)[0] == 0)
                        seams.at<cv::Vec2b>(corner.x+i, corner.y+j)[0] = 1;
                    found = true;
                }
            }
        }
    }
}


void minCut::update_init_value_seams(cv::Point2i corner_overlap, cv::Point2i corner, cv::Mat mask_seam)
{
    for(int i=0; i<patchRows; ++i)
        for(int j=0; j<patchCols; ++j)
        {
            int x_crt=corner.x+i;
            int y_crt=corner.y+j;
            if(x_crt >= corner_overlap.x && x_crt < corner_overlap.x+overlapRows && y_crt >= corner_overlap.y && y_crt < corner_overlap.y+overlapCols)
            {
                // On se trouve sur la zone d'overlap
                if(mask_seam.at<uchar>(x_crt-corner_overlap.x,y_crt-corner_overlap.y) == 2) //Si fait parti du nouveau patch on update ses valeurs
                {
                    init_value_seams.at<cv::Vec2b>(x_crt, y_crt) = cv::Vec2b(i,j);
                }
            }
            else
                init_value_seams.at<cv::Vec2b>(x_crt, y_crt) = cv::Vec2b(i,j);
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


float minCut::compute_translation_cost(cv::Mat &update_synthese, int i, int j, int &nb_pixels)
{
    nb_pixels =0;
    float cost=99999.0f;
    if(i >=0 && i<imRows-patchRows && j >=0 && j<imCols-patchCols)
    {
        for(int u=0; u<patchRows; ++u)
        {
            for(int v=0; v<patchCols; ++v)
            {
//                std::cout << (int)mask.at<uchar>(i+u, j+v)  ;
                if(mask.at<uchar>(i+u, j+v) == 1)
                {
                    nb_pixels+=1;
                    cv::Vec3b old = old_synthese.at<cv::Vec3b>(i+u,j+v);
                    cv::Vec3b update = update_synthese.at<cv::Vec3b>(i+u,j+v);
                    cost += (std::pow(abs(old[0]-update[0]),2) + std::pow(abs(old[1]-update[1]),2) + std::pow(abs(old[2]-update[2]),2))/3;
                }
            }
//            std::cout << std::endl;
        }
        cost = cost/nb_pixels;
    }
    return cost;
}


cv::Point2i minCut::random_patch_placement(int &nb_pixels)
{
    int i, j;
    do
    {
        i = rand() % imRows;
        j = rand() % imCols;
    } while(!isTranslationValid(cv::Point2i(i,j)));

    return cv::Point2i(i,j);
}



cv::Point2i minCut::entire_patch_matching_placement(int &nb_pixels)
{
    float cost_min = 99999.0f;

    cv::Point2i t(999,999);
    int min_i =0, max_i=imRows, min_j=0, max_j=imCols;
    if(border_mask[0]-patchRows > 0)
        min_i=border_mask[0]-patchRows ;
    if(border_mask[1]+patchRows < imRows)
        max_i=border_mask[1]+patchRows ;

    if(border_mask[2]-patchCols > 0)
        min_j=border_mask[2]-patchCols ;
    if(border_mask[3]+patchCols < imCols)
        max_j=border_mask[3]+patchCols ;

    int n=0;

    //std::cout<<"min_i = "<<min_i<< "   max_i = "<<max_i<<"   min_j = "<<min_j<<"   max_j = "<<max_j<<std::endl;
    for(int i=min_i; i<=max_i; ++i)
        for(int j=min_j; j<=max_j; ++j)
        {
            if(isTranslationValid(cv::Point2i(i,j)))
            {
                // Création de notre nouvelle synthese
                cv::Mat update_synthese;
                old_synthese.copyTo(update_synthese);
                texture(cv::Range(0,patchRows),cv::Range(0,patchCols)).copyTo(update_synthese(cv::Rect(j,i,patchCols,patchRows)));
                float cost = compute_translation_cost(update_synthese, i, j, n);
                //std::cout<<"cost = "<<cost<<std::endl;
                if(cost < cost_min)
                {
                    nb_pixels = n;
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
    for(int u=0; u<overlapRows; ++u){
        for(int v=0; v<overlapCols; ++v)
        {
            std::cout<<(int)mat.at<uchar>(u,v)<<" ";
        }
        std::cout<<std::endl;
    }
}


void minCut::mat_affichage_vec(cv::Mat mat)
{
    for(int u=0; u<mat.size[0]; ++u){
        for(int v=0; v<mat.size[1]; ++v)
        {
            std::cout<<mat.at<cv::Vec3f>(u,v)[0];
        }
        std::cout<<std::endl;
    }
}


float minCut::compute_cost_edge(int x_crt, int y_crt, int x_adj, int y_adj, cv::Mat A, cv::Mat B)
{
    cv::Vec3b new_crt = B.at<cv::Vec3b>(x_crt,y_crt);
    cv::Vec3b old_crt = A.at<cv::Vec3b>(x_crt,y_crt);
    cv::Vec3b new_adj = B.at<cv::Vec3b>(x_adj,y_adj);
    cv::Vec3b old_adj = A.at<cv::Vec3b>(x_adj,y_adj);
    float r = abs(old_crt[0] - new_crt[0]) + abs(old_adj[0] - new_adj[0]);
    float g = abs(old_crt[1] - new_crt[1]) + abs(old_adj[1] - new_adj[1]);
    float b = abs(old_crt[2] - new_crt[2]) + abs(old_adj[2] - new_adj[2]);
    return (r+g+b)/3;
}


void minCut::compute_minCut()
{
    srand (time(NULL));

    std::cout<<"initialisation"<<std::endl;
    init();

    typedef Graph<int,int,int> GraphType;
    GraphType *g = new GraphType(/*estimated # of nodes*/ /*nb_pixels*/ maxSuperposedPixel, /*estimated # of edges*/ (overlapCols-1)*(overlapRows-1));

    for(int iter=1; iter<25000  ; ++iter)
    {
        std::cout<<"iteration "<<iter<<std::endl;

        imshow("old", old_synthese); cv::waitKey(0);
        cv::namedWindow("new",CV_WINDOW_NORMAL);
        imshow("new", new_synthese); cv::waitKey(0);

        int nb_pixels = 0;
        cv::Point2i t = entire_patch_matching_placement(nb_pixels);
        if(t == cv::Point2i(999,999))
        {
            std::cout<<"Plus de translation qui vérifie le nombre requis de pixels superposés. Fin du programme."<<std::endl;
            exit(0);
        }
        std::cout<<"t = "<<t.x<<" "<<t.y<<std::endl;

        texture(cv::Range(0,patchRows),cv::Range(0,patchCols)).copyTo(new_synthese(cv::Rect(t.y,t.x,patchCols,patchRows)));
        imshow("new", new_synthese); cv::waitKey(0);

        cv::Point2i overlap_corner = update_overlap_zone(t);
//        std::cout<<"overlap_zone"<<std::endl;
//        for(int u=-1; u<=overlapRows; ++u)
//        {
//            for(int v=-1; v<=overlapCols; ++v)
//            {
//                std::cout<<(int)overlap_zone.at<uchar>(overlap_corner.x+u,overlap_corner.y+v);
//            }
//            std::cout<<std::endl;
//        }

        //        std::cout<<"init_value_seams"<<std::endl;
        //        for(int u=0; u<imRows; ++u)
        //        {
        //            for(int v=0; v<imCols; ++v)
        //            {
        //                if(init_value_seams.at<cv::Vec2b>(u,v) != cv::Vec2b(255,255))
        //                    std::cout<<(int)init_value_seams.at<cv::Vec2b>(u,v)[0]<<" ";
        //                else
        //                    std::cout<<".";
        //                //std::cout<<"("<<(int)init_value_seams.at<cv::Vec2b>(overlap_corner.x+u,overlap_corner.y+v)[0]<<","<<(int)init_value_seams.at<cv::Vec2b>(overlap_corner.x+u,overlap_corner.y+v)[1]<<") ";
        //            }
        //            std::cout<<std::endl;
        //        }

//        std::cout<<"seams[0]"<<std::endl;
//        for(int u=-1; u<=overlapRows; ++u){
//            for(int v=-1; v<=overlapCols; ++v)
//            {
//                std::cout<<(int)seams.at<cv::Vec2b>(overlap_corner.x+u,overlap_corner.y+v)[0];
//            }
//            std::cout<<std::endl;
//        }
//        std::cout<<"seams[1]"<<std::endl;
//        for(int u=-1; u<=overlapRows; ++u){
//            for(int v=-1; v<=overlapCols; ++v)
//            {
//                std::cout<<(int)seams.at<cv::Vec2b>(overlap_corner.x+u,overlap_corner.y+v)[1];
//            }
//            std::cout<<std::endl;
//        }

        cv::Mat overlap;
        new_synthese.copyTo(overlap,overlap_zone);
        imshow("overlap", overlap);cv::waitKey(0);

        // Changement de condition : on se balade sur tout l'overlap et on regarde où ca se superpose
        //        if(nb_pixels != overlapCols*overlapRows)
        //        {
        //            std::cout<<"Erreur : nb_pixels != overlapCols*overlapRows"<<std::endl;
        //            exit(0);
        //        }

//        std::cout<<"avant reset"<<std::endl;
        g->reset();
//        std::cout<<"apres reset"<<std::endl;
        g->add_node(nb_pixels);
//        std::cout<<"nombre de seams ajoutés = "<<nb_pixels<<" dans le graph : "<<g->get_node_num()<<std::endl;

        // mask_seam = 1 valeur ; 1 si old et 2 si new (0 si overlap pas rectangulaire)
        cv::Mat mask_seam = cv::Mat::zeros(overlapRows, overlapCols, CV_8UC1);

        int num = 0;
        int seam_supp = 0;

        cv::Mat mat_num = cv::Mat::zeros(overlapRows, overlapCols, CV_8UC1);
        for(int i=0; i<overlapRows; ++i) // lignes
            for(int j=0; j<overlapCols; ++j) // colonnes
            {
                if(mask.at<uchar>(overlap_corner.x +i,overlap_corner.y +j)==1)
                {
                    mat_num.at<uchar>(i,j)=num;
                    num+=1;
                }
            }
//        cv::imshow("mat_num",mat_num); cv::waitKey(0);
        //mat_affichage(mat_num);
//        for(int i=0; )
        num=0;


        for(int i=0; i<overlapRows; ++i) // lignes
            for(int j=0; j<overlapCols; ++j) // colonnes
            {
                // Calcul edge à droite et en bas pour chaque point dans les deux sens
                int x_crt = overlap_corner.x + i;
                int y_crt = overlap_corner.y + j;

                bool bas = false;
                bool droite = false;

                if(mask.at<uchar>(x_crt,y_crt)==1)
                {
                    // On regarde si une ancienne bordure se trouve à cet endroit là
                    if(seams.at<cv::Vec2b>(x_crt, y_crt)[0] != 0)
                    {
                        //std::cout<<"(i, j) = "<<i<<" "<<j<<std::endl;
                        if(seams.at<cv::Vec2b>(x_crt, y_crt)[1] == 2) // bas
                        {
                            //std::cout << "COUCOU BAS" <<i << " " << j << std::endl;
                            if(overlap_zone.at<uchar>(x_crt,y_crt) == 1 && mask.at<uchar>(x_crt+1,y_crt)==1) // Pour éviter de rajouter un seam sur la bordure
                            {
                                bas = true;

                                // Rajoute un noeud dans le graphe
                                seam_supp ++;
                                g->add_node();

                                cv::Vec2b s_As = init_value_seams.at<cv::Vec2b>(x_crt, y_crt);
                                cv::Vec2b t_As = s_As + cv::Vec2b(1,0);
                                cv::Vec2b t_At = init_value_seams.at<cv::Vec2b>(x_crt+1,y_crt);
                                cv::Vec2b s_At = t_At - cv::Vec2b(1,0);
                                float color1, color2, color3, cost;
                                //std::cout<<"bas : s_As = "<<s_As<<"   t_As = "<<t_As<<"   t_At = "<<t_At<<"   s_At = "<<s_At<<std::endl;

                                // avec B M(1, 4, A1, A4 )
                                color1 = abs(texture.at<cv::Vec3b>(s_As[0], s_As[1])[0] - texture.at<cv::Vec3b>(s_At[0], s_At[1])[0]) + abs(texture.at<cv::Vec3b>(t_As[0], t_As[1])[0] - texture.at<cv::Vec3b>(t_At[0], t_At[1])[0]);
                                color2 = abs(texture.at<cv::Vec3b>(s_As[0], s_As[1])[1] - texture.at<cv::Vec3b>(s_At[0], s_At[1])[1]) + abs(texture.at<cv::Vec3b>(t_As[0], t_As[1])[1] - texture.at<cv::Vec3b>(t_At[0], t_At[1])[1]);
                                color3 = abs(texture.at<cv::Vec3b>(s_As[0], s_As[1])[2] - texture.at<cv::Vec3b>(s_At[0], s_At[1])[2]) + abs(texture.at<cv::Vec3b>(t_As[0], t_As[1])[2] - texture.at<cv::Vec3b>(t_At[0], t_At[1])[2]);
                                cost = (color1+color2+color3)/3;
                                //std::cout<<"cost avec B = "<<cost<<std::endl;
                                g->add_tweights(nb_pixels-1+seam_supp, 0, cost);

                                // entre le point courant et le seam supplémentaire M(1, 4, A1 , B)
                                color1 = abs(texture.at<cv::Vec3b>(s_As[0], s_As[1])[0] - new_synthese.at<cv::Vec3b>(x_crt, y_crt)[0]) + abs(texture.at<cv::Vec3b>(t_As[0], t_As[1])[0] - new_synthese.at<cv::Vec3b>(x_crt+1, y_crt)[0]);
                                color2 = abs(texture.at<cv::Vec3b>(s_As[0], s_As[1])[1] - new_synthese.at<cv::Vec3b>(x_crt, y_crt)[1]) + abs(texture.at<cv::Vec3b>(t_As[0], t_As[1])[1] - new_synthese.at<cv::Vec3b>(x_crt+1, y_crt)[1]);
                                color3 = abs(texture.at<cv::Vec3b>(s_As[0], s_As[1])[2] - new_synthese.at<cv::Vec3b>(x_crt, y_crt)[2]) + abs(texture.at<cv::Vec3b>(t_As[0], t_As[1])[2] - new_synthese.at<cv::Vec3b>(x_crt+1, y_crt)[2]);
                                cost = (color1+color2+color3)/3;
                                //std::cout<<"cost entre 1 et seam = "<<cost<<std::endl;
                                g->add_edge(mat_num.at<uchar>(i,j), nb_pixels-1+seam_supp, cost, cost);
//                                g->add_edge(num, nb_pixels-1+seam_supp, cost, cost);

                                // entre le seam supplémentaire et le point de l'autre coté de la bordure M(1, 4, B, A4 )
                                color1 = abs(new_synthese.at<cv::Vec3b>(x_crt, y_crt)[0] - texture.at<cv::Vec3b>(s_At[0], s_At[1])[0]) + abs(new_synthese.at<cv::Vec3b>(x_crt+1, y_crt)[0] - texture.at<cv::Vec3b>(t_At[0], t_At[1])[0]);
                                color2 = abs(new_synthese.at<cv::Vec3b>(x_crt, y_crt)[1] - texture.at<cv::Vec3b>(s_At[0], s_At[1])[1]) + abs(new_synthese.at<cv::Vec3b>(x_crt+1, y_crt)[1] - texture.at<cv::Vec3b>(t_At[0], t_At[1])[1]);
                                color3 = abs(new_synthese.at<cv::Vec3b>(x_crt, y_crt)[2] - texture.at<cv::Vec3b>(s_At[0], s_At[1])[2]) + abs(new_synthese.at<cv::Vec3b>(x_crt+1, y_crt)[2] - texture.at<cv::Vec3b>(t_At[0], t_At[1])[2]);
                                cost = (color1+color2+color3)/3;
                                //std::cout<<"cost entre seam et 4 = "<<cost<<std::endl;
                                g->add_edge(mat_num.at<uchar>(i+1,j), nb_pixels-1+seam_supp, cost, cost);
//                                g->add_edge(num+overlapCols, nb_pixels-1+seam_supp, cost, cost);
                            }
                        }

                        else if(seams.at<cv::Vec2b>(x_crt, y_crt)[1] == 4) // droite
                        {
                            //std::cout << "COUCOU DROITE" << std::endl;
                            if(overlap_zone.at<uchar>(x_crt,y_crt) == 1 && mask.at<uchar>(x_crt,y_crt+1)==1)
                            {
                                droite = true;

                                // Rajoute un noeud dans le graphe
                                seam_supp ++;
                                g->add_node();

                                cv::Vec2b s_As = init_value_seams.at<cv::Vec2b>(x_crt, y_crt);
                                cv::Vec2b t_As = s_As + cv::Vec2b(0,1);
                                cv::Vec2b t_At = init_value_seams.at<cv::Vec2b>(x_crt,y_crt+1);
                                cv::Vec2b s_At = t_At - cv::Vec2b(0,1);
                                float color1, color2, color3, cost;

                                // avec B M(1, 4, A1 , A4 )
                                color1 = abs(texture.at<cv::Vec3b>(s_As[0], s_As[1])[0] - texture.at<cv::Vec3b>(s_At[0], s_At[1])[0]) + abs(texture.at<cv::Vec3b>(t_As[0], t_As[1])[0] - texture.at<cv::Vec3b>(t_At[0], t_At[1])[0]);
                                color2 = abs(texture.at<cv::Vec3b>(s_As[0], s_As[1])[1] - texture.at<cv::Vec3b>(s_At[0], s_At[1])[1]) + abs(texture.at<cv::Vec3b>(t_As[0], t_As[1])[1] - texture.at<cv::Vec3b>(t_At[0], t_At[1])[1]);
                                color3 = abs(texture.at<cv::Vec3b>(s_As[0], s_As[1])[2] - texture.at<cv::Vec3b>(s_At[0], s_At[1])[2]) + abs(texture.at<cv::Vec3b>(t_As[0], t_As[1])[2] - texture.at<cv::Vec3b>(t_At[0], t_At[1])[2]);
                                cost = (color1+color2+color3)/3;
                                g->add_tweights(nb_pixels-1+seam_supp,0,cost);

                                // entre le point courant et le seam supplémentaire M(1, 4, A1 , B)
                                color1 = abs(texture.at<cv::Vec3b>(s_As[0], s_As[1])[0] - new_synthese.at<cv::Vec3b>(x_crt, y_crt)[0]) + abs(texture.at<cv::Vec3b>(t_As[0], t_As[1])[0] - new_synthese.at<cv::Vec3b>(x_crt, y_crt+1)[0]);
                                color2 = abs(texture.at<cv::Vec3b>(s_As[0], s_As[1])[1] - new_synthese.at<cv::Vec3b>(x_crt, y_crt)[1]) + abs(texture.at<cv::Vec3b>(t_As[0], t_As[1])[1] - new_synthese.at<cv::Vec3b>(x_crt, y_crt+1)[1]);
                                color3 = abs(texture.at<cv::Vec3b>(s_As[0], s_As[1])[2] - new_synthese.at<cv::Vec3b>(x_crt, y_crt)[2]) + abs(texture.at<cv::Vec3b>(t_As[0], t_As[1])[2] - new_synthese.at<cv::Vec3b>(x_crt, y_crt+1)[2]);
                                cost = (color1+color2+color3)/3;
                                g->add_edge(mat_num.at<uchar>(i,j), nb_pixels-1+seam_supp, cost, cost);
//                                g->add_edge(num, nb_pixels-1+seam_supp, cost, cost);

                                // entre le seam supplémentaire et le point de l'autre coté de la bordure M(1, 4, B, A4 )
                                color1 = abs(new_synthese.at<cv::Vec3b>(x_crt, y_crt)[0] - texture.at<cv::Vec3b>(s_At[0], s_At[1])[0]) + abs(new_synthese.at<cv::Vec3b>(x_crt, y_crt+1)[0] - texture.at<cv::Vec3b>(t_At[0], t_At[1])[0]);
                                color2 = abs(new_synthese.at<cv::Vec3b>(x_crt, y_crt)[1] - texture.at<cv::Vec3b>(s_At[0], s_At[1])[1]) + abs(new_synthese.at<cv::Vec3b>(x_crt, y_crt+1)[1] - texture.at<cv::Vec3b>(t_At[0], t_At[1])[1]);
                                color3 = abs(new_synthese.at<cv::Vec3b>(x_crt, y_crt)[2] - texture.at<cv::Vec3b>(s_At[0], s_At[1])[2]) + abs(new_synthese.at<cv::Vec3b>(x_crt, y_crt+1)[2] - texture.at<cv::Vec3b>(t_At[0], t_At[1])[2]);
                                cost = (color1+color2+color3)/3;
                                g->add_edge(mat_num.at<uchar>(i,j+1), nb_pixels-1+seam_supp, cost, cost);
//                                g->add_edge(num+1, nb_pixels-1+seam_supp, cost, cost);
                            }
                        }
                    }

                    if(i<overlapRows-1 && mask.at<uchar>(x_crt+1,y_crt)==1 && bas == false) // sauf bordure bas - calcul du cout avec le point en dessous et la réciproque
                    {
                        // liaison avec le point en dessous
                        int x_adj = x_crt + 1;
                        int y_adj = y_crt;
                        //float cost_direct = norm(old_synthese.at<cv::Vec3b>(x_crt,y_crt) - new_synthese.at<cv::Vec3b>(x_crt,y_crt)) + norm(old_synthese.at<cv::Vec3b>(x_adj,y_adj) - new_synthese.at<cv::Vec3b>(x_adj,y_adj));
                        //float cost_indirect = norm(old_synthese.at<cv::Vec3b>(x_adj,y_adj) - new_synthese.at<cv::Vec3b>(x_adj,y_adj)) + norm(old_synthese.at<cv::Vec3b>(x_crt,y_crt) - new_synthese.at<cv::Vec3b>(x_crt,y_crt));
                        float cost = compute_cost_edge(x_crt, y_crt, x_adj, y_adj, old_synthese, new_synthese);
                        g->add_edge(mat_num.at<uchar>(i,j), mat_num.at<uchar>(i+1,j), cost, cost);
//                      g->add_edge(num, num+overlapCols, cost, cost);
                        //std::cout<<"en dessous: "<<cost<<std::endl;
                        //std::cout << "NumBas " << num << " "<< num+overlapCols<<std::endl;

                        //std::cout << " Bas " << (int)mat_num.at<uchar>(i,j)<< " B " <<(int)mat_num.at<uchar>(i+1,j) <<std::endl;
                    }

                    if(j<overlapCols-1 && mask.at<uchar>(x_crt,y_crt+1)==1 && droite == false) // sauf bordure droite - calcul du cout avec le point à droite et la réciproque
                    {
                        // liaison avec le point à droite
                        int x_adj = x_crt;
                        int y_adj = y_crt + 1;
                        float cost = compute_cost_edge(x_crt, y_crt, x_adj, y_adj, old_synthese, new_synthese);
                        //float cost_direct = norm(old_synthese.at<cv::Vec3b>(x_crt,y_crt) - new_synthese.at<cv::Vec3b>(x_crt,y_crt)) + norm(old_synthese.at<cv::Vec3b>(x_adj,y_adj) - new_synthese.at<cv::Vec3b>(x_adj,y_adj));
                        //float cost_indirect = norm(old_synthese.at<cv::Vec3b>(x_adj,y_adj) - new_synthese.at<cv::Vec3b>(x_adj,y_adj)) + norm(old_synthese.at<cv::Vec3b>(x_crt,y_crt) - new_synthese.at<cv::Vec3b>(x_crt,y_crt));
                        g->add_edge(mat_num.at<uchar>(i,j), mat_num.at<uchar>(i,j+1), cost, cost);
//                        g->add_edge(num, num+1, cost, cost);
                        //std::cout << "NumDro " << num <<" "<< num+1<<std::endl;
                        //std::cout << " Dro " << (int)mat_num.at<uchar>(i,j)<< " B " <<(int)mat_num.at<uchar>(i,j+1) <<std::endl;
                        //std::cout<<"a droite: "<<cost<<std::endl;
                    }

                    if(overlap_zone.at<uchar>(x_crt,y_crt) == 2) // appartient au nouveau patch
                    {
                        g->add_tweights( mat_num.at<uchar>(i,j),   /* capacities old=source new=sink */ 0, 16384 ); // 1/4 de la valeur maximale possible pour un int (32 signed oint) = 65536/4
//                        g->add_tweights( num,   /* capacities old=source new=sink*/ 0, 16384 );
                        //std::cout << "Tn " << num << " " << (int)mat_num.at<uchar>(i,j) << std::endl;
                    }

                    if(overlap_zone.at<uchar>(x_crt,y_crt) == 3) // appartient à l'image de base
                    {
                        g->add_tweights( mat_num.at<uchar>(i,j),   /* capacities old=source new=sink*/ 16384, 0 );
//                        g->add_tweights( num,   /* capacities old=source new=sink*/ 16384, 0 );
                        //std::cout << "To " << num << " " << (int)mat_num.at<uchar>(i,j) << std::endl;
                    }

                    ++num;
                }
            }

        //mat_affichage(overlap_zone);
//        std::cout<<"avant calcul maxflow()"<<std::endl;
//        std::cout<<"Nombre de nodes dans graph = "<<g->get_node_num()<<" devrait etre num+seam_supp = "<<num<<"+"<<seam_supp<<"="<<num+seam_supp<<std::endl;
        int flow = g -> maxflow();
//        std::cout<<"après calcul maxflow()"<<std::endl;

//        printf("Flow = %d\n", flow);
        //printf("Minimum cut:\n");

        num=0;
        for(int i=0; i<overlapRows; ++i) // lignes
        {
            for(int j=0; j<overlapCols; ++j) // colonnes
            {
                int x_crt = overlap_corner.x + i;
                int y_crt = overlap_corner.y + j;
                if(overlap_zone.at<uchar>(x_crt,y_crt)!=0)
                {
                    if(g->what_segment(num) == GraphType::SOURCE)
                    {
                        //std::cout<<"1 ";
                        new_synthese.at<cv::Vec3b>(x_crt, y_crt) = old_synthese.at<cv::Vec3b>(x_crt, y_crt);
                        mask_seam.at<uchar>(i,j) = 1;
                    }
                    else //if(g->what_segment(num) == GraphType::SINK)
                    {
                        //std::cout<<"2 ";
                        mask_seam.at<uchar>(i,j) = 2;
                    }
                    // Sinon appartient au SINK donc garde la valeur du new_synthese
                    ++num;
                }
            }
            //std::cout<<std::endl;
        }

        //delete g;
        imshow("new", new_synthese); cv::waitKey(0);

        update_seams(overlap_corner, mask_seam, iter+1);
        update_init_value_seams(overlap_corner, t, mask_seam);

//        std::cout<<"mask_seam :"<<std::endl;
//        for(int u=0; u<overlapRows; ++u){
//            for(int v=0; v<overlapCols; ++v)
//            {
//                std::cout<<(int)mask_seam.at<uchar>(u,v);
//            }
//            std::cout<<std::endl;
//        }

//        std::cout<<"seams[0] :"<<std::endl;
//        for(int u=0; u<overlapRows; ++u){
//            for(int v=0; v<overlapCols; ++v)
//            {
//                std::cout<<(int)seams.at<cv::Vec2b>(overlap_corner.x+u,overlap_corner.y+v)[0];
//            }
//            std::cout<<std::endl;
//        }
        cv::Mat border_seams[2];
        split(seams,border_seams);
        cv::namedWindow("seams",CV_WINDOW_NORMAL);
        imshow("seams",border_seams[0]*255);


        // Mise à jour pour l'itération suivante
        new_synthese.copyTo(old_synthese);
        update_mask(t);
        overlap_zone.zeros(imRows,imCols,CV_8UC1);

    }

    delete g;
    return;
}




