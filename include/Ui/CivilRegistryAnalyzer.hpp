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

namespace Ui
{
    class CivilRegistryAnalyzer;
}

using CsvHolder = std::map<std::string, std::string>;
using RawResultHolder = std::vector<std::vector<std::string>>;
using ParagraphHolder = std::vector<std::string>;
Q_DECLARE_METATYPE(std::string)
Q_DECLARE_METATYPE(RawResultHolder)
Q_DECLARE_METATYPE(ParagraphHolder)
Q_DECLARE_METATYPE(CsvHolder)

namespace CivilRegistryAnalyzer
{
    class TextDetectionThread : public QThread
    {
        Q_OBJECT
    public:
        explicit TextDetectionThread(QObject* parent = nullptr,
                                     std::vector<std::vector<cv::Mat>> paragraphs = {},
                                     bool withCorrection = false,
                                     bool withAnalysis = false):
                QThread(parent),
                m_paragraphs(std::move(paragraphs)),
                m_withCorrection(withCorrection),
                m_withAnalysis(withAnalysis)
        {
        }

        void run() override
        {
            pybind11::gil_scoped_acquire acquire{};

            std::vector<std::string> paragraphsResults{};
            TextDetection textDetection{};

            emit onNewOutput("--INFO-- Start text extraction");
            size_t paragraphCount = 0;
            for(const auto& paragraph : m_paragraphs)
            {
                std::string tempParagraph{};
                size_t wordCount = 0;
                for(const auto& word : paragraph)
                {
                    emit onNewOutput(QString::fromStdString("--INFO-- Processing " + std::to_string(wordCount++)
                                + " out of " + std::to_string(paragraph.size()) + " on "
                                + std::to_string(paragraphCount) + " out of "
                                + std::to_string(m_paragraphs.size()) + "paragraphs"));
                    cv::Mat temp = word.clone();
                    if(word.channels() != 1 )
                    {
                        cv::cvtColor(temp, temp, cv::COLOR_BGR2GRAY);
                    }
                    std::string result = textDetection.Process(temp);
                    if(!result.empty())
                    {
                        tempParagraph += result + ' ';
                        emit progressChanged(QString::fromStdString(result));
                    }
                }

                paragraphCount++;
                if(!tempParagraph.empty())
                {
                    paragraphsResults.emplace_back(tempParagraph);
                }
            }

            emit onNewOutput("--INFO-- End of detection");

            if (m_withCorrection)
            {
                emit onNewOutput("--INFO-- Start correction");
                std::vector<std::string> result{};
                for(auto& paragraph : paragraphsResults)
                {
                    result.emplace_back(textDetection.Correct(paragraph));
                }
                emit onNewCorrectedText(result);

                emit onNewOutput("--INFO-- End of correction");
            }
            else
            {
                emit onNewOutput("--INFO-- Skip correction, provide raw paragraphs");
                std::vector<std::string> result{};
                for(auto& paragraph : paragraphsResults)
                {
                    result.emplace_back(paragraph);
                }
                emit onNewCorrectedText(result);
            }


            if (m_withAnalysis)
            {
                emit onNewOutput("--INFO-- Start analysis");
                for(auto& paragraph: paragraphsResults)
                {
                    auto analysis = textDetection.AnalyseText(paragraph);
                    emit onNewAnalysis(analysis);
                }
                emit onNewOutput("--INFO-- END analysis");
            }

            emit finish();
        }

    signals:
        void onNewOutput(QString);
        void progressChanged(QString newExtractedText);
        void onNewCorrectedText(std::vector<std::string> newExtractedText);
        void onNewAnalysis(std::map<std::string, std::string> newAnalysis);
        void finish();

    private:
        // See :
        // https://github.com/pybind/pybind11/issues/1723#issuecomment-475226782
        // https://github.com/pybind/pybind11/issues/1273#issuecomment-366449829
        pybind11::gil_scoped_release guard{};
        std::vector<std::vector<cv::Mat>> m_paragraphs;
        bool m_withAnalysis;
        bool m_withCorrection;
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
        void onNewCorrectedText(std::vector<std::string> newExtractedText);

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

        TextDetectionThread* m_workerThread = nullptr;

        py::scoped_interpreter interpreter{};

    };
}



#endif //CIVILREGISTRYANALYSER_CIVILREGISTRYANALYZER_HPP
