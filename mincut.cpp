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
    seams = cv::Mat::zeros(imRows,imCols,CV_32FC3);
    init_value_seams = cv::Mat::zeros(imRows, imCols, CV_16UC2);

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
    seams = cv::Mat::zeros(imRows,imCols,CV_32FC3);
    init_value_seams = cv::Mat::zeros(imRows, imCols, CV_16UC2);

    maxSuperposedPixel = 256;
    minSuperPosedPixel = 256;

    border_mask = cv::Vec4i(imRows,0,imCols,0);
}


void minCut::init()
{
    // Copie de l'image originale dans l'image de synthèse que l'on cherche à créer
    cv::Point2i t (floor(imRows/3),floor(imCols/3));
    texture(cv::Range(0,patchRows),cv::Range(0,patchCols)).copyTo(old_synthese(cv::Rect(t.y, t.x, patchCols, patchRows)));
    old_synthese.copyTo(new_synthese);

    update_mask(t);
    update_init_value_seams(t);
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
        if(n!=0)
            overlapCols=n;
        n=0;
    }

    bool limits = false; // on veut que les pixels source/sink soient en haut/bas OU gauche/droite
    // On cherche à définir si les points appartiennent au nouveau patch ou à l'ancienne synthèse
    for(int u=0; u<overlapRows; ++u)
    {
        // colonne de gauche = corner.y
        if(corner.y-1 >= t.y && corner.y-1 <= t.y+patchCols-1) // la colonne à droite de la colonne la plus à gauche de notre overlap se trouve dans le patch -> appartient au patch
        {
            overlap_zone.at<uchar>(corner.x+u,corner.y) = 2;
            limits = true;
        }
        else if(mask.at<uchar>(corner.x+u, corner.y-1) == 1) // si cette colonne est à coé d'une colonne appartenant au mask -> appartient à l'ancienne synthèse
        {
            overlap_zone.at<uchar>(corner.x+u,corner.y) = 3;
            limits = true;
        }
        //sinon appartient à ni l'un ni l'autre

        // colonne de droite = corner.y+(overlapCols-1)
        if(corner.y+(overlapCols-1)+1 >= t.y && corner.y+overlapCols <= t.y+patchCols-1) // appartient au patch
        {
            overlap_zone.at<uchar>(corner.x+u,corner.y+overlapCols-1) = 2;
            limits = true;
        }
        else if(mask.at<uchar>(corner.x+u, corner.y+overlapCols) == 1)
        {
            overlap_zone.at<uchar>(corner.x+u,corner.y+overlapCols-1) = 3;
            limits = true;
        }
    }

    if(limits == false)
    {
        for(int v=0; v<overlapCols; ++v)
        {
            // ligne du haut = corner.x
            if(corner.x-1 >= t.x && corner.x-1 < t.x+patchRows)
                overlap_zone.at<uchar>(corner.x,corner.y+v) = 2;
            else if(mask.at<uchar>(corner.x-1, corner.y+v) == 1)
                overlap_zone.at<uchar>(corner.x,corner.y+v) = 3;

            // ligne du bas = corner.x+(overlapRows-1)
            if(corner.x+(overlapRows-1)+1 >= t.x && corner.x+overlapRows < t.x+patchRows-1)
                overlap_zone.at<uchar>(corner.x+overlapRows-1,corner.y+v) = 2;
            else if(mask.at<uchar>(corner.x+overlapRows, corner.y+v) == 1)
                overlap_zone.at<uchar>(corner.x+overlapRows-1,corner.y+v) = 3;
        }
    }

    return corner;
}


void minCut::update_seams(cv::Point2i corner, cv::Mat mask_seam, int index_patch)
// mask_seam = 3 valeurs ; 0: 0 si old et 1 si new; 1: cout à droite; 2: cout en bas
// index_patch = numéro du nouveau patch
// seams : 1: numéro du patch; 2: pixel avec lequel on calcul le cout; 3: le cout
// pixel avec lequel on calcul le cout : 1=en haut, 2=en bas, 3=à gauche, 4= à droite
// possibilité d'optimiser
{
    bool found = false;
    for(int i=0; i<mask_seam.size[0]; ++i)
        for(int j=0; j<mask_seam.size[1]; ++j)
        {
            found = false;
            //Parcours les 4 voisins pour voir si frontière (un 1 et un 0 à coté)
            int val_mask = mask_seam.at<cv::Vec3f>(i,j)[0];
            // faire conditions haut/bas
            if(i>0)
            {
                if(mask_seam.at<cv::Vec3f>(i-1,j)[0] != val_mask) //haut
                {
                    seams.at<cv::Vec3f>(corner.x+i, corner.y+j)[1] = 1;
                    seams.at<cv::Vec3f>(corner.x+i, corner.y+j)[2] = mask_seam.at<cv::Vec3f>(i-1,j)[2];
                    if(val_mask == 1) // nouveau patch donc pas encore présent dans seams
                        seams.at<cv::Vec3f>(corner.x+i, corner.y+j)[0] = index_patch;
                    //else garde le meme index, mais cout change car la frontiere a évoluée
                    else if (val_mask == 0 && seams.at<cv::Vec3f>(corner.x+i, corner.y+j)[0] == 0)
                        seams.at<cv::Vec3f>(corner.x+i, corner.y+j)[0] = 1;
                    found = true;
                }
            }
            if(i<mask_seam.size[0]-1 && found == false)
            {
                if(mask_seam.at<cv::Vec3f>(i+1,j)[0] != val_mask) //bas
                {
                    seams.at<cv::Vec3f>(corner.x+i, corner.y+j)[1] = 2;
                    seams.at<cv::Vec3f>(corner.x+i, corner.y+j)[2] = mask_seam.at<cv::Vec3f>(i,j)[2];
                    if(val_mask == 1)
                        seams.at<cv::Vec3f>(corner.x+i, corner.y+j)[0] = index_patch;
                    else if (val_mask == 0 && seams.at<cv::Vec3f>(corner.x+i, corner.y+j)[0] == 0)
                        seams.at<cv::Vec3f>(corner.x+i, corner.y+j)[0] = 1;
                    found = true;
                }
            }
            if(j>0 && found == false)
            {
                if(mask_seam.at<cv::Vec3f>(i,j-1)[0] != val_mask) //gauche
                {
                    seams.at<cv::Vec3f>(corner.x+i, corner.y+j)[1] = 3;
                    seams.at<cv::Vec3f>(corner.x+i, corner.y+j)[2] = mask_seam.at<cv::Vec3f>(i,j-1)[1];
                    if(val_mask == 1)
                        seams.at<cv::Vec3f>(corner.x+i, corner.y+j)[0] = index_patch;
                    else if (val_mask == 0 && seams.at<cv::Vec3f>(corner.x+i, corner.y+j)[0] == 0)
                        seams.at<cv::Vec3f>(corner.x+i, corner.y+j)[0] = 1;
                    found = true;
                }
            }
            if(j<mask_seam.size[1]-1 && found == false)
            {
                if(mask_seam.at<cv::Vec3f>(i,j+1)[0] != val_mask) //droite
                {
                    seams.at<cv::Vec3f>(corner.x+i, corner.y+j)[1] = 4;
                    seams.at<cv::Vec3f>(corner.x+i, corner.y+j)[2] = mask_seam.at<cv::Vec3f>(i,j)[1];
                    if(val_mask == 1)
                        seams.at<cv::Vec3f>(corner.x+i, corner.y+j)[0] = index_patch;
                    else if (val_mask == 0 && seams.at<cv::Vec3f>(corner.x+i, corner.y+j)[0] == 0)
                        seams.at<cv::Vec3f>(corner.x+i, corner.y+j)[0] = 1;
                    found = true;
                }
            }
        }
}


void minCut::update_init_value_seams(cv::Point2i corner)
{
    for(int i=0; i<patchRows; ++i)
        for(int j=0; j<patchCols; ++j)
        {
            if(init_value_seams.at<cv::Vec2i>(corner.x+i, corner.y+j) == cv::Vec2i(patchRows+100,patchCols+100))
            {
                init_value_seams.at<cv::Vec2i>(corner.x+i, corner.y+j) = cv::Vec2i(i,j);
            }
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
                    cost += (std::pow(abs(old[0]-update[0]),2) + std::pow(abs(old[1]-update[1]),2) + std::pow(abs(old[2]-update[2]),2))/3;
                }
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

    cv::Point2i t(12,12);
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
                //std::cout<<"cost = "<<cost<<std::endl;
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


void minCut::mat_affichage_vec(cv::Mat mat)
{
    for(int u=0; u<mat.size[0]; ++u){
        for(int v=0; v<mat.size[1]; ++v)
        {
            std::cout<<"("<<mat.at<cv::Vec3f>(u,v)[0]<<", "<<mat.at<cv::Vec3f>(u,v)[1]<<", "<<mat.at<cv::Vec3f>(u,v)[2]<<") ";
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

    init();

    typedef Graph<int,int,int> GraphType;
    GraphType *g = new GraphType(/*estimated # of nodes*/ /*nb_pixels*/ maxSuperposedPixel, /*estimated # of edges*/ (overlapCols-1)*(overlapRows-1));

    for(int iter=1; iter<10; ++iter)
    {
        int nb_pixels = 0;
        cv::Point2i t = entire_patch_matching_placement(nb_pixels);
        //std::cout<<"Apres patch placement t = "<<t.x<<" "<<t.y<<std::endl;

        texture(cv::Range(0,patchRows),cv::Range(0,patchCols)).copyTo(new_synthese(cv::Rect(t.y,t.x,patchCols,patchRows)));

        update_init_value_seams(t);

        cv::imwrite("synthese_non_amelioree.png",new_synthese);
        imshow("old", old_synthese); cv::waitKey(0);
        imshow("new", new_synthese); cv::waitKey(0);

        //std::cout<<"overlapCols = "<<overlapCols<<" overlapRows = "<<overlapRows<<std::endl;

        cv::Point2i overlap_corner = update_overlap_zone(t);

        cv::Mat overlap;
        new_synthese.copyTo(overlap,overlap_zone);
        imshow("overlap", overlap);cv::waitKey(0);

        std::cout<<"nb_pixels = "<<nb_pixels<<std::endl;
        std::cout<<"overlapCols = "<<overlapCols<<" overlapRows = "<<overlapRows<<std::endl;

        if(nb_pixels != overlapCols*overlapRows)
        {
            std::cout<<"Erreur : nb_pixels != overlapCols*overlapRows"<<std::endl;
            exit(0);
        }

        g->add_node(nb_pixels);

        mat_affichage(overlap_zone);

        std::cout<<"overlap_corner = "<<overlap_corner.x <<" "<<overlap_corner.y<<std::endl;

        // mask_seam = 3 valeurs ; 0: 0 si old et 1 si new; 1: cout à droite; 2: cout en bas
        cv::Mat mask_seam = cv::Mat::zeros(overlapRows, overlapCols, CV_32FC3);

        int num = 0;
        int seam_supp = 0;
        for(int i=0; i<overlapRows; ++i) // lignes
            for(int j=0; j<overlapCols; ++j) // colonnes
            {
                // Calcul edge à droite et en bas pour chaque point dans les deux sens
                int x_crt = overlap_corner.x + i;
                int y_crt = overlap_corner.y + j;

                // On regarde si une ancienne bordure se trouve à cet endroit là
                if(seams.at<cv::Vec3f>(x_crt, y_crt)[0] != 0)
                {
                    // Rajoute un noeud dans le graphe
                    seam_supp ++;
                    g->add_node();

                    if(seams.at<cv::Vec3f>(x_crt, y_crt)[1] == 2) // bas
                    {
                        // avec B M(1, 4, A1 , A4 )
                        cv::Vec2i s_As = init_value_seams.at<cv::Vec2i>(x_crt, y_crt);
                        cv::Vec2i t_As = s_As + cv::Vec2i(1,0);
                        cv::Vec2i t_At = init_value_seams.at<cv::Vec2i>(x_crt+1,y_crt);
                        cv::Vec2i s_At = t_As + cv::Vec2i(-1,0);
                        float color1 = abs(texture.at<cv::Vec3b>(s_As[0], s_As[1])[0] - texture.at<cv::Vec3b>(s_At[0], s_At[1])[0]) + abs(texture.at<cv::Vec3b>(t_As[0], t_As[1])[0] - texture.at<cv::Vec3b>(t_At[0], t_At[1])[0]);
                        float color2 = abs(texture.at<cv::Vec3b>(s_As[0], s_As[1])[1] - texture.at<cv::Vec3b>(s_At[0], s_At[1])[1]) + abs(texture.at<cv::Vec3b>(t_As[0], t_As[1])[1] - texture.at<cv::Vec3b>(t_At[0], t_At[1])[1]);
                        float color3 = abs(texture.at<cv::Vec3b>(s_As[0], s_As[1])[2] - texture.at<cv::Vec3b>(s_At[0], s_At[1])[2]) + abs(texture.at<cv::Vec3b>(t_As[0], t_As[1])[2] - texture.at<cv::Vec3b>(t_At[0], t_At[1])[2]);
                        float cost = (color1+color2+color3)/3;
                        g->add_tweights(nb_pixels+seam_supp,0,cost);

                        // entre le point courant et le seam supplémentaire M(1, 4, A1 , B)
                        color1 = abs(texture.at<cv::Vec3b>(s_As[0], s_As[1])[0] - new_synthese.at<cv::Vec3b>(x_crt, y_crt)[0]) + abs(texture.at<cv::Vec3b>(t_As[0], t_As[1])[0] - new_synthese.at<cv::Vec3b>(x_crt+1, y_crt)[0]);
                        color2 = abs(texture.at<cv::Vec3b>(s_As[0], s_As[1])[1] - new_synthese.at<cv::Vec3b>(x_crt, y_crt)[1]) + abs(texture.at<cv::Vec3b>(t_As[0], t_As[1])[1] - new_synthese.at<cv::Vec3b>(x_crt+1, y_crt)[1]);
                        color3 = abs(texture.at<cv::Vec3b>(s_As[0], s_As[1])[2] - new_synthese.at<cv::Vec3b>(x_crt, y_crt)[2]) + abs(texture.at<cv::Vec3b>(t_As[0], t_As[1])[2] - new_synthese.at<cv::Vec3b>(x_crt+1, y_crt)[2]);
                        cost = (color1+color2+color3)/3;
                        g->add_edge(num, num+seam_supp, cost, cost);

                        // entre le seam supplémentaire et le point de l'autre coté de la bordure M(1, 4, B, A4 )
                        color1 = abs(new_synthese.at<cv::Vec3b>(x_crt, y_crt)[0] - texture.at<cv::Vec3b>(s_At[0], s_At[1])[0]) + abs(new_synthese.at<cv::Vec3b>(x_crt+1, y_crt)[0] - texture.at<cv::Vec3b>(t_At[0], t_At[1])[0]);
                        color2 = abs(new_synthese.at<cv::Vec3b>(x_crt, y_crt)[1] - texture.at<cv::Vec3b>(s_At[0], s_At[1])[1]) + abs(new_synthese.at<cv::Vec3b>(x_crt+1, y_crt)[1] - texture.at<cv::Vec3b>(t_At[0], t_At[1])[1]);
                        color3 = abs(new_synthese.at<cv::Vec3b>(x_crt, y_crt)[2] - texture.at<cv::Vec3b>(s_At[0], s_At[1])[2]) + abs(new_synthese.at<cv::Vec3b>(x_crt+1, y_crt)[2] - texture.at<cv::Vec3b>(t_At[0], t_At[1])[2]);
                        cost = (color1+color2+color3)/3;
                        g->add_edge(num+1, num+seam_supp, cost, cost);
                    }

                    else if(seams.at<cv::Vec3f>(x_crt, y_crt)[1] == 3) // droite
                    {
                        // avec B M(1, 4, A1 , A4 )
                        cv::Vec2i s_As = init_value_seams.at<cv::Vec2i>(x_crt, y_crt);
                        cv::Vec2i t_As = s_As + cv::Vec2i(0,1);
                        cv::Vec2i t_At = init_value_seams.at<cv::Vec2i>(x_crt,y_crt+1);
                        cv::Vec2i s_At = t_As + cv::Vec2i(0,-1);
                        float color1 = abs(texture.at<cv::Vec3b>(s_As[0], s_As[1])[0] - texture.at<cv::Vec3b>(s_At[0], s_At[1])[0]) + abs(texture.at<cv::Vec3b>(t_As[0], t_As[1])[0] - texture.at<cv::Vec3b>(t_At[0], t_At[1])[0]);
                        float color2 = abs(texture.at<cv::Vec3b>(s_As[0], s_As[1])[1] - texture.at<cv::Vec3b>(s_At[0], s_At[1])[1]) + abs(texture.at<cv::Vec3b>(t_As[0], t_As[1])[1] - texture.at<cv::Vec3b>(t_At[0], t_At[1])[1]);
                        float color3 = abs(texture.at<cv::Vec3b>(s_As[0], s_As[1])[2] - texture.at<cv::Vec3b>(s_At[0], s_At[1])[2]) + abs(texture.at<cv::Vec3b>(t_As[0], t_As[1])[2] - texture.at<cv::Vec3b>(t_At[0], t_At[1])[2]);
                        float cost = (color1+color2+color3)/3;
                        g->add_tweights(nb_pixels+seam_supp,0,cost);

                        // entre le point courant et le seam supplémentaire M(1, 4, A1 , B)
                        color1 = abs(texture.at<cv::Vec3b>(s_As[0], s_As[1])[0] - new_synthese.at<cv::Vec3b>(x_crt, y_crt)[0]) + abs(texture.at<cv::Vec3b>(t_As[0], t_As[1])[0] - new_synthese.at<cv::Vec3b>(x_crt, y_crt+1)[0]);
                        color2 = abs(texture.at<cv::Vec3b>(s_As[0], s_As[1])[1] - new_synthese.at<cv::Vec3b>(x_crt, y_crt)[1]) + abs(texture.at<cv::Vec3b>(t_As[0], t_As[1])[1] - new_synthese.at<cv::Vec3b>(x_crt, y_crt+1)[1]);
                        color3 = abs(texture.at<cv::Vec3b>(s_As[0], s_As[1])[2] - new_synthese.at<cv::Vec3b>(x_crt, y_crt)[2]) + abs(texture.at<cv::Vec3b>(t_As[0], t_As[1])[2] - new_synthese.at<cv::Vec3b>(x_crt, y_crt+1)[2]);
                        cost = (color1+color2+color3)/3;
                        g->add_edge(num, num+seam_supp, cost, cost);

                        // entre le seam supplémentaire et le point de l'autre coté de la bordure M(1, 4, B, A4 )
                        color1 = abs(new_synthese.at<cv::Vec3b>(x_crt, y_crt)[0] - texture.at<cv::Vec3b>(s_At[0], s_At[1])[0]) + abs(new_synthese.at<cv::Vec3b>(x_crt, y_crt+1)[0] - texture.at<cv::Vec3b>(t_At[0], t_At[1])[0]);
                        color2 = abs(new_synthese.at<cv::Vec3b>(x_crt, y_crt)[1] - texture.at<cv::Vec3b>(s_At[0], s_At[1])[1]) + abs(new_synthese.at<cv::Vec3b>(x_crt, y_crt+1)[1] - texture.at<cv::Vec3b>(t_At[0], t_At[1])[1]);
                        color3 = abs(new_synthese.at<cv::Vec3b>(x_crt, y_crt)[2] - texture.at<cv::Vec3b>(s_At[0], s_At[1])[2]) + abs(new_synthese.at<cv::Vec3b>(x_crt, y_crt+1)[2] - texture.at<cv::Vec3b>(t_At[0], t_At[1])[2]);
                        cost = (color1+color2+color3)/3;
                        g->add_edge(num+1, num+seam_supp, cost, cost);
                    }
                }

                if(i<overlapRows-1) // sauf bordure bas - calcul du cout avec le point en dessous et la réciproque
                {
                    // liaison avec le point en dessous
                    int x_adj = x_crt;
                    int y_adj = y_crt + 1;
                    //float cost_direct = norm(old_synthese.at<cv::Vec3b>(x_crt,y_crt) - new_synthese.at<cv::Vec3b>(x_crt,y_crt)) + norm(old_synthese.at<cv::Vec3b>(x_adj,y_adj) - new_synthese.at<cv::Vec3b>(x_adj,y_adj));
                    //float cost_indirect = norm(old_synthese.at<cv::Vec3b>(x_adj,y_adj) - new_synthese.at<cv::Vec3b>(x_adj,y_adj)) + norm(old_synthese.at<cv::Vec3b>(x_crt,y_crt) - new_synthese.at<cv::Vec3b>(x_crt,y_crt));
                    float cost = compute_cost_edge(x_crt, y_crt, x_adj, y_adj, old_synthese, new_synthese);
                    g->add_edge(num, num+overlapRows, cost, cost);
                    mask_seam.at<cv::Vec3f>(i,j)[2] = cost;
                    //std::cout<<"en dessous: "<<cost<<std::endl;
                }

                if(j<overlapCols-1) // sauf bordure droite - calcul du cout avec le point à droite et la réciproque
                {
                    // liaison avec le point à droite
                    int x_adj = x_crt + 1;
                    int y_adj = y_crt;
                    float cost = compute_cost_edge(x_crt, y_crt, x_adj, y_adj, old_synthese, new_synthese);
                    //float cost_direct = norm(old_synthese.at<cv::Vec3b>(x_crt,y_crt) - new_synthese.at<cv::Vec3b>(x_crt,y_crt)) + norm(old_synthese.at<cv::Vec3b>(x_adj,y_adj) - new_synthese.at<cv::Vec3b>(x_adj,y_adj));
                    //float cost_indirect = norm(old_synthese.at<cv::Vec3b>(x_adj,y_adj) - new_synthese.at<cv::Vec3b>(x_adj,y_adj)) + norm(old_synthese.at<cv::Vec3b>(x_crt,y_crt) - new_synthese.at<cv::Vec3b>(x_crt,y_crt));
                    g->add_edge(num, num+1, cost, cost);
                    mask_seam.at<cv::Vec3f>(i,j)[1] = cost;
                    //std::cout<<"a droite: "<<cost<<std::endl;
                }

                if(overlap_zone.at<uchar>(x_crt,y_crt) == 2) // appartient au nouveau patch
                    g->add_tweights( num,   /* capacities old=source new=sink */ 0, 16384 ); // 1/4 de la valeur maximale possible pour un int (32 signed oint) = 65536/4

                if(overlap_zone.at<uchar>(x_crt,y_crt) == 3) // appartient à l'image de base
                    g->add_tweights( num,   /* capacities old=source new=sink*/ 16384, 0 );

                ++num;
            }

        std::cout<<"num = "<<num<<std::endl;

        int flow = g -> maxflow();

        printf("Flow = %d\n", flow);
        //printf("Minimum cut:\n");


        num=0;
        for(int i=0; i<overlapRows; ++i) // lignes
        {
            for(int j=0; j<overlapCols; ++j) // colonnes
            {
                int x_crt = overlap_corner.x + i;
                int y_crt = overlap_corner.y + j;
                if(g->what_segment(num) == GraphType::SOURCE)
                {
                    std::cout<<"0 ";
                    new_synthese.at<cv::Vec3b>(x_crt, y_crt) = old_synthese.at<cv::Vec3b>(x_crt, y_crt);
                    mask_seam.at<cv::Vec3f>(i,j)[0] = 0;
                }
                else //if(g->what_segment(num) == GraphType::SINK)
                {
                    std::cout<<"1 ";
                    mask_seam.at<cv::Vec3f>(i,j)[0] = 1;
                }
                // Sinon appartient au SINK donc garde la valeur du new_synthese
                ++num;
            }
            std::cout<<std::endl;
        }

        //delete g;

        imshow("new", new_synthese); cv::waitKey(0);
        cv::imwrite("new_synthese.png",new_synthese);

        update_seams(overlap_corner, mask_seam, iter+1);
        std::cout<<"mask_seams :"<<std::endl;
        mat_affichage_vec(mask_seam);
        std::cout<<"seams :"<<std::endl;
        mat_affichage_vec(seams);
        cv::Mat border_seams[3];
        split(seams,border_seams);
        imshow("seams",border_seams[0]);

        // Mise à jour pour l'itération suivante
        new_synthese.copyTo(old_synthese);
        update_mask(t);
        overlap_zone.zeros(imRows,imCols,CV_8UC1);
        g->reset();


    }

    delete g;
    return;
}




