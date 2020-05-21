//
// Created by Platholl on 16/05/2020.
//

#ifndef CIVILREGISTRYANALYSER_CIVILREGISTRYANALYZER_HPP
#define CIVILREGISTRYANALYSER_CIVILREGISTRYANALYZER_HPP

#include <QFileDialog>
#include <QMainWindow>
#include <QMessageBox>

#undef slots
#include "Model/TextDetection/TextDetection.hpp"
#define slots

#include "Model/ImageUtil.hpp"
#include "Ui/ImageSegmenterDialog.hpp"
#include "Model/TextDetection/TextCorrection.hpp"

namespace Ui
{
    class CivilRegistryAnalyzer;
}

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
            m_textDetection = std::make_unique<TextDetection>();
            for(const auto& img : m_imageFragments)
            {
                std::string result = m_textDetection->Process(img);

                if(!result.empty())
                {
                    emit progressChanged(QString::fromStdString(result));
                }
            }
            emit finish();
        }
    signals:
        void progressChanged(QString newExtractedText);
        void finish();
    private:
        std::unique_ptr<TextDetection> m_textDetection;
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
        std::unique_ptr<TextCorrection> m_textCorrector;
    };
}



#endif //CIVILREGISTRYANALYSER_CIVILREGISTRYANALYZER_HPP
