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
                detectedText += QString::fromStdString(txt) + "\n";
            }
            QString txt = ui->m_ptDetectedText->toPlainText();
            ui->m_ptDetectedText->clear();
            ui->m_ptDetectedText->setPlainText(detectedText);
        }
        else
        {
            ui->m_ptDetectedText->clear();
        }
    }

    void CivilRegistryAnalyzer::resizeEvent(QResizeEvent* event)
    {
        QMainWindow::resizeEvent(event);
        UpdateUi();
    }

    void CivilRegistryAnalyzer::onProgressChanged(QString info)
    {
        m_extractedText.emplace_back(info.toStdString() + "\n");
        UpdateUi();
    }

    void CivilRegistryAnalyzer::extractTextFinished()
    {
        ui->m_pbDetectWords->setEnabled(true);
    }

    void CivilRegistryAnalyzer::onNewAnalysis(std::map<std::string, std::string> newAnalysis)
    {
        const std::string outputCsv = "outputCsv.csv";

        std::ifstream readFile(outputCsv);

        if(!readFile.good())
        {
            readFile.close();
            std::ofstream writeFile{outputCsv, std::ios::app};
            for (const auto & newAnalyse : newAnalysis)
            {
                writeFile << newAnalyse.first << ",";
            }
            writeFile << std::endl;
            writeFile.close();
        }
        std::ofstream writeFile{outputCsv, std::ios::app};

        for (const auto & newAnalyse : newAnalysis)
        {
            writeFile << newAnalyse.second << ",";
        }
        writeFile << std::endl;
        writeFile.close();

        QMessageBox::information(this, "Information", "The analysis is done, you can find"
                                                      " the outputCsv.csv at the root path of this program.");
    }

    void CivilRegistryAnalyzer::onNewCorrectedText(std::vector<std::string> newExtractedText)
    {
        m_extractedText.clear();
        for(auto& ntext : newExtractedText)
        {
            m_extractedText.emplace_back(ntext);
        }
        UpdateUi();
    }


    void CivilRegistryAnalyzer::OpenImage()
    {
        auto* dialog = new QFileDialog(this);

        dialog->setFileMode(QFileDialog::FileMode::ExistingFile);
        dialog->setWindowTitle("Select raw dataset images.");
        if(dialog->exec()) {
            m_imageFragments.clear();
            m_extractedText.clear();

            std::string file = dialog->selectedFiles()[0].toStdString();
            m_src = cv::imread(file);
            UpdateUi();

            auto *segmenterDialog = new DatasetBuilder::ImageSegmenterDialog(QString::fromStdString(file), this);
            if (segmenterDialog->exec())
            {
                auto images = segmenterDialog->GetImages();

                if (!images.empty())
                {
                    m_imageFragments.insert(m_imageFragments.end(), images.begin(), images.end());
                }
                ui->m_pbDetectWords->setEnabled(true);
                UpdateUi();
            }
        }
    }

    void CivilRegistryAnalyzer::ExtractText()
    {
        if(!m_imageFragments.empty())
        {

            std::reverse(std::begin(m_imageFragments), std::end(m_imageFragments));

            auto* workerThread = new TextDetectionThread(m_imageFragments);

            qRegisterMetaType<std::vector<std::string> >("std::vector<std::string>");
            qRegisterMetaType<std::map<std::string, std::string>>("std::map<std::string, std::string>");

            // Connect our signal and slot
            QObject::connect(workerThread, &TextDetectionThread::progressChanged,
                             this, &CivilRegistryAnalyzer::onProgressChanged);

            QObject::connect(workerThread, &TextDetectionThread::onNewCorrectedText,
                             this, &CivilRegistryAnalyzer::onNewCorrectedText);

            QObject::connect(workerThread, &TextDetectionThread::onNewAnalysis,
                             this, &CivilRegistryAnalyzer::onNewAnalysis);

            QObject::connect(workerThread, &QThread::finished,
                             workerThread, &QObject::deleteLater);

            QObject::connect(workerThread, &QThread::finished,
                        this, &CivilRegistryAnalyzer::extractTextFinished);

            ui->m_pbDetectWords->setEnabled(false);

            workerThread->start();
        }

    }

    CivilRegistryAnalyzer::~CivilRegistryAnalyzer()
    {
        delete ui;
    }
}