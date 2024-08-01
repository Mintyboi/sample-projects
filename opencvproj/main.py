import cv2
import numpy as np

tmp = np.zeros((400, 600), dtype = "uint8")
# cv2.imshow("default black box", tmp)
cv2.line(tmp, (0,0), (100,50), (255,255,255))
# cv2.imshow("black box with line", tmp)
cv2.rectangle(tmp, (100,50), (180, 130), (255, 0, 0))
cv2.imshow("black box with rect", tmp)

cv2.waitKey()
cv2.destroyAllWindows()

# cv::Mat g(inBitmap->GetHeight(), inBitmap->GetWidth(), CV_8UC4, inBitmap->GetRawData());
# cv::Rect rect(0, 0, inBitmap->GetWidth(), inBitmap->GetHeight());

# cv::rectangle(g, rect, in_Color, cv::FILLED);