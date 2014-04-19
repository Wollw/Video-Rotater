#include <cv.h>
#include <highgui.h>
#include <iostream>
#include <gflags/gflags.h>

using namespace std;
using namespace cv;

DEFINE_bool(display, true, "Display video in window.");
DEFINE_bool(quiet, false, "Suppress terminal output.");
DEFINE_string(save, "output.avi", "Save output to file.");
DEFINE_string(axis, "y", "Axis of rotation.");

int main(int argc, char **argv)
{
    google::ParseCommandLineFlags(&argc, &argv, true);

    if (argc !=2) {
        cout << "usage: " << argv[0] << " input-video" << endl;
        return -1;
    }

    VideoCapture vc(argv[argc-1]);
    if(!vc.isOpened()) return -1;

    Mat frame_in;
    vc >> frame_in;
    vc.set(CV_CAP_PROP_POS_FRAMES, 0);

    int width = vc.get(CV_CAP_PROP_FRAME_WIDTH);
    int height = vc.get(CV_CAP_PROP_FRAME_HEIGHT);
    int frame_count = vc.get(CV_CAP_PROP_FRAME_COUNT);

    Size s;
    if (FLAGS_axis == "x") {
        s = Size(width, frame_count);
    } else if (FLAGS_axis == "y") {
        s = Size(frame_count, height);
    } else {
        cout << "Invalid axis of rotation.";
        return 1;
    }

    Mat frame_out(s, frame_in.type());

    if (FLAGS_display)
        namedWindow(argv[0],1);

    VideoWriter vw;
    if (!FLAGS_save.empty()) {
        int ex = CV_FOURCC('I', 'Y', 'U', 'V');
        vw.open(FLAGS_save, ex, vc.get(CV_CAP_PROP_FPS), s);
        cout << "Saving to " << FLAGS_save << endl;
    }

    int frame_out_count = FLAGS_axis == "x" ? height : width;
    for (int j = 0; j < frame_out_count; j++)
    {
        if (!FLAGS_quiet)
            cout << "Frame " << j+1 << " of " << frame_out_count << "." << endl;

        for (int i = 0; i < frame_count; i++) {
            vc >> frame_in;
            if (FLAGS_axis == "x")
                frame_in.row(j).copyTo(frame_out.row(i));
            else
                frame_in.col(j).copyTo(frame_out.col(i));
        }
        vc.set(CV_CAP_PROP_POS_FRAMES, 0);

        if (FLAGS_display) {
            imshow(argv[0], frame_out);
            if(waitKey(30) == 0x1B /* ESC */ ) break;
        }

        if (!FLAGS_save.empty())
            vw.write(frame_out);

    }
    return 0;
}
