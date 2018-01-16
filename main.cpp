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
    cv::Mat image = cv::imread("../GraphCuts/images/fraise_32.jpg");
    if (!image.data)
    {
        std::cout << "Error reading file 1 "<< "image/" << std::endl;
        exit(0);
    }

    minCut mincut(image,70,70);

    mincut.compute_minCut();





//    //imshow("Image de base", image); cv::waitKey(0);

//    int superp = 100;



//    cv::Mat old_synthese =  cv::Mat::zeros(700, 1000, CV_8UC3);
//    cv::Mat new_synthese =  cv::Mat::zeros(700, 1000, CV_8UC3);

//    // Copie de l'image originale dans l'image de synthèse que l'on cherche à créer
//    image(cv::Range(0,image.rows),cv::Range(0,image.cols)).copyTo(new_synthese(cv::Rect(300,400,image.cols,image.rows)));
//    new_synthese.copyTo(old_synthese);

//    image(cv::Range(0,image.rows),cv::Range(0,image.cols)).copyTo(new_synthese(cv::Rect(300,400-image.rows+superp,image.cols,image.rows)));
//    imshow("Image new", new_synthese); cv::waitKey(0);
//    //imshow("Image old", old_synthese); cv::waitKey(0);

//    cv::Mat superp_new = cv::Mat::zeros(superp, image.cols, CV_8UC3);
//    new_synthese(cv::Range(400,400+superp-1),cv::Range(300,300+image.cols)).copyTo(superp_new);
//    cv::Mat superp_old = cv::Mat::zeros(superp, image.cols, CV_8UC3);
//    old_synthese(cv::Range(400,400+superp-1),cv::Range(300,300+image.cols)).copyTo(superp_old);
//    //imshow("Image", superp_new); cv::waitKey(0);
//    //imshow("Image", superp_old); cv::waitKey(0);

//    typedef Graph<int,int,int> GraphType;
//    GraphType *g = new GraphType(/*estimated # of nodes*/ superp*image.cols, /*estimated # of edges*/ (image.cols-1)*(superp-1));

//    g->add_node(superp*image.cols);

//    int x_superp = 400;
//    int y_superp = 300;

//    int num = 0;
//    for(int i=0; i<superp; ++i) // lignes
//        for(int j=0; j<image.cols; ++j) // colonnes
//        {
//            std::cout<<"num = "<<num<<std::endl;
//            // Calcul edge à droite et en bas pour chaque point dans les deux sens

//            int x_crt = x_superp + i;
//            int y_crt = y_superp + j;
//            if(i<superp-1) // sauf bordure bas - calcul du cout avec le point en dessous et la réciproque
//            {
//                // liaison avec le point en dessous
//                int x_adj = x_crt;
//                int y_adj = y_crt + 1;
//                float cost_direct = norm(old_synthese.at<cv::Vec3b>(x_crt,y_crt) - new_synthese.at<cv::Vec3b>(x_crt,y_crt)) + norm(old_synthese.at<cv::Vec3b>(x_adj,y_adj) - new_synthese.at<cv::Vec3b>(x_adj,y_adj));
//                float cost_indirect = norm(old_synthese.at<cv::Vec3b>(x_adj,y_adj) - new_synthese.at<cv::Vec3b>(x_adj,y_adj)) + norm(old_synthese.at<cv::Vec3b>(x_crt,y_crt) - new_synthese.at<cv::Vec3b>(x_crt,y_crt));
//                g->add_edge(num, num+image.rows, cost_direct, cost_indirect);
//                std::cout<<"en dessous: "<<cost_direct<<" "<<cost_indirect<<std::endl;
//            }

//            if(j<image.cols-1) // sauf bordure droite - calcul du cout avec le point à droite et la réciproque
//            {
//                // liaison avec le point à droite
//                int x_adj = x_crt + 1;
//                int y_adj = y_crt;
//                float cost_direct = norm(old_synthese.at<cv::Vec3b>(x_crt,y_crt) - new_synthese.at<cv::Vec3b>(x_crt,y_crt)) + norm(old_synthese.at<cv::Vec3b>(x_adj,y_adj) - new_synthese.at<cv::Vec3b>(x_adj,y_adj));
//                float cost_indirect = norm(old_synthese.at<cv::Vec3b>(x_adj,y_adj) - new_synthese.at<cv::Vec3b>(x_adj,y_adj)) + norm(old_synthese.at<cv::Vec3b>(x_crt,y_crt) - new_synthese.at<cv::Vec3b>(x_crt,y_crt));
//                g->add_edge(num, num+1, cost_direct, cost_indirect);
//                std::cout<<"a droite: "<<cost_direct<<" "<<cost_indirect<<std::endl;
//            }

//            if(i==0) // appartient au nouveau patch
//                g->add_tweights( num,   /* capacities old=source new=sink */ 0, 1000 );

//            if(i==superp-1) // appartient à l'image de base
//                g->add_tweights( num,   /* capacities old=source new=sink*/ 1000, 0 );

//            ++num;
//        }

//    int flow = g -> maxflow();

//    printf("Flow = %d\n", flow);
//    printf("Minimum cut:\n");

//    num=0;
//    for(int i=0; i<superp; ++i) // lignes
//        for(int j=0; j<image.cols; ++j) // colonnes
//        {
//            int x_crt = x_superp + i;
//            int y_crt = y_superp + j;
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

//    imshow("Image", new_synthese); cv::waitKey(0);

//    //std::cout<<"size image : cols = "<<image.cols<<" rows = "<<image.rows<<std::endl;

    return 0;
}

