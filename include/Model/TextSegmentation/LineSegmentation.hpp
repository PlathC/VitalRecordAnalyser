#include <iostream>
#include <map>
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"

typedef int valleyID;

class LineSegmentation;
class Region;
class Valley;

class Line {
public:
    Line(int initialValleyID);
    friend class LineSegmentation;
    friend class Region;

private:
    Region *above;
    Region *below;
    std::vector<valleyID> valleysID;
    int minRowPosition;
    int maxRowPosition;
    std::vector<cv::Point> points;

    void generateInitialPoints(int chunksNumber, int chunkWidth, int imgWidth, std::map<int, Valley *> mapValley);
    static bool compMinRowPosition(const Line *a, const Line *b);
};

class Peak {
public:
    Peak() {}
    Peak(int p, int v): position(p), value(v){}
    Peak(int p, int v, int s, int e): position(p), value(v){}

    int position;
    int value;

    bool operator<(const Peak &p) const;
    static bool comp(const Peak &a, const Peak &b);
};

class Valley {
public:
    Valley(): valleyID(ID++), used(false){}
    Valley(int cID, int p): chunkIndex(cID), valleyID(ID++), position(p), used(false){}

    static int ID;
    int chunkIndex;
    int valleyID;
    int position;
    bool used;
    Line *line;

    static bool comp(const Valley *a, const Valley *b);
};

class Region {
public:
    Region(Line *top, Line *bottom);
    friend class LineSegmentation;

private:
    int regionID;
    cv::Mat region;
    Line *top;
    Line *bottom;
    int height;
    int rowOffset;
    cv::Mat covariance;
    cv::Vec2f mean;

    bool updateRegion(cv::Mat &img, int);
    void calculateMean();
    void calculateCovariance();
    double biVariateGaussianDensity(cv::Mat point);
};

class Chunk {
public:
    Chunk(int o, int c, int w, cv::Mat i);
    friend class LineSegmentation;

    int findPeaksValleys(std::map<int, Valley *> &mapValley);

private:
    int index;
    int startCol;
    int width;
    cv::Mat img;
    std::vector<int> histogram;
    std::vector<Peak> peaks;
    std::vector<Valley *> valleys;
    int avgHeight;
    int avgWhiteHeight;
    int linesCount;

    void calculateHistogram();
};

class LineSegmentation {
public:
    LineSegmentation();

    cv::Mat binaryImg;
    std::vector<cv::Rect> contours;
    cv::Mat contoursDrawing;
    cv::Mat linesDrawing;

    void segment(cv::Mat &input, std::vector<cv::Mat> &output, int chunksNumber, int chunksProcess);
    void getContours();
    void generateChunks();
    void getInitialLines();
    void getRegions(std::vector<cv::Mat> &output);

    void generateRegions();
    void repairLines();
    void deslant(cv::Mat image, cv::Mat &output, int bgcolor);

private:
    std::string srcBase;
    std::string extension;

    int chunksNumber;
    int chunksToProcess;

    bool notPrimesArr[100007];
    std::vector<int> primes;

    int chunkWidth;
    std::vector<Chunk *> chunks;
    std::map<int, Valley *> mapValley;
    std::vector<Line *> initialLines;
    std::vector<Region *> lineRegions;
    int avgLineHeight;
    int predictedLineHeight;

    void sieve();
    void addPrimesToVector(int, std::vector<int> &);
    void printLines(cv::Mat &inputOutput);

    Line * connectValleys(int i, Valley *currentValley, Line *line, int valleysMinAbsDist);
    bool componentBelongsToAboveRegion(Line &, cv::Rect &);
};
