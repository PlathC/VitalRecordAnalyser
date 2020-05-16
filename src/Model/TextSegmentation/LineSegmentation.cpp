#include "Model/TextSegmentation/LineSegmentation.hpp"

LineSegmentation::LineSegmentation() {
    sieve();
};

void LineSegmentation::segment(cv::Mat &input, std::vector<cv::Mat> &output, int chunksNumber, int chunksProcess) {
    this->binaryImg = input.clone();
    this->chunksNumber = chunksNumber;
    this->chunksToProcess = chunksProcess;

    getContours();
    generateChunks();
    getInitialLines();

    if (this->initialLines.size() > 0){
        generateRegions();
        repairLines();
        generateRegions();
        printLines(input);
        getRegions(output);
    } else {
        output.push_back(input);
    }

    for(int i=0; i<output.size(); i++)
        deslant(output[i], output[i], 255);
}

void LineSegmentation::sieve() {
    notPrimesArr[0] = notPrimesArr[1] = 1;
    for (int i=2; i<1e5; ++i) {
        if (notPrimesArr[i]) continue;

        primes.push_back(i);
        for (int j=i*2; j<1e5; j += i) {
            notPrimesArr[j] = 1;
        }
    }
}

void LineSegmentation::addPrimesToVector(int n, std::vector<int> &probPrimes) {
    for (int i=0; i<primes.size(); ++i) {
        while (n % primes[i]) {
            n /= primes[i];
            probPrimes[i]++;
        }
    }
}

void LineSegmentation::printLines(cv::Mat &inputOutput) {
    cvtColor(inputOutput, inputOutput, cv::COLOR_GRAY2BGR);

    for (auto line : initialLines) {
        int lastRow = -1;

        for (auto point : line->points) {
            inputOutput.at<cv::Vec3b>(point.x, point.y) = cv::Vec3b(0,0,255);

            if (lastRow != -1 && point.x != lastRow) {
                for (int i = std::min(lastRow, point.x); i < cv::max(lastRow, point.x); i++)
                    inputOutput.at<cv::Vec3b>(i, point.y) = cv::Vec3b(0,0,255);
            }
            lastRow = point.x;
        }
    }
}

void LineSegmentation::getContours() {
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(this->binaryImg, contours, hierarchy, cv::RETR_LIST, cv::CHAIN_APPROX_NONE, cv::Point(0, 0));

    std::vector<std::vector<cv::Point>> contoursPoly(contours.size());
    std::vector<cv::Rect> boundRect(contours.size()-1);

    for (size_t i=0; i<contours.size()-1; i++) {
        approxPolyDP(cv::Mat(contours[i]), contoursPoly[i], 1, true);
        boundRect[i] = cv::boundingRect(cv::Mat(contoursPoly[i]));
    }

    cv::Rect2d rectangle3;
    std::vector<cv::Rect> mergedRectangles;
    bool isRepeated;
    cvtColor(this->binaryImg, this->contoursDrawing, cv::COLOR_GRAY2BGR);

    for (int i=0; i<boundRect.size(); i++) {
        isRepeated = false;

        for (int j=i+1; j<boundRect.size(); j++) {
            rectangle3 = boundRect[i] & boundRect[j];

            if ((rectangle3.area() == boundRect[i].area()) || (rectangle3.area() == boundRect[j].area())) {
                isRepeated = true;
                rectangle3 = boundRect[i] | boundRect[j];
                cv::Rect2d mergedRectangle(rectangle3.tl().x, rectangle3.tl().y, rectangle3.width, rectangle3.height);

                if (j == boundRect.size() - 2)
                    mergedRectangles.push_back(mergedRectangle);

                boundRect[j] = mergedRectangle;
            }
        }
        if (!isRepeated)
            mergedRectangles.push_back(boundRect[i]);
    }

    for (size_t i=0; i<mergedRectangles.size(); i++)
        rectangle(this->contoursDrawing, mergedRectangles[i].tl(), mergedRectangles[i].br(), cv::Vec3b(0,0,255), 2, 8, 0);

    this->contours = mergedRectangles;
}

void LineSegmentation::generateChunks() {
    int width = binaryImg.cols;
    chunkWidth = width / chunksNumber;

    for (int i=0, startPixel=0; i<chunksNumber; ++i) {
        Chunk *c = new Chunk(
                i,
                startPixel,
                chunkWidth,
                cv::Mat(binaryImg, cv::Range(0, binaryImg.rows), cv::Range(startPixel, startPixel + chunkWidth)));

        this->chunks.push_back(c);
        startPixel += chunkWidth;
    }
}

void LineSegmentation::getInitialLines() {
    int numberOfHeights = 0, valleysMinAbsDist = 0;

    for (int i=0; i<chunksToProcess; i++) {
        int avgHeight = this->chunks[i]->findPeaksValleys(mapValley);

        if (avgHeight) numberOfHeights++;
        valleysMinAbsDist += avgHeight;
    }
    valleysMinAbsDist /= numberOfHeights;
    this->predictedLineHeight = valleysMinAbsDist;

    for (int i=chunksToProcess-1; i >= 0; i--) {
        if (chunks[i]->valleys.empty()) continue;

        for (auto &valley : chunks[i]->valleys) {
            if (valley->used) continue;
            valley->used = true;

            Line *newLine = new Line(valley->valleyID);
            newLine = connectValleys(i-1, valley, newLine, valleysMinAbsDist);
            newLine->generateInitialPoints(chunksNumber, chunkWidth, binaryImg.cols, mapValley);

            if (newLine->valleysID.size() > 1)
                this->initialLines.push_back(newLine);
        }
    }
}

Line * LineSegmentation::connectValleys(int i, Valley *currentValley, Line *line, int valleysMinAbsDist) {
    if (i <= 0 || chunks[i]->valleys.empty()) return line;

    int connectedTo = -1;
    int minDistance = 100000;

    for (int j=0; j<this->chunks[i]->valleys.size(); j++) {
        Valley *valley = this->chunks[i]->valleys[j];
        if (valley->used) continue;

        int dist = currentValley->position - valley->position;
        dist = dist < 0 ? -dist : dist;
        if (minDistance > dist && dist <= valleysMinAbsDist) {
            minDistance = dist, connectedTo = j;
        }
    }

    if (connectedTo == -1) {
        return line;
    }

    line->valleysID.push_back(this->chunks[i]->valleys[connectedTo]->valleyID);
    Valley *v = this->chunks[i]->valleys[connectedTo];
    v->used = true;

    return connectValleys(i-1, v, line, valleysMinAbsDist);
}

void LineSegmentation::generateRegions() {
    sort(this->initialLines.begin(), this->initialLines.end(), Line::compMinRowPosition);
    this->lineRegions = std::vector<Region *>();

    Region *r = new Region(nullptr, this->initialLines[0]);
    r->updateRegion(this->binaryImg, 0);

    this->initialLines[0]->above = r;
    this->lineRegions.push_back(r);

    if (r->height < this->predictedLineHeight * 2.5)
        this->avgLineHeight += r->height;

    for (int i=0; i<this->initialLines.size(); ++i) {
        Line *topLine = this->initialLines[i];
        Line *bottomLine = (i == this->initialLines.size()-1) ? nullptr : this->initialLines[i + 1];

        Region *r = new Region(topLine, bottomLine);
        bool res = r->updateRegion(this->binaryImg, i);

        if (topLine != nullptr)
            topLine->below = r;

        if (bottomLine != nullptr)
            bottomLine->above = r;

        if (!res) {
            this->lineRegions.push_back(r);
            if (r->height < this->predictedLineHeight * 2.5)
                this->avgLineHeight += r->height;
        }
    }

    if (this->lineRegions.size() > 0)
        this->avgLineHeight /= this->lineRegions.size();
}

void LineSegmentation::repairLines() {

    for (Line *line : initialLines) {
        std::map<int, bool> columnProcessed = std::map<int, bool>();

        for (int i=0; i<line->points.size(); i++) {
            cv::Point &point = line->points[i];
            int x = (line->points[i]).x, y = (line->points[i]).y;

            if (this->binaryImg.at<uchar>(point.x, point.y) == 255) {
                if (i == 0) continue;
                bool blackFound = false;

                if (line->points[i - 1].x != line->points[i].x) {
                    int minRow = std::min(line->points[i - 1].x, line->points[i].x);
                    int maxRow = std::max(line->points[i - 1].x, line->points[i].x);

                    for (int j = minRow; j <= maxRow && !blackFound; ++j) {
                        if (this->binaryImg.at<uchar>(j, line->points[i - 1].y) == 0) {
                            x = j, y = line->points[i - 1].y;
                            blackFound = true;
                        }
                    }
                }
                if (!blackFound) continue;
            }
            if (columnProcessed[y]) continue;
            columnProcessed[y] = true;

            for (auto contour : this->contours) {
                if (y >= contour.tl().x && y <= contour.br().x && x >= contour.tl().y && x <= contour.br().y) {
                    if (contour.br().y - contour.tl().y > this->avgLineHeight * 0.9) continue;

                    bool isComponentAbove = componentBelongsToAboveRegion(*line, contour);

                    int newRow;
                    if (!isComponentAbove) {
                        newRow = contour.tl().y;
                        line->minRowPosition = std::min(line->minRowPosition, newRow);
                    } else {
                        newRow = contour.br().y;
                        line->maxRowPosition = std::max(newRow, line->maxRowPosition);
                    }
                    for (int k = contour.tl().x; k < contour.tl().x + contour.width; k++) {
                        line->points[k].x = newRow;
                    }
                    i = (contour.br().x);
                    break;
                }
            }
        }
    }
}

bool LineSegmentation::componentBelongsToAboveRegion(Line &line, cv::Rect &contour) {
    std::vector<int> probAbovePrimes(primes.size(), 0);
    std::vector<int> probBelowPrimes(primes.size(), 0);
    int n = 0;

    for (int i=contour.tl().x; i<contour.tl().x + contour.width; i++) {
        for (int j=contour.tl().y; j<contour.tl().y + contour.height; j++) {
            if (binaryImg.at<uchar>(j, i) == 255) continue;
            n++;

            cv::Mat contourPoint = cv::Mat::zeros(1, 2, CV_32F);
            contourPoint.at<float>(0, 0) = j;
            contourPoint.at<float>(0, 1) = i;

            int newProbAbove = (int) ((line.above != nullptr) ? (line.above->biVariateGaussianDensity(
                    contourPoint.clone())) : 0);
            int newProbBelow = (int) ((line.below != nullptr) ? (line.below->biVariateGaussianDensity(
                    contourPoint.clone())) : 0);

            addPrimesToVector(newProbAbove, probAbovePrimes);
            addPrimesToVector(newProbBelow, probBelowPrimes);
        }
    }

    int probAbove = 0, probBelow = 0;

    for (int k = 0; k < probAbovePrimes.size(); ++k) {
        int mini = std::min(probAbovePrimes[k], probBelowPrimes[k]);

        probAbovePrimes[k] -= mini;
        probBelowPrimes[k] -= mini;

        probAbove += probAbovePrimes[k] * primes[k];
        probBelow += probBelowPrimes[k] * primes[k];
    }

    return probAbove < probBelow;
}

void LineSegmentation::getRegions(std::vector<cv::Mat> &output) {
    std::vector<cv::Mat> ret;
    for (auto region : this->lineRegions) {
        ret.push_back(region->region.clone());
    }
    output = ret;
}

struct Result {
    float sumAlpha = 0.0f;
    cv::Mat transform;
    cv::Size size;
    bool operator < (const Result& rhs) const { return sumAlpha < rhs.sumAlpha; }
};

void LineSegmentation::deslant(cv::Mat image, cv::Mat &output, int bgcolor){
    std::vector<float> alphaVals = {-1.0, -0.75, -0.5, -0.25, 0.0, 0.25, 0.5, 0.75, 1.0};
    std::vector<int> sumAlpha(alphaVals.size(), 0);
    std::vector<Result> results;

    for (size_t i=0; i<alphaVals.size(); ++i){
        Result result;
        const float alpha = alphaVals[i];
        const float shiftX = std::max(-alpha*image.rows, 0.0f);
        result.size = cv::Size(image.cols + static_cast<int>(ceil(abs(alpha*image.rows))), image.rows);

        result.transform = cv::Mat(2,3, CV_32F);
        result.transform.at<float>(0,0) = 1;
        result.transform.at<float>(0,1) = alpha;
        result.transform.at<float>(0,2) = shiftX;
        result.transform.at<float>(1,0) = 0;
        result.transform.at<float>(1,1) = 1;
        result.transform.at<float>(1,2) = 0;

        cv::Mat imgSheared;
        warpAffine(image, imgSheared, result.transform, result.size, cv::INTER_NEAREST);

        for (int x=0; x<imgSheared.cols; ++x){
            std::vector<int> fgIndices;

            for (int y=0; y<imgSheared.rows; ++y){
                if (imgSheared.at<unsigned char>(y,x))
                    fgIndices.push_back(y);
            }
            if (fgIndices.empty()) continue;

            int hAlpha = static_cast<int>(fgIndices.size());
            int deltaYAlpha = fgIndices.back() - fgIndices.front() + 1;

            if (hAlpha == deltaYAlpha)
                result.sumAlpha += hAlpha*hAlpha;
            results.push_back(result);
        }

        Result bestResult = *max_element(results.begin(), results.end());
        warpAffine(image, output, bestResult.transform, bestResult.size, cv::INTER_LINEAR, cv::BORDER_CONSTANT, cv::Scalar(bgcolor));
    }
}

Chunk::Chunk(int i, int c, int w, cv::Mat m) : valleys(std::vector<Valley *>()), peaks(std::vector<Peak>()) {
    this->index = i;
    this->startCol = c;
    this->width = w;
    this->img = m.clone();
    this->histogram.resize((unsigned long) this->img.rows);
    this->avgHeight = 0;
    this->avgWhiteHeight = 0;
    this->linesCount = 0;
}

void Chunk::calculateHistogram() {
    int blackCount = 0, currentHeight = 0, currentWhiteCount = 0, whiteLinesCount = 0;
    std::vector<int> whiteSpaces;

    for (int i=0; i<this->img.rows; ++i) {
        blackCount = 0;
        for (int j=0; j<this->img.cols; ++j) {
            if (this->img.at<uchar>(i, j) == 0) {
                blackCount++;
                this->histogram[i]++;
            }
        }
        if (blackCount) {
            currentHeight++;
            if (currentWhiteCount) {
                whiteSpaces.push_back(currentWhiteCount);
            }
            currentWhiteCount = 0;
        } else {
            currentWhiteCount++;
            if (currentHeight) {
                linesCount++;
                avgHeight += currentHeight;
            }
            currentHeight = 0;
        }
    }

    sort(whiteSpaces.begin(), whiteSpaces.end());
    for (int i=0; i<whiteSpaces.size(); ++i) {
        if (whiteSpaces[i] > 4 * avgHeight) break;
        avgWhiteHeight += whiteSpaces[i];
        whiteLinesCount++;
    }
    if (whiteLinesCount) avgWhiteHeight /= whiteLinesCount;

    if (linesCount) avgHeight /= linesCount;
    avgHeight = std::max(30, int(avgHeight + (avgHeight / 2.0)));
}

int Chunk::findPeaksValleys(std::map<int, Valley *> &mapValley) {
    this->calculateHistogram();

    for (int i=1; i+1<this->histogram.size(); i++) {
        int leftVal = this->histogram[i-1], centreVal = this->histogram[i], rightVal = this->histogram[i+1];

        if (centreVal >= leftVal && centreVal >= rightVal) {
            if (!peaks.empty() && i - peaks.back().position <= avgHeight / 2 &&
                centreVal >= peaks.back().value) {
                peaks.back().position = i;
                peaks.back().value = centreVal;
            } else if (peaks.size() > 0 && i - peaks.back().position <= avgHeight / 2 &&
                       centreVal < peaks.back().value) {}
            else {
                peaks.push_back(Peak(i, centreVal));
            }
        }
    }

    int peaksAverageValues = 0;
    std::vector<Peak> newPeaks;
    for (auto peak : peaks) {
        peaksAverageValues += peak.value;
    }
    peaksAverageValues /= std::max(1, int(peaks.size()));

    for (auto peak : peaks) {
        if (peak.value >= peaksAverageValues / 4) {
            newPeaks.push_back(peak);
        }
    }
    linesCount = int(newPeaks.size());
    peaks = newPeaks;

    sort(peaks.begin(), peaks.end());
    peaks.resize(linesCount + 1 <= peaks.size() ? (unsigned long) linesCount + 1 : peaks.size());
    sort(peaks.begin(), peaks.end(), Peak::comp);

    for (int i=1; i<peaks.size(); i++) {
        int minPosition = (peaks[i - 1].position + peaks[i].position) / 2;
        int minValue = this->histogram[minPosition];

        for (int j=(peaks[i-1].position + avgHeight / 2);
             j < (i == peaks.size() ? this->img.rows : peaks[i].position - avgHeight - 30); j++) {

            int valleyBlackCount = 0;
            for (int l=0; l<this->img.cols; ++l) {
                if (this->img.at<uchar>(j, l) == 0)
                    valleyBlackCount++;
            }
            if (i == peaks.size() && valleyBlackCount <= minValue) {
                minValue = valleyBlackCount;
                minPosition = j;
                if (!minValue) {
                    minPosition = std::min(this->img.rows - 10, minPosition + avgHeight);
                    j = this->img.rows;
                }
            } else if (minValue != 0 && valleyBlackCount <= minValue) {
                minValue = valleyBlackCount;
                minPosition = j;
            }
        }

        auto *newValley = new Valley(this->index, minPosition);
        valleys.push_back(newValley);
        mapValley[newValley->valleyID] = newValley;
    }
    return int(ceil(avgHeight));
}

Line::Line(int initialValleyID): minRowPosition(0), maxRowPosition(0), points(std::vector<cv::Point>()) {
    valleysID.push_back(initialValleyID);
}

void Line::generateInitialPoints(int chunksNumber, int chunkWidth, int imgWidth, std::map<int, Valley *> mapValley) {
    int c = 0, previousRow = 0;
    sort(valleysID.begin(), valleysID.end());

    if (mapValley[valleysID.front()]->chunkIndex > 0) {
        previousRow = mapValley[valleysID.front()]->position;
        maxRowPosition = minRowPosition = previousRow;

        for (int j = 0; j < mapValley[valleysID.front()]->chunkIndex * chunkWidth; j++) {
            if (c++ == j)
                points.push_back(cv::Point(previousRow, j));
        }
    }

    for (auto id : valleysID) {
        int chunkIndex = mapValley[id]->chunkIndex;
        int chunkRow = mapValley[id]->position;
        int chunkStartColumn = chunkIndex * chunkWidth;

        for (int j=chunkStartColumn; j<chunkStartColumn + chunkWidth; j++) {
            minRowPosition = std::min(minRowPosition, chunkRow);
            maxRowPosition = std::max(maxRowPosition, chunkRow);
            if (c++ == j)
                points.push_back(cv::Point(chunkRow, j));
        }
        if (previousRow != chunkRow) {
            previousRow = chunkRow;
            minRowPosition = std::min(minRowPosition, chunkRow);
            maxRowPosition = std::max(maxRowPosition, chunkRow);
        }
    }

    if (chunksNumber-1 > mapValley[valleysID.back()]->chunkIndex) {
        int chunkIndex = mapValley[valleysID.back()]->chunkIndex,
                chunkRow = mapValley[valleysID.back()]->position;
        for (int j = chunkIndex * chunkWidth + chunkWidth; j < imgWidth; j++) {
            if (c++ == j)
                points.push_back(cv::Point(chunkRow, j));
        }
    }
}

bool Line::compMinRowPosition(const Line *a, const Line *b) {
    return a->minRowPosition < b->minRowPosition;
}

Region::Region(Line *top, Line *bottom) {
    this->top = top;
    this->bottom = bottom;
    this->height = 0;
}

bool Region::updateRegion(cv::Mat &binaryImg, int regionID) {
    this->regionID = regionID;

    int minRegionRow = rowOffset = (top == nullptr) ? 0 : top->minRowPosition;
    int maxRegionRow = (bottom == nullptr) ? binaryImg.rows : bottom->maxRowPosition;

    int start = std::min(minRegionRow, maxRegionRow), end = std::max(minRegionRow, maxRegionRow);
    region = cv::Mat::ones(end - start, binaryImg.cols, CV_8U) * 255;

    for (int c=0; c<binaryImg.cols; c++) {
        int start = ((top == nullptr) ? 0 : top->points[c].x);
        int end = ((bottom == nullptr) ? binaryImg.rows - 1 : bottom->points[c].x);

        if (end > start)
            this->height = std::max(this->height, end - start);

        for (int i=start; i<end; i++)
            region.at<uchar>(i - minRegionRow, c) = binaryImg.at<uchar>(i, c);
    }
    calculateMean();
    calculateCovariance();

    return countNonZero(region) == region.cols * region.rows;
}

void Region::calculateMean() {
    mean[0] = mean[1] = 0.0f;
    int n = 0;

    for (int i = 0; i < region.rows; i++) {
        for (int j = 0; j < region.cols; j++) {
            if (region.at<uchar>(i, j) == 255) continue;

            if (n == 0) {
                n = n + 1;
                mean = cv::Vec2f(i + rowOffset, j);
            } else {
                mean = (n - 1.0) / n * mean + 1.0 / n * cv::Vec2f(i + rowOffset, j);
                n = n + 1;
            }
        }
    }
}

void Region::calculateCovariance() {
    cv::Mat covariance = cv::Mat::zeros(2, 2, CV_32F);

    int n = 0;
    float sumISquared = 0, sumJSquared = 0, sumIJ = 0;

    for (int i = 0; i < region.rows; i++) {
        for (int j = 0; j < region.cols; j++) {
            if ((int) region.at<uchar>(i, j) == 255) continue;

            float newI = i + rowOffset - mean[0];
            float newJ = j - mean[1];

            sumISquared += newI * newI;
            sumIJ += newI * newJ;
            sumJSquared += newJ * newJ;
            n++;
        }
    }

    if (n) {
        covariance.at<float>(0, 0) = sumISquared / n;
        covariance.at<float>(0, 1) = sumIJ / n;
        covariance.at<float>(1, 0) = sumIJ / n;
        covariance.at<float>(1, 1) = sumJSquared / n;
    }
    this->covariance = covariance.clone();
}

double Region::biVariateGaussianDensity(cv::Mat point) {
    point.at<float>(0, 0) -= this->mean[0];
    point.at<float>(0, 1) -= this->mean[1];

    cv::Mat pointTranspose;
    transpose(point, pointTranspose);

    cv::Mat ret = ((point * this->covariance.inv() * pointTranspose));
    ret *= sqrt(determinant(this->covariance * 2 * CV_PI));

    return ret.at<float>(0, 0);
}

bool Peak::operator<(const Peak &p) const {
    return value > p.value;
}

bool Peak::comp(const Peak &a, const Peak &b) {
    return a.position < b.position;
}

int Valley::ID = 0;

bool Valley::comp(const Valley *a, const Valley *b) {
    return a->position < b->position;
}
