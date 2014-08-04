#include <algorithm>
#include <signal.h>
#include <cv.h>
#include <highgui.h>
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <list>
#include <unistd.h>

#include <gflags/gflags.h>

using namespace std;
using namespace cv;

DEFINE_bool(display, false, "Display video in window.");
DEFINE_bool(quiet, false, "Suppress terminal output.");
DEFINE_string(save, "", "Save output to file.");
DEFINE_string(axis, "y", "Axis of rotation.");
DEFINE_double(fps, 24, "Frame per second for output.");
DEFINE_double(threadcount, 4, "Thread count.");
DEFINE_int32(postrotate, 0, "Rotation of final frames to fix flipped source video.\n"
                            "\t0\tNo rotation\n"
                            "\t1\t90 degrees clockwise\n"
                            "\t2\t90 degrees counter-clockwise\n"
                            "\t3\t180 degrees\n"
                            );

enum postrotate_enum {
    ROT_NONE,
    ROT_90CW,
    ROT_90CCW,
    ROT_180
};

enum axis_enum {
    AXIS_X,
    AXIS_Y,
    AXIS_T
};

typedef struct options_struct {
    enum axis_enum axis;
    Size size;
    int frame_count;
    VideoCapture **video_srcs;
    VideoWriter *video_dst;
    enum postrotate_enum postrotate;
} options_type;

typedef struct {
    unsigned int thread_id;
    unsigned int thread_count;
    int current_frame;
    options_type *options;
    Mat frame_in;
    Mat frame_out;
} thread_data;

int video_file_filter(options_type *o, char *window_name);

options_type * create_options(int *argc, char ***argv) {
    google::ParseCommandLineFlags(argc, argv, true);
    options_type *o = (options_type *) calloc(1, sizeof(options_type));
    
    // Prerotation?
    o->postrotate = (enum postrotate_enum)FLAGS_postrotate;

    // open input video
    o->video_srcs = (VideoCapture**)malloc(sizeof(VideoCapture*) * FLAGS_threadcount);
    for (int i = 0; i < FLAGS_threadcount; i++) {
        string input_file = (*argv)[(*argc) - 1];
        o->video_srcs[i] = new VideoCapture(input_file);
        if(!o->video_srcs[i]->isOpened()) return NULL;
    }

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
    int width = o->video_srcs[0]->get(CV_CAP_PROP_FRAME_WIDTH);
    int height = o->video_srcs[0]->get(CV_CAP_PROP_FRAME_HEIGHT);
    int frame_count = o->video_srcs[0]->get(CV_CAP_PROP_FRAME_COUNT);
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
        if (o->postrotate == ROT_NONE || o->postrotate == ROT_180) {
            o->video_dst->open(FLAGS_save, ex, FLAGS_fps, o->size);
        } else {
            o->video_dst->open(FLAGS_save, ex, FLAGS_fps, Size(o->size.height, o->size.width));
        }
    } else {
        cout << "Output file required." << endl;
        return NULL;
    }

    return o;

}

int main(int argc, char **argv) {
    setbuf(stdout, NULL);

    google::SetUsageMessage("Rotate video in time.");

    options_type *o = create_options(&argc, &argv);
    if (o == NULL)
        return -1;

    if (FLAGS_display) {
        namedWindow(argv[0],CV_WINDOW_NORMAL);
        setWindowProperty(argv[0], CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);
    }

    video_file_filter(o, argv[0]);

    for (int i = 0; i < FLAGS_threadcount; i++) {
        delete o->video_srcs[i];
    }
    free(o->video_srcs);
    delete o->video_dst;
    free(o);

    return 0;
}

void *rotate_frame_(void *vdata) {
    thread_data *data = (thread_data*)vdata;
    options_type *o = data->options;
    int j = data->current_frame;
    VideoCapture *vs = o->video_srcs[data->thread_id];
    int frame_count_src = vs->get(CV_CAP_PROP_FRAME_COUNT);
    vs->set(CV_CAP_PROP_POS_FRAMES, 0);
    for (int i = data->thread_id; i < frame_count_src; i+=data->thread_count) {
        *(vs) >> data->frame_in;
        switch (o->axis) {
            case AXIS_X:
                data->frame_in.row(j).copyTo(data->frame_out.row(i));
                break;
            case AXIS_Y:
                data->frame_in.col(j).copyTo(data->frame_out.col(i));
                break;
            default:
                cout << "UNIMPLEMENTED" << endl;
                return NULL;
        }
    }
}

void rotate_frame(options_type *o, int current_frame, Mat fin, Mat fout) {
    int thread_count = FLAGS_threadcount;
    pthread_t threads[thread_count];
    int irets[thread_count];
    thread_data data[thread_count];

    for (unsigned int i = 0; i < thread_count; i++) {
        data[i] = thread_data{i, thread_count, current_frame, o, fin, fout};
        irets[i] = pthread_create(&threads[i], NULL, rotate_frame_, &data[i]);
        if (irets[i]) {
            fprintf(stderr,"Error - pthread_create() return code: %d\n",irets[i]);
            exit(EXIT_FAILURE);
        }
    }

    for (unsigned int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
    }
}

int video_file_filter(options_type *o, char *window_name) {
    Mat frame_in;
    *(o->video_srcs[0]) >> frame_in;
    Mat frame_out(o->size, frame_in.type());

    int frame_count_src = o->video_srcs[0]->get(CV_CAP_PROP_FRAME_COUNT);
    for (int j = 0; j < o->frame_count; j++) {

        // Output status to console
        if (!FLAGS_quiet)
            printf("Frame %d of %d...",j + 1, o->frame_count);

        rotate_frame(o, j, frame_in, frame_out);

        if (o->postrotate != ROT_NONE) {
            printf("postrotation...");
        }

        // Do any postrotation required before finishing frame.
        Mat f;
        if (o->postrotate == ROT_90CW) {
            transpose(frame_out, f);
            flip(f, f, 1);
        } else if (o->postrotate == ROT_90CCW) {
            transpose(frame_out, f);
            flip(f, f, 0);
        } else if (o->postrotate == ROT_180) {
            flip(frame_out, f, -1);
        } else {
            f = frame_out;
        }

        // Display window
        if (FLAGS_display) {
            imshow(window_name, f);
            if (waitKey(1) == '')
                return 0;
        }

        // Save frame to file
        if (!FLAGS_save.empty()) {
            printf("writing...");
            o->video_dst->write(f);
        }
        
        printf("done.\n");

    }
}
