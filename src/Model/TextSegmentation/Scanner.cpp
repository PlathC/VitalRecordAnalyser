#include <utility>

#include "Model/TextSegmentation/Scanner.hpp"

Scanner::Scanner()
{
	this->cropped = false;
};

bool compareContourAreas(const std::vector<cv::Point>& contour1, const std::vector<cv::Point>& contour2)
{
	double i = fabs(cv::contourArea(cv::Mat(contour1)));
	double j = fabs(cv::contourArea(cv::Mat(contour2)));
	return (i > j);
}

bool compareXCords(const cv::Point& p1, const cv::Point& p2)
{
	return (p1.x < p2.x);
}

bool compareYCords(const cv::Point& p1, const cv::Point& p2)
{
	return (p1.y < p2.y);
}

bool compareDistance(const std::pair<cv::Point, cv::Point>& p1, const std::pair<cv::Point, cv::Point>& p2)
{
	return (cv::norm(p1.first - p1.second) < cv::norm(p2.first - p2.second));
}

double distance(const cv::Point& p1, const cv::Point& p2)
{
	return std::sqrt(static_cast<double>(((p1.x - p2.x) * (p1.x - p2.x)) + ((p1.y - p2.y) * (p1.y - p2.y))));
}

void resizeToHeight(const cv::Mat& src, cv::Mat &dst, int height)
{
	cv::Size s = cv::Size(static_cast<int>(src.cols * (height / static_cast<double>(src.rows))), height);
	cv::resize(src, dst, s, cv::INTER_AREA);
}

void Scanner::orderPoints(const std::vector<cv::Point>& inpts, std::vector<cv::Point> &ordered)
{
    std::vector<cv::Point> ninpts = std::vector<cv::Point>(inpts);
	std::sort(ninpts.begin(), ninpts.end(), compareXCords);
	std::vector<cv::Point> lm(ninpts.begin(), ninpts.begin()+2);
	std::vector<cv::Point> rm(ninpts.end()-2, ninpts.end());

	std::sort(lm.begin(), lm.end(), compareYCords);
	cv::Point tl(lm[0]);
	cv::Point bl(lm[1]);
	std::vector<std::pair<cv::Point, cv::Point> > tmp;

	for(const auto & i : rm)
	{
		tmp.emplace_back(tl, i);
	}

	std::sort(tmp.begin(), tmp.end(), compareDistance);
	cv::Point tr(tmp[0].second);
	cv::Point br(tmp[1].second);

	ordered.push_back(tl);
	ordered.push_back(tr);
	ordered.push_back(br);
	ordered.push_back(bl);
}

void Scanner::fourPointTransform(const cv::Mat& src, cv::Mat& dst, const std::vector<cv::Point>& pts){
	std::vector<cv::Point> orderedPts;
	orderPoints(pts, orderedPts);

	double wa = distance(orderedPts[2], orderedPts[3]);
	double wb = distance(orderedPts[1], orderedPts[0]);
	double mw = std::max(wa, wb);

	double ha = distance(orderedPts[1], orderedPts[2]);
	double hb = distance(orderedPts[0], orderedPts[3]);
	double mh = std::max(ha, hb);

	cv::Point2f src_[] ={
        cv::Point2f(static_cast<float>(orderedPts[0].x), static_cast<float>(orderedPts[0].y)),
        cv::Point2f(static_cast<float>(orderedPts[1].x), static_cast<float>(orderedPts[1].y)),
        cv::Point2f(static_cast<float>(orderedPts[2].x), static_cast<float>(orderedPts[2].y)),
		cv::Point2f(static_cast<float>(orderedPts[3].x), static_cast<float>(orderedPts[3].y)),
	};

	cv::Point2f dst_[] ={
        cv::Point2f(0,0),
        cv::Point2f(static_cast<float>(mw - 1), 0),
        cv::Point2f(static_cast<float>(mw - 1), static_cast<float>(mh - 1)),
        cv::Point2f(0, static_cast<float>(mh - 1))
	};

	cv::Mat m = cv::getPerspectiveTransform(src_, dst_);
	cv::warpPerspective(src, dst, m, cv::Size(static_cast<int>(mw), static_cast<int>(mh)), cv::BORDER_REPLICATE, cv::INTER_LINEAR);
}

void Scanner::processEdge(const cv::Mat& input, cv::Mat& output, int openKSize, int closeKSize, bool gaussianBlur)
{
	cv::Mat image_pp, structuringElmt;
	cv::cvtColor(input, image_pp, cv::COLOR_BGR2GRAY);

	if (openKSize > 0)
	{
		structuringElmt = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(openKSize,openKSize));
		cv::morphologyEx(image_pp, image_pp, cv::MORPH_OPEN, structuringElmt);
	}
	if (closeKSize > 0)
	{
		structuringElmt = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(closeKSize,closeKSize));
		cv::morphologyEx(image_pp, image_pp, cv::MORPH_CLOSE, structuringElmt);
	}

	if (gaussianBlur)
	{
		cv::GaussianBlur(image_pp, image_pp, cv::Size(7,7), 0);
	}
	cv::Canny(image_pp, output, 50, 60, 3, true);
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

	for(int i=0; i<contours.size(); i++)
	{
		cv::convexHull(cv::Mat(contours[i]), hull[i], false);
		sum_area += static_cast<int>(cv::contourArea(cv::Mat(hull[i])));
	}
	mean_area = sum_area / static_cast<int>(hull.size());

	for(const auto& i : hull)
	{
		if(cv::contourArea(cv::Mat(i)) >= mean_area)
		{
			shapes.push_back(i);
		}
	}
	std::sort(shapes.begin(), shapes.end(), compareContourAreas);

	for(int i=1; i<shapes.size(); i++)
	{
		for(int j=0; j<shapes[i].size(); j++)
			shapes[0].push_back(shapes[i][j]);
	}

	cv::convexHull(cv::Mat(shapes[0]), hull[0], false);
	cv::drawContours(edged, hull, 0, 255, 2);
	cv::findContours(edged, contours, hierarchy, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);

	std::vector<std::vector<cv::Point>> approx;
	approx.resize(contours.size());

	for(int i=0; i<contours.size(); i++)
	{
		double peri = 0.01 * arcLength(contours[i], true);
		approxPolyDP(contours[i], approx[i], peri, true);
	}

	for(auto& i : approx)
	{
		if(i.size() == 4)
		{
			for(auto& j : i)
				j *= ratio;

			fourPointTransform(orig, output, i);
			this->cropped = true;
			return;
		}
	}

	processEdge(image, edgedCache, 11, 9, false);

	cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(21,21));
	cv::dilate(edgedCache, edgedCache, kernel);

    cv::normalize(edgedCache, edgedCache, 0, 255, cv::NORM_MINMAX, CV_32F);

	int minX = edgedCache.cols, minY = edgedCache.rows;
	int maxX = 0, maxY = 1;

	for (int i=0; i<edgedCache.rows; i++)
	{
		for (int j=0; j<edgedCache.cols; j++)
		{
			if (edgedCache.at<float>(i,j) > 0){
				minX = j < minX ? j : minX;
				minY = i < minY ? i : minY;

				maxX = j > maxX ? j : maxX;
				maxY = i > maxY ? i : maxY;
			}
		}
	}

	if (maxX > minX || maxY > minY)
	{
		minX = static_cast<int>(minX * ratio);
		minY = static_cast<int>(minY * ratio);
		maxX = static_cast<int>(maxX * ratio);
		maxY = static_cast<int>(maxY * ratio);

		int width = maxX - minX;
		int height = maxY - minY;

		orig(cv::Rect(minX, minY, width, height)).copyTo(output);
	}
	else
    {
		output = orig;
	}
}
