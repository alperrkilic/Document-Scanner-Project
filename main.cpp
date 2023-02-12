#include <stdio.h>
#include <string>
#include <vector>
#include <algorithm>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

Mat imgOriginal, imgGray, imgCanny, imgThreshold, imgBlur, imgDil, imgErode, imgWarp, imgCrop;
vector<Point> initialPoints, docPoints;

float w = 420, h = 596;

vector<Point> reorder(vector<Point> points);
vector<Point> getContours(Mat image);
Mat preProcessing(Mat img);
void drawPoints(vector<Point> points, Scalar color);
Mat getWarp(Mat img, vector<Point> points, float w, float h);

int main(int argc, char **argv)
{
  string path = "C:\\OPEN-CV Projects\\Document Scanner Project\\Image-Processing\\paper.jpg";
  imgOriginal = imread(path);
  // Just to see image clearly resizing. Since resizing the image lessens the quality, we'll make this line as comment-line after we're done
  // resize(imgOriginal, imgOriginal, Size(), 0.5, 0.5);

  // Preprocessing
  imgThreshold = preProcessing(imgOriginal);

  // Get Contours - Biggest Rectangle
  initialPoints = getContours(imgThreshold);
  // drawPoints(initialPoints, Scalar(0, 0, 255)); // before reordering

  // Reorder Points
  docPoints = reorder(initialPoints);
  // drawPoints(docPoints, Scalar(0, 255, 0)); // after reordering

  // Warp
  imgWarp = getWarp(imgOriginal, docPoints, w, h);

  // Crop
  int cropVal = 10;
  Rect roi(cropVal, cropVal, w - (2 * cropVal), h - (2 * cropVal)); // creating the rectangle that is to be cropped from the image
  imgCrop = imgWarp(roi);

  imshow("Original Image", imgOriginal);
  imshow("Image Threshold", imgThreshold);
  imshow("Image Warp", imgWarp);
  imshow("Image Crop", imgCrop);
  waitKey(0);

  return 0;
}

Mat preProcessing(Mat img)
{
  cvtColor(img, imgGray, COLOR_BGR2GRAY); // apply gray color (COLOR_BGR2GRAY) to img and then store it into imgGray
  GaussianBlur(imgGray, imgBlur, Size(3, 3), 3, 0);
  Canny(imgBlur, imgCanny, 25, 75); // Finding edges

  Mat kernel = getStructuringElement(MORPH_RECT, Size(3, 3));
  dilate(imgCanny, imgDil, kernel);
  // erode(imgDil, imgErode, kernel);
  return imgDil;
}

vector<Point> getContours(Mat image)
{
  vector<vector<Point>> contours; // think of a vector, every element is a vector and this vectors has Point elements
  vector<Vec4i> hierarchy;
  findContours(image, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE); // openCV function to used for finding contours
  // function says, from the source 'image' find contours into variable contours that is declared above.

  vector<vector<Point>> conPoly(contours.size()); //
  vector<Rect> boundRect(contours.size());

  vector<Point> biggest; // finding points of the biggest rectangle and then storing it into the biggest variable
  int maxArea = 0;

  for (int i = 0; i < contours.size(); i++)
  {
    int area = contourArea(contours[i]); // finding contour areas, to avoid little ones controlling with if statement
    cout << area << endl;
    if (area > 1000)
    {
      float peri = arcLength(contours[i], true);
      approxPolyDP(contours[i], conPoly[i], 0.02 * peri, true);

      // Each conPoly element has a vector and that vector holds the points edges of a specified region and we're calculating conPoly
      // with approxPolyDP() function which takes contours as sources and conPoly as destination and 0.02 * peri is the epsilon which
      // is the parameter specifying the approximation accuracy (maximum distance between the original curve and its approximation).
      // And the last one true is boolean if true, approximated curve is closed
      // meaning that first and last vertices is connected (closed-region)

      // Since this program is designed to scan documents, I assume that the maximum area of a contour should be a paper. Therefore,
      // in the control statements the maximum area and a shape that is 4 edged with conPoly[i].size() function
      // every element of conPoly function was a vector and this vector holds the points of the enclosed area. so checking with conPoly is
      // appropriate with this logic.

      if (area > maxArea && conPoly[i].size() == 4)
      {
        // drawContours(imgOriginal, conPoly, i, Scalar(255, 0, 255), 3); //drawing lines around 4 edged shapes
        biggest = {conPoly[i][0], conPoly[i][1], conPoly[i][2], conPoly[i][3]}; // copying i'th vector's points into biggest vector that contains points
        maxArea = area;
      }

      // drawContours(imgOriginal, conPoly, i, Scalar(255, 0, 255), 3); // drawing all contours
      // rectangle(imgOriginal, boundRect[i].tl(), boundRect[i].br(), Scalar(0, 255, 0), 5);
    }
  }

  return biggest;
}

void drawPoints(vector<Point> points, Scalar color)
// this function is not used but shown for the purpose when called, circles the points of the edges and the color for circles
{

  for (int i = 0; i < points.size(); i++)
  {
    circle(imgOriginal, points[i], 30, color, FILLED);
    putText(imgOriginal, to_string(i), points[i], FONT_HERSHEY_PLAIN, 10, color, 5);
  }
}

vector<Point> reorder(vector<Point> points) // this part is explained in the readme section, ordering the element numbers
{
  vector<Point> newPoints;
  vector<int> sumPoints, subPoints;

  for (int i = 0; i < 4; i++) // since rectangle has 4 edges, loop should run 4 times
  {
    sumPoints.push_back(points[i].x + points[i].y);
    subPoints.push_back(points[i].x - points[i].y);
  }

  // begin() --> iterator to the first element of the vector
  // end() --> iterator to the last element of the vector
  // min_element() -> finding the minimum element of the vector (comes with <algorithm>)
  // max_element() -> finding the maximum element of the vector (comes with <algorithm>)

  // Ordering the points
  newPoints.push_back(points[min_element(sumPoints.begin(), sumPoints.end()) - sumPoints.begin()]);
  newPoints.push_back(points[max_element(subPoints.begin(), subPoints.end()) - subPoints.begin()]);
  newPoints.push_back(points[min_element(subPoints.begin(), subPoints.end()) - subPoints.begin()]);
  newPoints.push_back(points[max_element(sumPoints.begin(), sumPoints.end()) - sumPoints.begin()]);

  return newPoints;
}

Mat getWarp(Mat img, vector<Point> points, float w, float h)
// warping the image to obtain the text with respect to the reordered points
{
  Point2f src[4] = {points[0], points[1], points[2], points[3]}; // points of the unwarped image and then we'll transform the perspective
  Point2f dst[4] = {{0.0f, 0.0f}, {w, 0.0f}, {0.0f, h}, {w, h}};
  // since width and height of the image must be specified, left top is assigned as 0,0 and the second one will be the right top that is
  // found in the width distance and can be changed so changing the x value to w is ok. For the left bottom x must be 0 and height to the
  // bottom must be specified as h and finally the last point is assigned as w and h.

  Mat matrix = getPerspectiveTransform(src, dst);     // getting the perspective from src to dst and storing it into matrix
  warpPerspective(img, imgWarp, matrix, Point(w, h)); // and warping the perspective into imgWarp global variable from matrix with point(w,h)

  return imgWarp;
}