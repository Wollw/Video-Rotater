#include <cv.h>
#include <highgui.h>
#include <iostream>
#include <gflags/gflags.h>

using namespace std;
using namespace cv;

DEFINE_bool(display, true, "Display video in window.");
DEFINE_bool(verbose, false, "Display video in window.");
DEFINE_string(save, "", "Save output to file.");

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
    Mat frame_out(frame_in.size(), frame_in.type()); // don't copy
    vc.set(CV_CAP_PROP_POS_FRAMES, 0);

    int width = vc.get(CV_CAP_PROP_FRAME_WIDTH);
    int height = vc.get(CV_CAP_PROP_FRAME_HEIGHT);
    int frame_count = vc.get(CV_CAP_PROP_FRAME_COUNT);
    
    frame_out.resize(frame_count);
    for (int i = 0; i < frame_count; i++) {
        frame_in.row(0).copyTo(frame_out.row(i));
    }

    if (FLAGS_display)
        namedWindow(argv[0],1);

    VideoWriter vw;
    if (!FLAGS_save.empty()) {
        int ex = CV_FOURCC('I', 'Y', 'U', 'V');
        Size s = Size(width, frame_count);
        vw.open(FLAGS_save, ex, vc.get(CV_CAP_PROP_FPS), s);
    }

    for (int j = 0; j < height; j++)
    {
        if (FLAGS_verbose)
            cout << "Frame " << j+1 << " of " << height << "." << endl;

        for (int i = 0; i < frame_count; i++) {
            vc >> frame_in;
            frame_in.row(j).copyTo(frame_out.row(i));
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
