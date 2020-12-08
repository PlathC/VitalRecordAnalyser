//
// Created by Platholl on 16/05/2020.
//

#ifndef CIVILREGISTRYANALYSER_CIVILREGISTRYANALYZER_HPP
#define CIVILREGISTRYANALYSER_CIVILREGISTRYANALYZER_HPP

#include <fstream>
#include <map>
#include <utility>

#include <QCoreApplication>
#include <QFileDialog>
#include <QMainWindow>
#include <QMessageBox>

#undef slots
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>

#include "Model/TextDetection/TextDetection.hpp"
#define slots

#include "Model/ImageUtil.hpp"
#include "Ui/ImageSegmenterDialog.hpp"
#include "Ui/PyOutputRedirector.hpp"

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
        explicit TextDetectionThread(const std::vector<std::vector<cv::Mat>>& paragraphs):
                m_paragraphs(paragraphs)
        {}

        void run() override
        {
            pybind11::gil_scoped_acquire acquire;

            CivilRegistryAnalyser::PyOutputRedirector redirector{};

            auto callback = [&](QString content){
                emit onNewOutput(content);
            };

            QObject::connect(&redirector, &CivilRegistryAnalyser::PyOutputRedirector::newStdOutContent, callback);
            QObject::connect(&redirector, &CivilRegistryAnalyser::PyOutputRedirector::newStdErrContent, callback);

            std::vector<std::vector<std::string>> paragraphsResults{};
            std::string completeResult{};
            TextDetection textDetection{};

            for(const auto& paragraph : m_paragraphs)
            {
                for(const auto& word : paragraph)
                {
                    std::string result = textDetection.Process(word);
                    if(!result.empty())
                    {
                        completeResult += result;
                        emit progressChanged(QString::fromStdString(result));
                    }
                }

                if(!completeResult.empty())
                {
                    paragraphsResults.emplace_back(completeResult);
                    completeResult.clear();
                }
            }
            emit onNewOutput("--INFO-- End of detection");
            emit onNewOutput("--INFO-- Start correction");

            for(auto& paragraph : paragraphsResults)
            {
                for(auto& words : paragraph)
                    words = textDetection.Correct(words);
            }
            emit onNewCorrectedText(paragraphsResults);

            emit onNewOutput("--INFO-- End of correction");
            emit onNewOutput("--INFO-- Start analysis");

            for(auto& paragraph: paragraphsResults)
            {
                for(auto& words : paragraph)
                {
                    auto analysis = textDetection.AnalyseText(words);
                    emit onNewAnalysis(analysis);
                }
            }

            emit finish();
        }
    signals:
        void onNewOutput(QString);
        void progressChanged(QString newExtractedText);
        void onNewCorrectedText(std::vector<std::vector<std::string>> newExtractedText);
        void onNewAnalysis(std::map<std::string, std::string> newAnalysis);
        void finish();

    private:
        // See :
        // https://github.com/pybind/pybind11/issues/1723#issuecomment-475226782
        // https://github.com/pybind/pybind11/issues/1273#issuecomment-366449829
        pybind11::gil_scoped_release guard{};
        std::vector<std::vector<cv::Mat>> m_paragraphs;
    };

    class CivilRegistryAnalyzer : public QMainWindow
    {
    Q_OBJECT
    public:
        explicit CivilRegistryAnalyzer(QWidget* parent = nullptr);

        void resizeEvent(QResizeEvent* event) override;

        ~CivilRegistryAnalyzer() override;

    public slots:
        void onProgressChanged(QString info);
        void extractTextFinished();
        void onNewAnalysis(std::map<std::string, std::string> newAnalysis);
        void onNewCorrectedText(std::vector<std::vector<std::string>> newExtractedText);

    private slots:
        void OpenImage();
        void ExtractText();

    private:
        void UpdateUi();

    private:
        Ui::CivilRegistryAnalyzer* ui;
        cv::Mat m_src;
        QPixmap m_pixmapSrc;
        std::vector<std::vector<cv::Mat>> m_paragraphsFragments;
        std::vector<std::vector<std::string>> m_extractedText;

        py::scoped_interpreter interpreter{};

    };
}



#endif //CIVILREGISTRYANALYSER_CIVILREGISTRYANALYZER_HPP
