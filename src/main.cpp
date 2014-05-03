#include <cv.h>
#include <highgui.h>
#include <iostream>
#include <gflags/gflags.h>
#include <cstdlib>

using namespace std;
using namespace cv;

DEFINE_bool(display, true, "Display video in window.");
DEFINE_bool(quiet, false, "Suppress terminal output.");
DEFINE_string(save, "", "Save output to file.");
DEFINE_string(axis, "y", "Axis of rotation.");

enum axis_enum {
    AXIS_X,
    AXIS_Y,
    AXIS_T
};

typedef struct options_struct {
    enum axis_enum axis;
    Size size;
    int frame_count;
    VideoCapture *video_src;
    VideoWriter *video_dst;
} options_type;

options_type * create_options(int *argc, char ***argv) {
    google::ParseCommandLineFlags(argc, argv, true);
    options_type *o = (options_type *) calloc(1, sizeof(options_type));
    
    if (*argc != 2) {
        cout << "usage: " << *argv[0] << " input-video" << endl;
        free(o);
        return NULL;
    }

    // open input video
    string input_file = (*argv)[(*argc) - 1];
    o->video_src = new VideoCapture(input_file);
    if(!o->video_src->isOpened()) return NULL;

    // get axis
    if (FLAGS_axis == "x") {
        o->axis = AXIS_X;
    } else if (FLAGS_axis == "y") {
        o->axis = AXIS_Y;
    } else if (FLAGS_axis == "t") {
        o->axis = AXIS_T;
    } else {
        free(o);
        return NULL;
    }

    // get size of output video
    int width = o->video_src->get(CV_CAP_PROP_FRAME_WIDTH);
    int height = o->video_src->get(CV_CAP_PROP_FRAME_HEIGHT);
    int frame_count = o->video_src->get(CV_CAP_PROP_FRAME_COUNT);
    if (FLAGS_axis == "x") {
        o->size = Size(width, frame_count);
        o->frame_count = height;
    } else if (FLAGS_axis == "y") {
        o->size = Size(frame_count, height);
        o->frame_count = width;
    } else {
        free(o);
        return NULL;
    }

    // get output video
    if (!FLAGS_save.empty()) {
        o->video_dst = new VideoWriter;
        int ex = CV_FOURCC('I', 'Y', 'U', 'V');
        o->video_dst->open(
                FLAGS_save, ex,
                o->video_src->get(CV_CAP_PROP_FPS),
                o->size);
    }

    return o;

}

int main(int argc, char **argv) {

    options_type *o = create_options(&argc, &argv);

    Mat frame_in;
    *(o->video_src) >> frame_in;
    Mat frame_out(o->size, frame_in.type());

    if (FLAGS_display) {
        namedWindow(argv[0],CV_WINDOW_NORMAL);
        setWindowProperty(argv[0], CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);
    }

    int frame_count_src = o->video_src->get(CV_CAP_PROP_FRAME_COUNT);
    for (int j = 0; j < o->frame_count; j++) {

        // Output status to console
        if (!FLAGS_quiet)
            cout << "Frame " << j+1 << " of " << o->frame_count << "." << endl;

        // Create a new frame.
        o->video_src->set(CV_CAP_PROP_POS_FRAMES, 0);
        for (int i = 0; i < frame_count_src; i++) {
            *(o->video_src) >> frame_in;
            switch (o->axis) {
                case AXIS_X:
                    frame_in.row(j).copyTo(frame_out.row(i));
                    break;
                case AXIS_Y:
                    frame_in.col(j).copyTo(frame_out.col(i));
                    break;
                default:
                    cout << "UNIMPLEMENTED" << endl;
                    return -1;
            }
        }

        // Display window
        if (FLAGS_display) {
            imshow(argv[0], frame_out);
            waitKey(30);
        }

        // Save frame to file
        if (!FLAGS_save.empty())
            o->video_dst->write(frame_out);

    }

    delete o->video_src;
    delete o->video_dst;
    free(o);

    return 0;
}
