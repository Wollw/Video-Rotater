import cv2
import numpy
from collections import deque

queue = deque([])

vc = cv2.VideoCapture(0);

cv2.namedWindow('frame',cv2.WINDOW_NORMAL)
cv2.setWindowProperty('frame',
        cv2.WND_PROP_FULLSCREEN,
        cv2.cv.CV_WINDOW_FULLSCREEN)

height = int(vc.get(cv2.cv.CV_CAP_PROP_FRAME_HEIGHT))

while len(queue) < height:
    queue.append(vc.read()[1])
    print "Loading frame {0} of {1}".format(len(queue), height)


frame = queue[0]

while 1:
    for i in range(0,height):
        frame[i] =  queue[i][i].copy()
    cv2.imshow('frame', frame)
    cv2.waitKey(30)
    queue.popleft()
    queue.append(vc.read()[1])
