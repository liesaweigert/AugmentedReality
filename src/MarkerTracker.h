#ifndef MAR_TEST_MARKERTRACKER_H
#define MAR_TEST_MARKERTRACKER_H

#include <stdlib.h>
#include <opencv2/opencv.hpp>
#include "PoseEstimation.h"
#include <iomanip>
#include "DrawPrimitives.h"

using namespace std;
using namespace cv;

struct Marker{
    int marker_code;
    Eigen::Matrix4f marker_matrix;
    bool visible;
};

class MarkerTracker {

    public:
        void find (Mat& image, Marker* buttons, int buttons_count);

    private:
        int sampleSubPix(const Mat &point_source, const Point2f &p);
        Point2f intersection(Vec4f line1, Vec4f line2);
};


#endif //MAR_TEST_MARKERTRACKER_H
