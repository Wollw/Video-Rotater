#!/usr/bin/env python2
import cv2
import numpy
from collections import deque

cv2.namedWindow('frame',cv2.WINDOW_NORMAL)
cv2.setWindowProperty('frame',
        cv2.WND_PROP_FULLSCREEN,
        cv2.cv.CV_WINDOW_FULLSCREEN)

# Open the web cam and get the pixel height of the video feed.
vc = cv2.VideoCapture(0);
height = int(vc.get(cv2.cv.CV_CAP_PROP_FRAME_HEIGHT))

# Buffer enough frames to fill each row of the video.
queue = deque([])
while len(queue) < height:
    queue.append(vc.read()[1])
    print "Loading frame {0} of {1}".format(len(queue), height)

# Reusing the first frame as our destination frame.
frame = queue[0]

while 1:
    #
    for i in range(0,height):
        frame[i] =  queue[i][i].copy()
    cv2.imshow('frame', frame)
    if cv2.waitKey(1) == ord(''):
        break
    queue.popleft()
    queue.append(vc.read()[1])
