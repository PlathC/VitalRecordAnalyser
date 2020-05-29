//
// Created by Platholl on 16/05/2020.
//

#ifndef CIVILREGISTRYANALYSER_CIVILREGISTRYANALYZER_HPP
#define CIVILREGISTRYANALYSER_CIVILREGISTRYANALYZER_HPP

#include <fstream>
#include <map>

#include <QFileDialog>
#include <QMainWindow>
#include <QMessageBox>

#undef slots
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>

#include "Model/TextDetection/TextDetection.hpp"
#include "Model/TextDetection/TextCorrection.hpp"
#define slots

#include "Model/ImageUtil.hpp"
#include "Ui/ImageSegmenterDialog.hpp"

namespace Ui
{
    class CivilRegistryAnalyzer;
}

using CsvHolder = std::map<std::string, std::string>;
using ParagraphHolder = std::vector<std::string>;
Q_DECLARE_METATYPE(std::string)
Q_DECLARE_METATYPE(ParagraphHolder)
Q_DECLARE_METATYPE(CsvHolder)

namespace CivilRegistryAnalyzer
{
    class TextDetectionThread : public QThread
    {
        Q_OBJECT
    public:
        explicit TextDetectionThread(const std::vector<cv::Mat>& imageFragments) :
            m_imageFragments(imageFragments)
        {}

        void run() override
        {
            std::string completeResult = "";
            TextDetection textDetection;
            for(const auto& img : m_imageFragments)
            {
                std::string result = textDetection.Process(img);

                if(!result.empty())
                {
                    completeResult += result;
                    emit progressChanged(QString::fromStdString(result));
                }
            }
            std::vector<std::string> textsParagraphs = textDetection.Correct(completeResult);
            emit onNewCorrectedText(textsParagraphs);

            for(auto& paragraph: textsParagraphs)
            {
                auto analysis = textDetection.AnalyseText(completeResult);

                emit onNewAnalysis(analysis);
            }

            emit finish();
        }
    signals:
        void progressChanged(QString newExtractedText);
        void onNewCorrectedText(std::vector<std::string> newExtractedText);
        void onNewAnalysis(std::map<std::string, std::string> newAnalysis);
        void finish();

    private:
        std::vector<cv::Mat> m_imageFragments;
    };

    class CivilRegistryAnalyzer : public QMainWindow
    {
    Q_OBJECT
    public:
        explicit CivilRegistryAnalyzer(QWidget* parent = nullptr);

        void resizeEvent(QResizeEvent* event);

        ~CivilRegistryAnalyzer() override;

    public slots:
        void onProgressChanged(QString info);
        void extractTextFinished();
        void onNewAnalysis(std::map<std::string, std::string> newAnalysis);
        void onNewCorrectedText(std::vector<std::string> newExtractedText);

    private slots:
        void OpenImage();
        void ExtractText();

    private:
        void UpdateUi();

    private:
        Ui::CivilRegistryAnalyzer* ui;
        cv::Mat m_src;
        std::vector<cv::Mat> m_imageFragments;
        std::vector<std::string> m_extractedText;
    };
}



#endif //CIVILREGISTRYANALYSER_CIVILREGISTRYANALYZER_HPP
