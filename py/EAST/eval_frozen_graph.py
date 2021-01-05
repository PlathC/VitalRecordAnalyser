# USAGE
# python opencv_text_detection_image.py --image images/lebron_james.jpg
# --east frozen_east_text_detection.pb

# import the necessary packages
from imutils.object_detection import non_max_suppression
import numpy as np
import argparse
import time
import cv2
import lanms
from icdar import restore_rectangle_rbox


def resize_image(im, max_side_len=2400):
    '''
    resize image to a size multiple of 32 which is required by the network
    :param im: the resized image
    :param max_side_len: limit of max image size to avoid out of memory in gpu
    :return: the resized image and the resize ratio
    '''
    h, w, _ = im.shape

    resize_w = w
    resize_h = h

    # limit the max side
    if max(resize_h, resize_w) > max_side_len:
        ratio = float(max_side_len) / resize_h if resize_h > resize_w else float(max_side_len) / resize_w
    else:
        ratio = 1.
    resize_h = int(resize_h * ratio)
    resize_w = int(resize_w * ratio)

    resize_h = resize_h if resize_h % 32 == 0 else (resize_h // 32 - 1) * 32
    resize_w = resize_w if resize_w % 32 == 0 else (resize_w // 32 - 1) * 32
    resize_h = max(32, resize_h)
    resize_w = max(32, resize_w)
    im = cv2.resize(im, (int(resize_w), int(resize_h)))

    ratio_h = resize_h / float(h)
    ratio_w = resize_w / float(w)

    return im, (ratio_h, ratio_w)


def detect(score_map, geo_map, timer, score_map_thresh=0.8, box_thresh=0.1, nms_thres=0.2):
    '''
    restore text boxes from score map and geo map
    :param score_map:
    :param geo_map:
    :param timer:
    :param score_map_thresh: threshhold for score map
    :param box_thresh: threshhold for boxes
    :param nms_thres: threshold for nms
    :return:
    '''
    if len(score_map.shape) == 4:
        score_map = score_map[0, 0, :, :]
        geo_map = geo_map[0, :, :, ]
    # filter the score map
    xy_text = np.argwhere(score_map > score_map_thresh)
    # sort the text boxes via the y axis
    xy_text = xy_text[np.argsort(xy_text[:, 0])]
    # restore
    start = time.time()
    text_box_restored = restore_rectangle_rbox(xy_text[:, ::-1] * 4,
                                               np.transpose(geo_map[:, xy_text[:, 0], xy_text[:, 1]]))  # N*4*2
    print('{} text boxes before nms'.format(text_box_restored.shape[0]))
    boxes = np.zeros((text_box_restored.shape[0], 9), dtype=np.float32)
    boxes[:, :8] = text_box_restored.reshape((-1, 8))
    boxes[:, 8] = score_map[xy_text[:, 0], xy_text[:, 1]]
    timer['restore'] = time.time() - start
    # nms part
    start = time.time()
    # boxes = nms_locality.nms_locality(boxes.astype(np.float64), nms_thres)
    boxes = lanms.merge_quadrangle_n9(boxes.astype('float32'), nms_thres)
    timer['nms'] = time.time() - start

    if boxes.shape[0] == 0:
        return None, timer

    # here we filter some low score boxes by the average score map, this is different from the orginal paper
    for i, box in enumerate(boxes):
        mask = np.zeros_like(score_map, dtype=np.uint8)
        cv2.fillPoly(mask, box[:8].reshape((-1, 4, 2)).astype(np.int32) // 4, 1)
        boxes[i, 8] = cv2.mean(score_map, mask)[0]
    boxes = boxes[boxes[:, 8] > box_thresh]

    return boxes, timer


def sort_poly(p):
    min_axis = np.argmin(np.sum(p, axis=1))
    p = p[[min_axis, (min_axis + 1) % 4, (min_axis + 2) % 4, (min_axis + 3) % 4]]
    if abs(p[0, 0] - p[1, 0]) > abs(p[0, 1] - p[1, 1]):
        return p
    else:
        return p[[0, 3, 2, 1]]


# construct the argument parser and parse the arguments
ap = argparse.ArgumentParser()
ap.add_argument("-i", "--image", type=str,
                default='./images/1902Page9_2.png', help="path to input image")
ap.add_argument("-east", "--east", type=str,
                default='./frozen_graph.pb', help="path to input EAST text detector")
ap.add_argument("-c", "--min-confidence", type=float, default=0.5,
                help="minimum probability required to inspect a region")
ap.add_argument("-w", "--width", type=int, default=320,
                help="resized image width (should be multiple of 32)")
ap.add_argument("-e", "--height", type=int, default=320,
                help="resized image height (should be multiple of 32)")
args = vars(ap.parse_args())

im = cv2.imread(args["image"])
im_resized, (ratio_h, ratio_w) = resize_image(im)
(height, width) = im_resized.shape[:2]

# define the two output layer names for the EAST detector model that
# we are interested -- the first is the output probabilities and the
# second can be used to derive the bounding box coordinates of text
layerNames = [
    "feature_fusion/Conv_7/Sigmoid",
    "feature_fusion/concat_3"]

# load the pre-trained EAST text detector
print("[INFO] loading EAST text detector...")
net = cv2.dnn.readNetFromTensorflow('./frozen_graph.pb')

# construct a blob from the image and then perform a forward pass of
# the model to obtain the two output layer sets
# blob = cv2.dnn.blobFromImage(im_resized, 1.0, (width, height),
#                              (123.68, 116.78, 103.94), swapRB=True, crop=False)

blob = cv2.dnn.blobFromImage(im_resized, size=(width, height), swapRB=True, crop=False)

timer = {'net': 0, 'restore': 0, 'nms': 0}
start = time.time()
net.setInput(blob)
(scores, geometry) = net.forward(layerNames)
timer['net'] = time.time() - start
boxes, timer = detect(score_map=scores, geo_map=geometry, timer=timer)
end = time.time()

if boxes is not None:
    boxes = boxes[:, :8].reshape((-1, 4, 2))
    boxes[:, :, 0] /= ratio_w
    boxes[:, :, 1] /= ratio_h

for box in boxes:
    # to avoid submitting errors
    box = sort_poly(box.astype(np.int32))
    if np.linalg.norm(box[0] - box[1]) < 5 or np.linalg.norm(box[3] - box[0]) < 5:
        continue
    # f.write('{},{},{},{},{},{},{},{}\r\n'.format(
    #     box[0, 0], box[0, 1], box[1, 0], box[1, 1], box[2, 0], box[2, 1], box[3, 0], box[3, 1],
    # ))
    test = box.astype(np.int32).reshape((-1, 2))
    cv2.polylines(im, [box.astype(np.int32).reshape((-1, 1, 2))], True, color=(255, 0, 0),
                  thickness=1)
# show timing information on text prediction
print("[INFO] text detection took {:.6f} seconds".format(end - start))
print(timer)

# show the output image
im = cv2.resize(im, (1080, 720))
cv2.imshow("Text Detection", im)
cv2.waitKey(0)
