#include "Model/TextSegmentation/Scanner.hpp"

Scanner::Scanner() {
	this->cropped = false;
};

bool compareContourAreas(std::vector<cv::Point> contour1, std::vector<cv::Point> contour2){
	double i = fabs(cv::contourArea(cv::Mat(contour1)));
	double j = fabs(cv::contourArea(cv::Mat(contour2)));
	return (i > j);
}

bool compareXCords(cv::Point p1, cv::Point p2){
	return (p1.x < p2.x);
}

bool compareYCords(cv::Point p1, cv::Point p2){
	return (p1.y < p2.y);
}

bool compareDistance(std::pair<cv::Point, cv::Point> p1, std::pair<cv::Point, cv::Point> p2){
	return (cv::norm(p1.first - p1.second) < cv::norm(p2.first - p2.second));
}

double distance(cv::Point p1, cv::Point p2){
	return std::sqrt(((p1.x - p2.x) * (p1.x - p2.x)) + ((p1.y - p2.y) * (p1.y - p2.y)));
}

void resizeToHeight(cv::Mat src, cv::Mat &dst, int height){
	cv::Size s = cv::Size(src.cols * (height / double(src.rows)), height);
	cv::resize(src, dst, s, cv::INTER_AREA);
}

void Scanner::orderPoints(std::vector<cv::Point> inpts, std::vector<cv::Point> &ordered){
	std::sort(inpts.begin(), inpts.end(), compareXCords);
	std::vector<cv::Point> lm(inpts.begin(), inpts.begin()+2);
	std::vector<cv::Point> rm(inpts.end()-2, inpts.end());

	std::sort(lm.begin(), lm.end(), compareYCords);
	cv::Point tl(lm[0]);
	cv::Point bl(lm[1]);
	std::vector<std::pair<cv::Point, cv::Point> > tmp;

	for(size_t i = 0; i< rm.size(); i++){
		tmp.push_back(std::make_pair(tl, rm[i]));
	}

	std::sort(tmp.begin(), tmp.end(), compareDistance);
	cv::Point tr(tmp[0].second);
	cv::Point br(tmp[1].second);

	ordered.push_back(tl);
	ordered.push_back(tr);
	ordered.push_back(br);
	ordered.push_back(bl);
}

void Scanner::fourPointTransform(cv::Mat src, cv::Mat &dst, std::vector<cv::Point> pts){
	std::vector<cv::Point> orderedPts;
	orderPoints(pts, orderedPts);

	double wa = distance(orderedPts[2], orderedPts[3]);
	double wb = distance(orderedPts[1], orderedPts[0]);
	double mw = std::max(wa, wb);

	double ha = distance(orderedPts[1], orderedPts[2]);
	double hb = distance(orderedPts[0], orderedPts[3]);
	double mh = std::max(ha, hb);

	cv::Point2f src_[] ={
        cv::Point2f(orderedPts[0].x, orderedPts[0].y),
        cv::Point2f(orderedPts[1].x, orderedPts[1].y),
        cv::Point2f(orderedPts[2].x, orderedPts[2].y),
		cv::Point2f(orderedPts[3].x, orderedPts[3].y),
	};

	cv::Point2f dst_[] ={
        cv::Point2f(0,0),
        cv::Point2f(mw-1, 0),
        cv::Point2f(mw-1, mh-1),
        cv::Point2f(0, mh-1)
	};

	cv::Mat m = getPerspectiveTransform(src_, dst_);
	warpPerspective(src, dst, m, cv::Size(mw, mh), cv::BORDER_REPLICATE, cv::INTER_LINEAR);
}

void Scanner::processEdge(cv::Mat input, cv::Mat &output, int openKSize, int closeKSize, bool gaussianBlur){
	cv::Mat image_pp, structuringElmt;
	cvtColor(input, image_pp, cv::COLOR_BGR2GRAY);

	if (openKSize > 0){
		structuringElmt = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(openKSize,openKSize));
		morphologyEx(image_pp, image_pp, cv::MORPH_OPEN, structuringElmt);
	}
	if (closeKSize > 0){
		structuringElmt = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(closeKSize,closeKSize));
		morphologyEx(image_pp, image_pp, cv::MORPH_CLOSE, structuringElmt);
	}

	if (gaussianBlur){
		GaussianBlur(image_pp, image_pp, cv::Size(7,7), 0);
	}
	Canny(image_pp, output, 50, 60, 3, true);
}

void Scanner::process(cv::Mat image, cv::Mat &output){
	cv::Mat orig = image.clone();

	double ratio = image.rows / 500.0;
	resizeToHeight(image, image, 500);

	cv::Mat edged, edgedCache;
	processEdge(image, edged, 11, 11, true);
	edgedCache = edged.clone();

	std::vector<std::vector<cv::Point>> contours, shapes;
	std::vector<cv::Vec4i> hierarchy;
	
	findContours(edged, contours, hierarchy, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);
	edged = cv::Mat::zeros(cv::Size(image.cols, image.rows), CV_8UC1);

	std::vector<std::vector<cv::Point>> hull(contours.size());
	int sum_area = 0, mean_area = 0;

	for(int i=0; i<contours.size(); i++){
		cv::convexHull(cv::Mat(contours[i]), hull[i], false);
		sum_area += cv::contourArea(cv::Mat(hull[i]));
	}
	mean_area = sum_area / hull.size();

	for(int i=0; i<hull.size(); i++){
		if(cv::contourArea(cv::Mat(hull[i])) >= mean_area){
			shapes.push_back(hull[i]);
		}
	}
	sort(shapes.begin(), shapes.end(), compareContourAreas);	

	for(int i=1; i<shapes.size(); i++){
		for(int j=0; j<shapes[i].size(); j++)
			shapes[0].push_back(shapes[i][j]);
	}

	cv::convexHull(cv::Mat(shapes[0]), hull[0], false);
	cv::drawContours(edged, hull, 0, 255, 2);
	cv::findContours(edged, contours, hierarchy, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);

	std::vector<std::vector<cv::Point>> approx;
	approx.resize(contours.size());

	for(int i=0; i<contours.size(); i++){
		double peri = 0.01 * arcLength(contours[i], true);
		approxPolyDP(contours[i], approx[i], peri, true);
	}

	for(int i=0; i<approx.size(); i++){
		if(approx[i].size() == 4){
			for(int j=0; j<approx[i].size(); j++)
				approx[i][j] *= ratio;

			fourPointTransform(orig, output, approx[i]);
			this->cropped = true;
			return;
		}
	}

	processEdge(image, edgedCache, 11, 9, false);

	cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(21,21));
	dilate(edgedCache, edgedCache, kernel);

    normalize(edgedCache, edgedCache, 0, 255, cv::NORM_MINMAX, CV_32F);

	int minX = edgedCache.cols, minY = edgedCache.rows;
	int maxX = 0, maxY = 1;

	for (int i=0; i<edgedCache.rows; i++){
		for (int j=0; j<edgedCache.cols; j++){
			if (edgedCache.at<float>(i,j) > 0){
				minX = j < minX ? j : minX;
				minY = i < minY ? i : minY;

				maxX = j > maxX ? j : maxX;
				maxY = i > maxY ? i : maxY;
			}
		}
	}

	if (maxX > minX || maxY > minY){
		minX *= ratio; minY *= ratio;
		maxX *= ratio; maxY *= ratio;

		int width = maxX-minX;
		int height = maxY-minY;

		orig(cv::Rect(minX, minY, width, height)).copyTo(output);
	} else {
		output = orig;
	}
}
