#!/usr/bin/env python3

import scipy.io
import numpy as np
import cv2

# Return the scaled normal vector of the line P1P2.
# P1 and P2: two 2D points (x, y)
# scale the scaling factor needed to give marging to the oriented bounding box.
def norm(P1, P2, scale):
  length = np.sqrt((P1[0] - P2[0])**2 + (P1[1] - P2[1])**2)
  dx = P2[0] - P1[0]
  dy = P2[1] - P1[1]
  N = [-dy * scale / length, dx * scale / length]
  return N

# Crop the .jpg image indicated by the argument 'path' following the given segments
# given in the .mat file of the same name
def crop(folder, file):
  path = folder + file

  # Debug image: in where we are drawing debug lines
  debug = cv2.imread(path + '.jpg')

  # The image to crop. Note: 0 is to force the image to be in grey (optional)
  img = cv2.imread(path + '.jpg', 0)

  # Read the Matlab file in where points (marks) and segments (slots) are defines
  mat = scipy.io.loadmat(path + '.mat')
  count_slots = int(np.size(mat['slots']) / 4) # 4 fields by components

  # Extract all segments (slots)
  for i in range(count_slots):
    segment = mat['slots'][i]
    P1 = mat['marks'][segment[0] - 1] # -1 because Matlab starting counting from 1 and Python from 0
    P2 = mat['marks'][segment[1] - 1] # 0: X coordinate and 1: Y coordinate

    # Margin of the bounding box (in pixel)
    margin = 24

    # Compute the scaled normal of the line P1P2
    # With N we can obtain a bounding rectangle with only marges along the normal
    # rectangle = [P1 - N, P1 + N, P2 + N, P2 - N]
    N = norm(P1, P2, margin)

    # Now, we have to enlarge the other side of the rectangle, so we compute the
    # second scaled norm.
    M = norm(P1, P1 + N, margin)

    # Oriented bounding box (rectangle)
    rectangle = [P1 - N + M, P1 + N + M, P2 + N - M, P2 - N - M]

    # Debug: draw the rectangle in the debug picture
    pts = np.array(rectangle, np.int32).reshape((-1, 1, 2))
    debug = cv2.polylines(debug, [pts], True, (255, 0, 0), 2)

    # Crop and reshape the oriented bounding box to an image of dimension width x height.
    # We ask OpenCV to compute the transformation matrix.
    width, height = 192, 48
    src_pts = np.array(rectangle, dtype="float32")
    dst_pts = np.array([[0, height-1], [0, 0], [width-1, 0], [width-1, height-1]], dtype="float32")
    T = cv2.getPerspectiveTransform(src_pts, dst_pts)
    warped = cv2.warpPerspective(img, T, (width, height))
    cv2.imwrite(folder + 'output' + str(i) + '.jpg', warped)

  cv2.imwrite(folder + 'debug.jpg', debug)

# Example
crop('doc/', 'input')
