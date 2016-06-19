#include "MarkerTracker.h"

void MarkerTracker::find (Mat& img_bgr, Marker* buttons, int buttons_count){

    Mat img_gray;

    cvtColor(img_bgr, img_gray, CV_BGR2GRAY);
    threshold(img_gray, img_gray, 100, 255, THRESH_BINARY);

    vector<vector<Point>> contours;
    vector<vector<Point>> approx_contours;
    vector<Vec4i> hierarchy;
    Mat img_contours = img_gray.clone();

    CvScalar red = Scalar(0, 0, 255);
    CvScalar blue = Scalar(255, 0, 0);
    Scalar white = Scalar(255, 255, 255);

    findContours(img_contours, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0,0));

    approx_contours.resize(contours.size());

    bool first_stripe = true;
    bool first_marker = true;

    //We assume all markers are not visible at the beginning
    for (int i = 0; i < buttons_count; i++){
        buttons[i].visible = false;
    }

    for(int i = 0; i < contours.size(); i++) {
        approxPolyDP(Mat(contours[i]), approx_contours[i], 3, true);
        Rect rect = boundingRect(approx_contours[i]);

        //check whether the rectangle is logically big (lower bounds)
        if(rect.height > 30 && rect.width > 30){

            if(approx_contours[i].size() != 4){
                continue;
            }

            //rendering the contours in the image
            polylines(img_bgr, approx_contours[i], true, white, 3);

            Vec4f edge_lines[4];

            for(int j = 0; j < 4; j++){
                circle(img_bgr, approx_contours[i][j], 4, red, 4);

                //calculate distanced in x and y direction for vectors
                int dx = approx_contours[i][(j + 1) % 4].x - approx_contours[i][j].x;
                int dy = approx_contours[i][(j + 1) % 4].y - approx_contours[i][j].y;

                //calculate stripe size
                int stripe_length = (int)(sqrt(dx * dx + dy * dy) / 7);
                if (stripe_length < 5){
                    stripe_length = 5;
                }
                int stripe_width = 3;
                Size stripe_size = Size(stripe_width, stripe_length);

                //calculate vectors in edge and stripe direction
                Point2f vector_edge, vector_stripe;
                //using the normalization for unit vectors
                double vector_length = sqrt(dx * dx + dy * dy);

                //vector_length cannot be zero, because trivial
                vector_edge.x = dx / vector_length;
                vector_edge.y = dy / vector_length;
                vector_stripe.x = vector_edge.y;
                vector_stripe.y = - vector_edge.x;

                vector<Point2f> exact_edges(6);

                for(int k = 1; k < 7; k++){
                    Point dot = approx_contours[i][j] + ((approx_contours[i][(j + 1) % 4] - approx_contours[i][j]) * k / 7);

                    //visualization of separating dot
                    circle(img_bgr, dot, 3, blue, 2);

                    //calculation of the exact(subpixel accuracy) position of the edges
                    Mat stripe (stripe_size, CV_8UC1);
                    int half_stripe_length = stripe_length >> 1;

                    Point2f starting_point;
                    starting_point.x = (double)approx_contours[i][j].x + k * dx / 7.0;
                    starting_point.y = (double)approx_contours[i][j].y + k * dy / 7.0;


                    for (int width = -1; width <= 1; width++){
                        for (int length = -half_stripe_length; length < half_stripe_length; length++){
                            Point2f current_point;
                            current_point.x = starting_point.x + width * vector_edge.x + length * vector_stripe.x;
                            current_point.y = starting_point.y + width * vector_edge.y + length * vector_stripe.y;

                            int current_value = sampleSubPix(img_gray, current_point);
                            stripe.at<uchar>(length + half_stripe_length, width + 1) = (uchar)current_value;
                        }
                    }


                    //apply the sobel operator on the stripe
                    vector<int> sobel_stripe(stripe_length - 2);
                    for (int n = 1; n < stripe_length - 1; n++){
                        //first row of sobel matrix using the (n-1)th row of the stripe
                        unsigned char* stripe_row = &(stripe.at<uchar>(n - 1, 0));
                        double first_row = - stripe_row[0] - 2 * stripe_row[1] - stripe_row[2];

                        //third row of sobel matrix using the (n+1)th row of the stripe
                        stripe_row = &(stripe.at<uchar>(n + 1, 0));
                        double third_row = stripe_row[0] + 2 * stripe_row[1] + stripe_row[2];
                        sobel_stripe[n - 1] = first_row + third_row;
                    }

                    if (first_stripe) {
                        //display the stripe in a new window
                        namedWindow("Stripe", WINDOW_NORMAL);

                        Mat img_stripe((int)sobel_stripe.size(), 1, CV_8UC1);
                        for (int n = 0; n < sobel_stripe.size(); n++){
                            img_stripe.at<uchar>(n, 0) = (uchar)sobel_stripe[n];
                        }
                        normalize(img_stripe, img_stripe, 0, 255, CV_MINMAX, CV_8UC1);
                        resize(img_stripe, img_stripe, Size(), 50, 30, INTER_NEAREST);
                        imshow("Stripe", img_stripe);

                        first_stripe = false;
                    }

                    //finding the maximum pixel in the sobel_values
                    int max_value = -1;
                    int max_pixel_index = 0;
                    for (int n = 0; n < (stripe_length - 2); n++){
                        if(sobel_stripe[n] > max_value){
                            max_value = sobel_stripe[n];
                            max_pixel_index = n;
                        }
                    }

                    //calculate exact edge position
                    int y_minus_1 = (max_pixel_index > 0) ? sobel_stripe[max_pixel_index - 1] : 0;
                    int y_zero = max_value;
                    int y_plus_1 = (max_pixel_index < (stripe_length - 2)) ? sobel_stripe[max_pixel_index + 1] : 0;

                    //we use the quadratic equation y = ax^2 + bx + c with maxvalue as x = 0
                    int c = y_zero;
                    //y_plus_1 = a + b
                    //y_minus_1 = a - b
                    // => y_plus_1 + y_minus_1 = 2a
                    double a = (double)(y_plus_1 + y_minus_1) / 2.0;
                    // => y_plus_1 - y_minus_1 = 2b
                    double b = (double)(y_plus_1 - y_minus_1) / 2.0;

                    //search for y_max with 2 * a * xmax + b = 0
                    double x_max = - b / (2.0 * a);

                    //now we can save the final point with subpixel accuracy
                    double x_max_index = x_max - (double)half_stripe_length;
                    Point2f final_point;
                    final_point.x = starting_point.x + x_max_index * vector_stripe.x;
                    final_point.y = starting_point.y + x_max_index * vector_stripe.y;

                    exact_edges[k - 1] = final_point;

                    //And now visualization, so I know I am hecka smart
                    circle(img_bgr, final_point, 3, white, 1);
                } // end of iteration through 7 stripes

                //Mat exact_points(Size(1,6), CV_32FC2, exact_edge_points);
                fitLine(exact_edges, edge_lines[j], CV_DIST_L2, 0, 0.01, 0.01);

                //define line through two points x1 and x2
                Point x1;
                x1.x = edge_lines[j][2] - (int)(50 * edge_lines[j][0]);
                x1.y = edge_lines[j][3] - (int)(50 * edge_lines[j][1]);

                Point x2;
                x2.x = edge_lines[j][2] + (int)(50 * edge_lines[j][0]);
                x2.y = edge_lines[j][3] + (int)(50 * edge_lines[j][1]);

                //visualize the lines
                line(img_bgr, x1, x2, blue, 1, 8, 0);
            } //j free - end of iteration through 4 edges of the marker

            Point2f exact_corners[4];

            //now compute the intersections of the lines
            for (int j = 0; j < 4; j++){
                exact_corners[j] = intersection(edge_lines[j], edge_lines[(j + 1) % 4]);
                circle(img_bgr, exact_corners[j], 5, white, 3);
            }

            //calculating the perspective transformation
            Point2f points[4] = { Point2f(-0.5, -0.5), Point2f(-0.5, 5.5), Point2f(5.5, 5.5), Point2f(5.5, -0.5)};
            Mat transformation = getPerspectiveTransform(exact_corners, points);

            //creating a new 6x6 image
            Mat img_marker;
            Mat img_marker_resized;
            warpPerspective(img_gray, img_marker, transformation, Size(6,6), INTER_LINEAR);
            threshold(img_marker, img_marker, 100, 255, THRESH_BINARY);

            //check whether the markers have a black border
            bool black_border = true;
            for (int j = 0; j < 6; j++){
                if (img_marker.at<uchar>(j,0) != 0) black_border = false;
                if (img_marker.at<uchar>(0,j) != 0) black_border = false;
                if (img_marker.at<uchar>(j,5) != 0) black_border = false;
                if (img_marker.at<uchar>(5,j) != 0) black_border = false;
            }

            //only analyse the marker if it has a black border
            if (black_border){

                uint16_t marker_ids[4];
                int min_marker_rotation = 0;

                if (first_marker) {
                    //display the marker in a new window
                    namedWindow("Marker", WINDOW_NORMAL);

                    resize(img_marker, img_marker_resized, Size(), 50, 50, INTER_NEAREST);
                    imshow("Marker", img_marker_resized);

                    first_marker = false;
                }

                //save the marker-ids (0 = white, 1 = black)
                for (int j = 0; j < 4; j++){ //four rotations
                    for (int x = 1; x < 5; x++) { // first direction
                        for (int y = 1; y < 5; y++){

                            int xx = (j % 2 == 0) ? ((j == 0) ? x : (5 - x)) : ((j == 1) ? y : 5 - y);
                            int yy = (j % 2 == 0) ? ((j == 0) ? y : (5 - y)) : ((j == 1) ? 5 - x : x);

                            marker_ids[j] <<= 1;

                            if (img_marker.at<uchar>(xx, yy) == '\0'){
                                marker_ids[j] |= 1;
                            }

                        }

                    }
                }

                //let's filter out all black markers
                if(marker_ids[0] != 65535) {

                    //now, we find the minimum marker_id
                    uint16_t min_marker_id = marker_ids[0];
                    for (int j = 1; j < 4; j++) {
                        if (marker_ids[j] < min_marker_id) {
                            min_marker_id = marker_ids[j];
                            min_marker_rotation = j;
                        }
                    }

                    int current_button = -1;
                    bool searched_button = false;

                    //the markers we are searching are saved in buttons
                    //for each button, we see whether the correct marker has been found
                    for(int m = 0; m < buttons_count; m++){
                        if (min_marker_id == buttons[m].marker_code){
                            current_button = m;
                            searched_button = true;
                            continue;
                        }
                    }
                    if (searched_button) {

                        cout << "The marker rotation: " << min_marker_rotation << ", Marker: " << hex <<
                        min_marker_id << "\n";

                        //correct order of corners by found out order
                        Point2f temp_corners[4];

                        for (int j = 0; j < 4; j++) {
                            temp_corners[(j+ min_marker_rotation) % 4] = exact_corners[j];
                        }


                        for (int j = 0; j < 4; j++) {
                            exact_corners[j] = temp_corners[j];
                            //compute the coordinates of the markers in screen space
                            exact_corners[j].x -= 640;
                            exact_corners[j].y = -exact_corners[j].y + 360;
                        }


                        //now use given function EstimateSquarePose
                        Eigen::Matrix4f marker_matrix;
                        estimateSquarePose(marker_matrix, exact_corners, 0.048182);

                        buttons[current_button].marker_matrix = marker_matrix;
                        buttons[current_button].visible = true;
                    }
                }
            }
        }
    }
}

int MarkerTracker::sampleSubPix(const Mat &point_source, const Point2f &point) {

    int x = int(floorf(point.x));
    int y = int(floorf(point.y));

    if (x < 0 || x >= point_source.cols - 1 || y < 0 || y >= point_source.rows - 1) {
        return 127;
    }

    int dx = int(256 * (point.x - floorf(point.x)));
    int dy = int(256 * (point.y - floorf(point.y)));

    unsigned char* i = (unsigned char*)((point_source.data + y * point_source.step) + x);
    int a = i[0] + ((dx * (i[1] - i[0])) / 256);
    i += point_source.step;
    int b = i[0] + ((dx * (i[1] - i[0])) / 256);
    return a + ((dy * (b - a)) / 256);
}

Point2f MarkerTracker::intersection(Vec4f line1, Vec4f line2){
    Point2f intersect_point;

    //the lines are given in the form (vx, vy, px py)
    Point2f l1_p1;
    l1_p1.x = line1[2];
    l1_p1.y = line1[3];

    Point2f l1_p2;
    l1_p2.x = line1[2] + line1[0];
    l1_p2.y = line1[3] + line1[1];

    Point2f l2_p1;
    l2_p1.x = line2[2];
    l2_p1.y = line2[3];

    Point2f l2_p2;
    l2_p2.x = line2[2] + line2[0];
    l2_p2.y = line2[3] + line2[1];

    Point2f x = l2_p1 - l1_p1;
    Point2f d1 = l1_p2 - l1_p1;
    Point2f d2 = l2_p2 - l2_p1;

    float cross = d1.x * d2.y - d1.y * d2.x;
    if (abs(cross) < /*EPS*/1e-8)
        return Point2f(0,0);

    double t1 = (x.x * d2.y - x.y * d2.x)/cross;
    intersect_point = l1_p1 + d1 * t1;

    return intersect_point;
}