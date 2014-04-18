#include <cv.h>
#include <highgui.h>
#include <iostream>
#include <gflags/gflags.h>

using namespace std;
using namespace cv;

DEFINE_bool(display, true, "Display video in window.");
DEFINE_bool(save, false, "Save output to disk.");

int main(int argc, char **argv)
{
    google::ParseCommandLineFlags(&argc, &argv, true);

    //VideoCapture cap(0);
    VideoCapture cap("video.mp4");
    if(!cap.isOpened()) return -1;

    Mat frame_in;
    cap >> frame_in;
    Mat frame_out(frame_in.size(), frame_in.type()); // don't copy
    cap.set(CV_CAP_PROP_POS_FRAMES, 0);

    int width = cap.get(CV_CAP_PROP_FRAME_WIDTH);
    int height = cap.get(CV_CAP_PROP_FRAME_HEIGHT);
    int frame_count = cap.get(CV_CAP_PROP_FRAME_COUNT);
    
    frame_out.resize(frame_count);
    for (int i = 0; i < frame_count; i++) {
        frame_in.row(0).copyTo(frame_out.row(i));
    }

    if (FLAGS_display)
        namedWindow("frame",1);
    for (int j = 0; j < height; j++)
    {
        for (int i = 0; i < frame_count; i++) {
            cap >> frame_in;
            frame_in.row(j).copyTo(frame_out.row(i));
        }
        cap.set(CV_CAP_PROP_POS_FRAMES, 0);

        if (FLAGS_display)
            imshow("frame", frame_out);
        if(waitKey(30) >= 0) break;
    }
    return 0;
}
