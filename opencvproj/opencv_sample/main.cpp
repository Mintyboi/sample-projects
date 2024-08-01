/**
 * @file Drawing_1.cpp
 * @brief Simple geometric drawing
 * @author OpenCV team
 */
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include "bitmap.h"

#define w 400

using namespace cv;
// This is just a stubbed namespace
using namespace Gdiplus;

/// Function headers
void MyEllipse(Bitmap* inBitmap, double angle);
void MyFilledCircle(Bitmap* inBitmap, Point center);
void MyPolygon(Mat img);
void MyLine(Mat img, Point start, Point end);

/**
 * @function main
 * @brief Main function
 */
int main(void)
{
    //![create_images]
    /// Windows names
    char atom_window[] = "Drawing 1: Atom_withbitmap";
    char rook_window[] = "Drawing 2: Rook";

    /// Create black empty images
    Bitmap* atomBm = new Bitmap(w, w);
    Mat rook_image = Mat::zeros(w, w, CV_8UC3);
    //![create_images]


    /// 1. Draw a simple atom:
    /// -----------------------

    //![draw_atom]
    MyEllipse(atomBm, 90);
    MyEllipse(atomBm, 0);
    MyEllipse(atomBm, 45);
    MyEllipse(atomBm, -45);

    MyFilledCircle(atomBm, Point(w / 2, w / 2));


/// 2. Draw a rook
/// ------------------

//![draw_rook]
/// 2.a. Create a convex polygon
    MyPolygon(rook_image);

    //![rectangle]
    /// 2.b. Creating rectangles
    rectangle(rook_image,
        Point(0, 7 * w / 8),
        Point(w, w),
        Scalar(0, 255, 255),
        FILLED,
        LINE_8);
    //![rectangle]

    /// 2.c. Create a few lines
    MyLine(rook_image, Point(0, 15 * w / 16), Point(w, 15 * w / 16));
    MyLine(rook_image, Point(w / 4, 7 * w / 8), Point(w / 4, w));
    MyLine(rook_image, Point(w / 2, 7 * w / 8), Point(w / 2, w));
    MyLine(rook_image, Point(3 * w / 4, 7 * w / 8), Point(3 * w / 4, w));
    //![draw_rook]

    /// 3. Display your stuff!
    //Mat atom_image_bm(atomBm->GetHeight(), atomBm->GetWidth(), CV_8UC4, atomBm->GetRawData());
    imshow(atom_window, atomBm->getCvMat());
    moveWindow(atom_window, 0, 200);
    imshow(rook_window, rook_image);
    moveWindow(rook_window, w, 200);

    //delete atomBm;
    waitKey(0);
    return(0);
}

/// Function Declaration

/**
 * @function MyEllipse
 * @brief Draw a fixed-size ellipse with different angles
 */
 //![my_ellipse]
void MyEllipse(Bitmap* inBitmap, double angle)
{
    int thickness = 2;
    int lineType = 8;

    ellipse(inBitmap->getCvMat(),
        Point(w / 2, w / 2),
        Size(w / 4, w / 16),
        angle,
        0,
        360,
        Scalar(255, 0, 0),
        thickness,
        lineType);
}
//![my_ellipse]

/**
 * @function MyFilledCircle
 * @brief Draw a fixed-size filled circle
 */
 //![my_filled_circle]
void MyFilledCircle(Bitmap* inBitmap, Point center)
{
    circle(inBitmap->getCvMat(),
        center,
        w / 32,
        Scalar(0, 0, 255),
        FILLED,
        LINE_8);
}
//![my_filled_circle]

/**
 * @function MyPolygon
 * @brief Draw a simple concave polygon (rook)
 */
 //![my_polygon]
void MyPolygon(Mat img)
{
    int lineType = LINE_8;

    /** Create some points */
    Point rook_points[1][20];
    rook_points[0][0] = Point(w / 4, 7 * w / 8);
    rook_points[0][1] = Point(3 * w / 4, 7 * w / 8);
    rook_points[0][2] = Point(3 * w / 4, 13 * w / 16);
    rook_points[0][3] = Point(11 * w / 16, 13 * w / 16);
    rook_points[0][4] = Point(19 * w / 32, 3 * w / 8);
    rook_points[0][5] = Point(3 * w / 4, 3 * w / 8);
    rook_points[0][6] = Point(3 * w / 4, w / 8);
    rook_points[0][7] = Point(26 * w / 40, w / 8);
    rook_points[0][8] = Point(26 * w / 40, w / 4);
    rook_points[0][9] = Point(22 * w / 40, w / 4);
    rook_points[0][10] = Point(22 * w / 40, w / 8);
    rook_points[0][11] = Point(18 * w / 40, w / 8);
    rook_points[0][12] = Point(18 * w / 40, w / 4);
    rook_points[0][13] = Point(14 * w / 40, w / 4);
    rook_points[0][14] = Point(14 * w / 40, w / 8);
    rook_points[0][15] = Point(w / 4, w / 8);
    rook_points[0][16] = Point(w / 4, 3 * w / 8);
    rook_points[0][17] = Point(13 * w / 32, 3 * w / 8);
    rook_points[0][18] = Point(5 * w / 16, 13 * w / 16);
    rook_points[0][19] = Point(w / 4, 13 * w / 16);

    const Point* ppt[1] = { rook_points[0] };
    int npt[] = { 20 };

    fillPoly(img,
        ppt,
        npt,
        1,
        Scalar(255, 255, 255),
        lineType);
}
//![my_polygon]

/**
 * @function MyLine
 * @brief Draw a simple line
 */
 //![my_line]
void MyLine(Mat img, Point start, Point end)
{
    int thickness = 2;
    int lineType = LINE_8;

    line(img,
        start,
        end,
        Scalar(0, 0, 0),
        thickness,
        lineType);
}
//![my_line]