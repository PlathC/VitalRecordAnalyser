//
// Created by Platholl on 16/05/2020.
//

#include "Ui/CivilRegistryAnalyzer.hpp"
#include "ui_CivilRegistryAnalyzer.h"

namespace CivilRegistryAnalyzer
{
    CivilRegistryAnalyzer::CivilRegistryAnalyzer(QWidget* parent):
        QMainWindow(parent),
        ui(new Ui::CivilRegistryAnalyzer)
    {
        ui->setupUi(this);
        QObject::connect(ui->m_acOpenImage,
                         &QAction::triggered,
                         [&](){ OpenImage(); }
        );
    }

    void CivilRegistryAnalyzer::UpdateUi()
    {
        if(!m_src.empty())
        {
            cv::Mat nImg;
            int width  = m_src.cols,
                height = m_src.rows;

            int target_width = ui->m_lblImg->width();
            cv::Mat square   = cv::Mat::zeros( target_width, target_width, m_src.type() );

            int max_dim = ( width >= height ) ? width : height;
            float scale = ( ( float ) target_width ) / max_dim;
            cv::Rect roi;
            if ( width >= height )
            {
                roi.width = target_width;
                roi.x = 0;
                roi.height = height * scale;
                roi.y = ( target_width - roi.height ) / 2;
            }
            else
            {
                roi.y = 0;
                roi.height = target_width;
                roi.width = width * scale;
                roi.x = ( target_width - roi.width ) / 2;
            }

            cv::resize( m_src, square( roi ), roi.size());

            ui->m_lblImg->setPixmap(
                    QPixmap::fromImage(
                            ImageUtil::CvMatToQImage(square)
                    )
            );
        }
        if(!m_extractedText.empty())
        {
            QString detectedText = "";
            for(const auto& txt : m_extractedText)
            {
                detectedText += QString::fromStdString(txt);
            }
            ui->m_lblDetectedText->setText(detectedText);
        }

    }

    void CivilRegistryAnalyzer::resizeEvent(QResizeEvent* event)
    {
        QMainWindow::resizeEvent(event);
        UpdateUi();
    }

    void CivilRegistryAnalyzer::OpenImage()
    {
        auto* dialog = new QFileDialog(this);

        dialog->setFileMode(QFileDialog::FileMode::ExistingFile);
        dialog->setWindowTitle("Select raw dataset images.");
        if(dialog->exec()) {
            std::string file = dialog->selectedFiles()[0].toStdString();
            m_src = cv::imread(file);
            UpdateUi();
            auto *segmenterDialog = new DatasetBuilder::ImageSegmenterDialog(QString::fromStdString(file), this);

            if (segmenterDialog->exec()) {
                auto images = segmenterDialog->GetImages();
                if (!images.empty()) {
                    m_imageFragments.insert(m_imageFragments.end(), images.begin(), images.end());
                }
            }
        }
    }

    void CivilRegistryAnalyzer::ExtractText()
    {
        if(!m_imageFragments.empty())
        {
            if(!m_textDetection)
                m_textDetection = std::make_unique<TextDetection>();
            int i = 0;
            std::reverse(std::begin(m_imageFragments), std::end(m_imageFragments));
            for(const auto& img : m_imageFragments)
            {
                std::string result = m_textDetection->Process(img);

                if(!result.empty())
                {
                    m_extractedText.emplace_back( result + "\n");
                    std::cout << std::to_string(i++) << " ==> " << m_extractedText.back() << std::endl;
                    UpdateUi();
                }
                else
                {
                    std::cout << std::to_string(i++) << " ==> Empty !" << std::endl;
                }

            }
        }

    }

    CivilRegistryAnalyzer::~CivilRegistryAnalyzer()
    {
        delete ui;
    }
}