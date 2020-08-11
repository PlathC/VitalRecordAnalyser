#include "Model/TextSegmentation/Binarization.hpp"

#define uget(x,y)at<unsigned char>(y,x)
#define uset(x,y,v)at<unsigned char>(y,x)=v;
#define fget(x,y)at<float>(y,x)
#define fset(x,y,v)at<float>(y,x)=v;

Binarization::Binarization() {};

void Binarization::binarize(const cv::Mat& image, cv::Mat &output, bool light, int option)
{
    cv::Mat grayscale;
    cv::cvtColor(image, grayscale, cv::COLOR_BGR2GRAY);

    if (light){
        lightDistribution(grayscale);
    }

    int winy = static_cast<int>((2.0 * grayscale.rows-1) / 3);
    int winx = static_cast<int>(grayscale.cols-1 < winy ? grayscale.cols-1 : winy);
    if (winx > 127) winx = winy = 127;

    thresholdImg(grayscale, output, option, winx, winy, 0.1, 128);
}

void Binarization::thresholdImg(cv::Mat im, cv::Mat &output, int option, int winx, int winy, double k, double dR)
{
    if (option > 1)
    {
        float th = 0;
        int wxh	= winx/2;
        int wyh	= winy/2;

        output = im.clone();
        cv::Mat mapM = cv::Mat::zeros(im.rows, im.cols, CV_32F);
        cv::Mat mapS = cv::Mat::zeros(im.rows, im.cols, CV_32F);

        double minI, maxI;
        float maxS = static_cast<float>(calcLocalStats(im, mapM, mapS, winx, winy));

        minMaxLoc(im, &minI, &maxI);
        cv::Mat thsurf(im.rows, im.cols, CV_32F);

        float m, s;

        int xFirstth= wxh;
        int xLastth = im.cols-wxh-1;
        int yLastth = im.rows-wyh-1;
        int yFirstth = wyh;
        for	(int j = yFirstth; j <= yLastth; j++)
        {
            float *thSurfData = thsurf.ptr<float>(j) + wxh;
            float *mapMData = mapM.ptr<float>(j) + wxh;
            float *mapSData = mapS.ptr<float>(j) + wxh;

            for	(int i=0; i<=im.cols-winx; i++)
            {
                m = *mapMData++;
                s = *mapSData++;

                switch (option)
                {
                    case 2: // NIBLACK
                        th = static_cast<float>(m + k * s);
                        break;

                    case 3: // SAUVOLA
                        th = static_cast<float>(m * (1 + k*(s/dR-1)));
                        break;

                    case 4: // WOLF
                        th = static_cast<float>(m + k * (s/maxS-1) * (m-minI));
                        break;
                }
                *thSurfData++ = th;

                if (i==0)
                {
                    auto* thSurfPtr = thsurf.ptr<float>(j);
                    for (int v=0; v<=xFirstth; ++v)
                        *thSurfPtr++ = th;

                    if (j==yFirstth)
                    {
                        for (int u=0; u<yFirstth; ++u)
                        {
                            thSurfPtr = thsurf.ptr<float>(u);
                            for (int v=0; v <= xFirstth; ++v)
                                *thSurfPtr++ = th;
                        }
                    }

                    if (j == yLastth)
                    {
                        for (int u=yLastth+1; u<im.rows; ++u)
                        {
                            thSurfPtr = thsurf.ptr<float>(u);

                            for (int v=0; v<=xFirstth; ++v)
                                *thSurfPtr++ = th;
                        }
                    }
                }

                if (j == yFirstth)
                {
                    for (int u = 0; u < yFirstth; ++u)
                        thsurf.fset(i + wxh, u, th);
                }

                if (j == yLastth)
                {
                    for (int u = yLastth + 1; u < im.rows; ++u)
                        thsurf.fset(i + wxh, u, th);
                }
            }
            float *thSurfPtr = thsurf.ptr<float>(j) + xLastth;

            for (int i=xLastth; i<im.cols; ++i)
                *thSurfPtr++ = th;

            if (j == yFirstth)
            {
                for (int u=0; u<yFirstth; ++u)
                {
                    float *thSurfPtr = thsurf.ptr<float>(u) + xLastth;

                    for (int i = xLastth; i < im.cols; ++i)
                        *thSurfPtr++ = th;
                }
            }

            if (j == yLastth)
            {
                for (int u=yLastth+1; u<im.rows; ++u)
                {
                    float *thSurfPtr = thsurf.ptr<float>(u) + xLastth;

                    for (int i=xLastth; i<im.cols; ++i)
                        *thSurfPtr++ = th;
                }
            }
        }

        for	(int y = 0; y < im.rows; ++y)
        {
            auto* imData     = im.ptr<unsigned char>(y);
            auto* thSurfData = thsurf.ptr<float>(y);
            auto* outputData = output.ptr<unsigned char>(y);

            for	(int x=0; x<im.cols; ++x)
            {
                *outputData = static_cast<float>(*imData) >= *thSurfData ? 255 : 0;
                imData++;
                thSurfData++;
                outputData++;
            }
        }

    }
    else if (option == 1)
    {
        cv::Mat smoothedImg;
        blur(im, smoothedImg, cv::Size(3,3), cv::Point(-1,-1));
        threshold(smoothedImg, output, 0.0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);

    }
    else
    {
        threshold(im, output, 127, 255, cv::THRESH_BINARY);
    }
}

double Binarization::calcLocalStats(cv::Mat &im, cv::Mat &mapM, cv::Mat &mapS, int winx, int winy)
{
    cv::Mat imSum, imSumSq;
    integral(im, imSum, imSumSq, CV_64F);

    float m, s, maxS, sum, sumSq;
    int wxh	= winx/2;
    int wyh	= winy/2;
    int xFirstth = wxh;
    int yFirstth = wyh;
    int yLastth  = im.rows-wyh-1;
    double winarea = winx*winy;

    maxS = 0;
    for	(int j = yFirstth ; j<=yLastth; j++)
    {
        sum = sumSq = 0;

        double *sumTopLeft = imSum.ptr<double>(j - wyh);
        double *sumTopRight = sumTopLeft + winx;
        double *sumBottomLeft = imSum.ptr<double>(j - wyh + winy);
        double *sumBottomRight = sumBottomLeft + winx;

        double *sumEqTopLeft = imSumSq.ptr<double>(j - wyh);
        double *sumEqTopRight = sumEqTopLeft + winx;
        double *sumEqBottomLeft = imSumSq.ptr<double>(j - wyh + winy);
        double *sumEqBottomRight = sumEqBottomLeft + winx;

        sum = static_cast<float>((*sumBottomRight + *sumTopLeft) - (*sumTopRight + *sumBottomLeft));
        sumSq = static_cast<float>((*sumEqBottomRight + *sumEqTopLeft) - (*sumEqTopRight + *sumEqBottomLeft));

        m = static_cast<float>(sum / winarea);
        s = static_cast<float>(std::sqrt ((sumSq - m * sum) / winarea));
        if (s > maxS) maxS = s;

        float *mapMData = mapM.ptr<float>(j) + xFirstth;
        float *mapSData = mapS.ptr<float>(j) + xFirstth;
        *mapMData++ = m;
        *mapSData++ = s;

        for	(int i=1 ; i<=im.cols-winx; i++)
        {
            sumTopLeft++, sumTopRight++, sumBottomLeft++, sumBottomRight++;

            sumEqTopLeft++, sumEqTopRight++, sumEqBottomLeft++, sumEqBottomRight++;

            sum = static_cast<float>((*sumBottomRight + *sumTopLeft) - (*sumTopRight + *sumBottomLeft));
            sumSq = static_cast<float>((*sumEqBottomRight + *sumEqTopLeft) - (*sumEqTopRight + *sumEqBottomLeft));

            m = static_cast<float>(sum / winarea);
            s = static_cast<float>(std::sqrt ((sumSq - m * sum) / winarea));
            if (s > maxS) maxS = s;

            *mapMData++ = m;
            *mapSData++ = s;
        }
    }
    return maxS;
}

void Binarization::lightDistribution(cv::Mat &grayscale){
    getHistogram(grayscale);
    getCEI(grayscale);
    getEdge(grayscale);
    getTLI(grayscale);

    cv::Mat intImg = this->cei.clone();

    for (int y=0; y<intImg.cols; y++)
    {
        for (int x=0; x<intImg.rows; x++)
        {
            if (this->tliErosion.at<float>(x,y) == 0)
            {
                int head = x, end = x, n;

                while (end < this->tliErosion.rows && this->tliErosion.at<float>(end,y) == 0)
                {
                    end++;
                }
                end--;
                n = end - head + 1;

                if (n <= 30){
                    std::vector<float> mpvH, mpvE;
                    double minH, maxH, minE, maxE;

                    for (int k=0; k<5; k++){
                        if ((head - k) >= 0)
                            mpvH.push_back(this->cei.at<float>(head-k,y));
                        if ((end + k) < this->cei.rows)
                            mpvE.push_back(this->cei.at<float>(end+k,y));
                    }

                    cv::minMaxLoc(mpvH, &minH, &maxH);
                    cv::minMaxLoc(mpvE, &minE, &maxE);

                    for (int m=0; m<n; m++)
                        intImg.at<float>(head + m,y) = static_cast<float>(maxH + (m+1) * ((maxE - maxH) / n));
                }
            }
        }
    }

    cv::Mat kernel = cv::Mat::ones(cv::Size(11, 11), CV_32F) * 1/121;
    filter2D(scale(intImg), this->ldi, CV_32F, kernel);

    grayscale = (this->cei/this->ldi) * 260;

    for (int y=0; y<this->tliErosion.rows; y++)
    {
        for (int x=0; x<this->tliErosion.cols; x++)
        {
            if (this->tliErosion.at<float>(y,x) != 0)
                grayscale.at<float>(y,x) *= 1.5;
        }
    }

    GaussianBlur(grayscale, grayscale, cv::Size(3,3), 2);
    grayscale.convertTo(grayscale, CV_8U);
}

void Binarization::getHistogram(const cv::Mat& image){
    std::vector<cv::Mat> bgrPlanes;
    cv::split(image, bgrPlanes);

    int histSize[] = {30};
    float bins[] = {0,300};
    const float *ranges[] = {bins};

    for (auto& bgrPlane : bgrPlanes)
        cv::calcHist(&bgrPlane, 1, 0, cv::Mat(), this->histogram, 1, histSize, ranges, true, true);

    getHR(static_cast<float>(std::sqrt(image.rows * image.cols)));
}

void Binarization::getHR(float sqrtHW){
    hr = 0;
    for (int i=0; i < histogram.rows; i++)
    {
        if (histogram.at<float>(i,0) > sqrtHW)
        {
            hr = static_cast<float>(i * 10);
            break;
        }
    }
}

void Binarization::getCEI(const cv::Mat& grayscale)
{
    cv::Mat cei = (grayscale - (this->hr + 50 * 0.4)) * 2;
    cv::normalize(cei, this->cei, 0, 255, cv::NORM_MINMAX, CV_32F);
    cv::threshold(this->cei, this->ceiBin, 59, 255, cv::THRESH_BINARY_INV);
}

void Binarization::getEdge(const cv::Mat& grayscale)
{
    float m1[] = {-1,0,1,-2,0,2,-1,0,1};
    float m2[] = {-2,-1,0,-1,0,1,0,1,2};
    float m3[] = {-1,-2,-1,0,0,0,1,2,1};
    float m4[] = {0,1,2,-1,0,1,-2,-1,0};

    cv::Mat kernel1(3, 3, CV_32F, m1);
    cv::Mat kernel2(3, 3, CV_32F, m2);
    cv::Mat kernel3(3, 3, CV_32F, m3);
    cv::Mat kernel4(3, 3, CV_32F, m4);

    cv::Mat eg1, eg2, eg3, eg4;
    cv::filter2D(grayscale, eg1, CV_32F, kernel1);
    eg1 = cv::abs(eg1);

    cv::filter2D(grayscale, eg2, CV_32F, kernel2);
    eg2 = cv::abs(eg2);

    cv::filter2D(grayscale, eg3, CV_32F, kernel3);
    eg3 = cv::abs(eg3);

    cv::filter2D(grayscale, eg4, CV_32F, kernel4);
    eg4 = cv::abs(eg4);

    this->egAvg = scale((eg1 + eg2 + eg3 + eg4)/4);
    cv::threshold(this->egAvg, this->egBin, 30, 255, cv::THRESH_BINARY);
}

void Binarization::getTLI(const cv::Mat& grayscale)
{
    this->tli = cv::Mat::ones(cv::Size(grayscale.cols, grayscale.rows), CV_32F) * 255;
    this->tli -= this->egBin;
    this->tli -= this->ceiBin;
    cv::threshold(this->tli, this->tli, 0, 255, cv::THRESH_BINARY);

    cv::Mat kernel = cv::Mat::ones(cv::Size(3, 3), CV_32F);
    cv::erode(this->tli, this->tliErosion, kernel);
    cv::threshold(this->tliErosion, this->tliErosion, 0, 255, cv::THRESH_BINARY);
}

cv::Mat Binarization::scale(const cv::Mat& image)
{
    double min, max;
    cv::minMaxLoc(image, &min, &max);

    cv::Mat res = image / (max - min);
    cv::minMaxLoc(res, &min, &max);
    res -= min;
    res *= 255;

    return res;
}
