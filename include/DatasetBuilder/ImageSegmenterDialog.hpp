//
// Created by Platholl on 08/05/2020.
//

#ifndef CIVILREGISTRYANALYSER_IMAGESEGMENTERDIALOG_HPP
#define CIVILREGISTRYANALYSER_IMAGESEGMENTERDIALOG_HPP

#include <filesystem>

#include <QDialog>
#include <QThread>
#include <QTimer>

#include "TextSegmentation/TextSegmentation.hpp"
#include "DatasetBuilder/DatasetImage.hpp"

namespace fs = std::filesystem;

namespace Ui {
    class ImageSegmenterDialog;
}

namespace DatasetBuilder
{
    class ImageSegmenterDialog : public QDialog
    {
    Q_OBJECT
    public:
        explicit ImageSegmenterDialog(QString imgPath, QString outputPath, QWidget* parent = nullptr);

        [[nodiscard]] std::vector<cv::Mat> GetImages();


        ~ImageSegmenterDialog() override;

    private slots:
        void UpdateProgress();

    private:
        static const std::array<std::string, 3> SupportedImageFiles;
        std::unique_ptr<TextSegmentation> segmentor;

        Ui::ImageSegmenterDialog* ui;

        std::string m_outputFolder;
        std::string m_path;

        std::vector<cv::Mat> m_images;
        QTimer* m_progressTimer = nullptr;
    };
}

#endif //CIVILREGISTRYANALYSER_IMAGESEGMENTERDIALOG_HPP
