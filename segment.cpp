#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
//#include "opencv2/nonfree/nonfree.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <algorithm>



using namespace std;
using namespace cv;

double K = 150;
struct edge {
  int v1, v2;
  double w;
};
struct elem {
  int par, rank;
};
bool edgecmp (edge lhs, edge rhs) { return lhs.w < rhs.w; }

int set_find(vector<elem>* S, int x) {
  if (S->at(x).par != x)
    S->at(x).par = set_find(S, S->at(x).par);
  return S->at(x).par;
}
int set_union(vector<elem>* S, int x, int y) {
  int xroot = set_find(S, x);
  int yroot = set_find(S, y);

  if (xroot == yroot)
    return xroot;

  int xrank = S->at(xroot).rank;
  int yrank = S->at(yroot).rank;

  if (xrank < yrank) {
    S->at(xroot).par = yroot;
    return yroot;
  }
  else if (xrank > yrank) {
    S->at(yroot).par = xroot;
    return xroot;
  }
  else {
    S->at(yroot).par = xroot;
    S->at(xroot).rank += 1;
    return xroot;
  }
  
}
int main(int argc, char* argv[]) {
  Mat img = imread("baseball.jpeg", CV_LOAD_IMAGE_GRAYSCALE);
  GaussianBlur(img, img, Size(0, 0), 0.8);
  int rows = img.rows;
  int cols = img.cols;


  int max_inten = 0;
  for (int r = 0; r < rows; r++) {
    for (int c = 0; c < cols; c++) {
      if (max_inten < img.at<char>(r, c)) {
        max_inten = (int)img.at<char>(r, c);
      }
    }
  }
  
  vector<edge> E;
  E.reserve(rows*cols*8/2);


  for (int r = 0; r < rows; r++) {
    for (int c = 0; c < cols; c++) {
      for (int dr = max(0, r-1); dr < min(rows, r + 1); dr++) {
        for (int dc = max(0, c-1); dc < min(cols, c + 1); dc++) {
          if (r != dr || c != dc) {
            edge e;
            e.v1 = r*cols + c;
            e.v2 = dr*cols + dc;
            e.w = ((double)abs(img.at<char>(r, c) - img.at<char>(dr, dc)))/max_inten;
            E.push_back(e);
          }
        }
      }
    }
  }

  sort(E.begin(), E.end(), edgecmp);
  int m = E.size();


  vector<elem> S(rows*cols);
  for (int i = 0; i < S.size(); i++) {
    elem tmp = { i, 0 };
    S[i] = tmp;
  }

  cout << "Calculating segmentation with " << m << " edges" << endl;
  vector<double>Int(rows*cols, 0);
  vector<int>size(rows*cols, 1);
  for (int q = 0; q < m; q++) {
    edge e = E[q];
    int c1 = set_find(&S, e.v1);
    int c2 = set_find(&S, e.v2);
    if (c1 != c2) {
      double MInt = min(Int[c1] + K/size[c1], Int[c2] + K/size[c2]);
      if (e.w <= MInt) {
        int c3 = set_union(&S, c1, c2);
        Int[c3] = e.w;
        size[c3] = size[c1] + size[c2];
      }
    }
  }


  vector<int>component2id(rows*cols, 0);
  int t = 1;
  for (int r = 0; r < rows; r++) {
    for (int c = 0; c < cols; c++) {
      int v = r*cols + c;
      int comp = set_find(&S, v);
      if (component2id[comp] == 0) {
        component2id[comp] = t;
        t++;
      }
    }
  }
  cout << "Segmented into " << t-1 << " components" << endl;

  Mat segmentation(rows, cols, CV_32FC3, Scalar(0, 0, 0));
  for (int r = 0; r < rows; r++) {
    for (int c = 0; c < cols; c++) {
      int v = r*cols + c;
      int comp = set_find(&S, v);
      int id = component2id[comp];
      segmentation.at<Vec3f>(r, c) = Vec3f((double)id/(t-1)*359,
                                           0.8, 1);
    }
  }
  cvtColor(segmentation, segmentation, CV_HSV2BGR);
  namedWindow("Image", CV_WINDOW_NORMAL);
  imshow("Image", img);
  namedWindow("Segmentation", CV_WINDOW_NORMAL);
  imshow("Segmentation", segmentation);

  imwrite("segmented.jpg", segmentation);
  waitKey(0);
}


