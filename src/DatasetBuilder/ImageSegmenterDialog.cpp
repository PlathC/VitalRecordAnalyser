//
// Created by Platholl on 08/05/2020.
//

#include "DatasetBuilder/ImageSegmenterDialog.hpp"
#include "ui_ImageSegmenterDialog.h"

namespace DatasetBuilder
{
    const std::array<std::string, 3> ImageSegmenterDialog::SupportedImageFiles = {
            ".jpg", ".jpeg", ".png"
    };

    ImageSegmenterDialog::ImageSegmenterDialog(QString imgPath, QString outputPath, QWidget* parent):
            QDialog(parent),
            ui(new Ui::ImageSegmenterDialog),
            m_outputFolder(outputPath.toStdString()),
            m_path(imgPath.toStdString())
    {
        ui->setupUi(this);

        fs::path file {m_path};
        if(file.has_filename())
        {
            bool isFileSupported = std::find(
                    SupportedImageFiles.begin(),
                    SupportedImageFiles.end(),
                    file.extension().string())
                                   != SupportedImageFiles.end();
            if(!isFileSupported)
            {
                this->close();
            }
            else
            {
                ui->m_buttonBox->setDisabled(true);
                segmentor = std::make_unique<TextSegmentation>(file.string());

                m_progressTimer = new QTimer(this);
                ui->m_pbTask->setValue(0);
                QObject::connect(m_progressTimer, &QTimer::timeout, this, &ImageSegmenterDialog::UpdateProgress);
                m_progressTimer->setSingleShot(false);
                //nTimer->setInterval(100);
                m_progressTimer->start(100);
                segmentor->Start();
            }
        }
    }

    void ImageSegmenterDialog::UpdateProgress()
    {
        uint8_t progress = segmentor->Progress();

        ui->m_pbTask->setValue(progress);
        if(progress == 100)
        {
            segmentor->Join();
            m_images = segmentor->GetExtractedWords();
            m_progressTimer->stop();
            ui->m_buttonBox->setDisabled(false);
            ui->m_lblTaskInformation->setText("Task done !");
        }
        else
        {
            // TODO: Check why this is needed
            m_progressTimer->start(100);
        }
    }

    std::vector<cv::Mat> ImageSegmenterDialog::GetImages()
    {
        return m_images;
    }

    ImageSegmenterDialog::~ImageSegmenterDialog()
    {
        delete m_progressTimer;
        delete ui;
    }
}

