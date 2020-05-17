//
// Created by Platholl on 16/05/2020.
//

#ifndef CIVILREGISTRYANALYSER_CIVILREGISTRYANALYZER_HPP
#define CIVILREGISTRYANALYSER_CIVILREGISTRYANALYZER_HPP

#include <QMainWindow>
#include <QFileDialog>

#undef slots
#include "Model/TextDetection/TextDetection.hpp"
#define slots

#include "Model/ImageUtil.hpp"
#include "Ui/ImageSegmenterDialog.hpp"

namespace Ui
{
    class CivilRegistryAnalyzer;
}

namespace CivilRegistryAnalyzer
{
    class CivilRegistryAnalyzer : public QMainWindow
    {
    Q_OBJECT
    public:
        explicit CivilRegistryAnalyzer(QWidget* parent = nullptr);

        void resizeEvent(QResizeEvent* event);

        ~CivilRegistryAnalyzer() override;

    private slots:
        void OpenImage();
        void ExtractText();

    private:
        void UpdateUi();

    private:
        Ui::CivilRegistryAnalyzer* ui;
        cv::Mat m_src;
        std::vector<cv::Mat> m_imageFragments;
        std::unique_ptr<TextDetection> m_textDetection;
        std::vector<std::string> m_extractedText;
    };
}



#endif //CIVILREGISTRYANALYSER_CIVILREGISTRYANALYZER_HPP
