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
    cv::Mat image = cv::imread("../GraphCuts/images/nuts_32.jpg");
    if (!image.data)
    {
        std::cout << "Error reading file 1 "<< "image/" << std::endl;
        exit(0);
    }

    minCut mincut(image,image.size[0]*3,image.size[1]*3);

    mincut.compute_minCut();


    /** TEST */
//    typedef Graph<int,int,int> GraphType;
//    GraphType *g = new GraphType(/*estimated # of nodes*/ 4, /*estimated # of edges*/ 4);

//    g -> add_node(4);

//    g -> add_tweights( 0,   /* capacities */  0, 1000 );
//    g -> add_edge( 0, 1,    /* capacities */  130, 130 );
//    g -> add_edge( 0, 2,    /* capacities */  10, 10 );

//    g -> add_tweights( 1,   /* capacities */  0, 1000 );
//    g -> add_edge( 1, 3,    /* capacities */  100, 100 );

//    g -> add_tweights( 2,   /* capacities */  1000, 0 );
//    g -> add_edge( 2, 3,    /* capacities */  300, 300 );

//    g -> add_tweights( 3,   /* capacities */  1000, 0 );

//    int flow = g -> maxflow();

//    printf("Flow = %d\n", flow);
//    printf("Minimum cut:\n");
//    if (g->what_segment(0) == GraphType::SOURCE)
//        printf("node0 is in the SOURCE set\n");
//    else
//        printf("node0 is in the SINK set\n");
//    if (g->what_segment(1) == GraphType::SOURCE)
//        printf("node1 is in the SOURCE set\n");
//    else
//        printf("node1 is in the SINK set\n");
//    if (g->what_segment(2) == GraphType::SOURCE)
//        printf("node2 is in the SOURCE set\n");
//    else
//        printf("node2 is in the SINK set\n");
//    if (g->what_segment(3) == GraphType::SOURCE)
//        printf("node3 is in the SOURCE set\n");
//    else
//        printf("node3 is in the SINK set\n");

//    delete g;


    return 0;
}

