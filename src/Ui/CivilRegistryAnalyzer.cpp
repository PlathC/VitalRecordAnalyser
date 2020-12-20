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
        ui->m_lblImg->setMinimumSize(1, 1);

        QObject::connect(ui->m_pbOpenImage,
                         &QPushButton::clicked,
                         [&](){ OpenImage(); }
        );
        QObject::connect(ui->m_pbLaunchAnalysis,
                         &QPushButton::clicked,
                         [&](){ ExtractText(); }
        );
    }

    void CivilRegistryAnalyzer::UpdateUi()
    {
        if(!m_src.empty())
        {
            ui->m_lblImg->setPixmap(
                    m_pixmapSrc.scaled(ui->m_lblImg->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation)
            );
        }

        if(!m_extractedText.empty())
        {
            QString detectedText = "";
            for(const auto& txt : m_extractedText)
            {
                for(const auto& word : txt)
                    detectedText += QString::fromStdString(word);
                detectedText += "\n";
            }
            QString txt = ui->m_ptDetectedText->toPlainText();
            ui->m_ptDetectedText->clear();
            ui->m_ptDetectedText->setPlainText(detectedText);
            ui->m_ptDetectedText->moveCursor (QTextCursor::End);
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
        m_extractedText.emplace_back(std::vector<std::string>{info.toStdString() + "\n"});
        UpdateUi();
    }

    void CivilRegistryAnalyzer::extractTextFinished()
    {
        ui->m_pbLaunchAnalysis->setEnabled(true);
        QMessageBox::information(this, "Information", "The analysis is done, you can find"
                                                      " the outputCsv.csv at the root path of this program.");

        //pybind11::gil_scoped_acquire();
    }

    void CivilRegistryAnalyzer::onNewAnalysis(std::map<std::string, std::string> newAnalysis)
    {
        const std::string outputCsv = "outputCsv.csv";

        std::ifstream readFile(outputCsv);

        if(!readFile.good())
        {
            readFile.close();
            std::ofstream writeFile{ outputCsv, std::ios::app };
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
    }

    void CivilRegistryAnalyzer::onNewCorrectedText(std::vector<std::string> newExtractedText)
    {
        m_extractedText.clear();
        for(const auto& nText : newExtractedText)
        {
            m_extractedText.emplace_back(std::vector<std::string>{nText});
        }
        UpdateUi();
    }

    void CivilRegistryAnalyzer::OpenImage()
    {
        auto* dialog = new QFileDialog(this);

        dialog->setFileMode(QFileDialog::FileMode::ExistingFile);
        dialog->setWindowTitle("Select raw dataset images.");
        if(dialog->exec()) {
            m_paragraphsFragments.clear();
            m_extractedText.clear();

            std::string file = dialog->selectedFiles()[0].toStdString();
            m_src = cv::imread(file);
            m_pixmapSrc = QPixmap::fromImage(ImageUtil::CvMatToQImage(m_src));
            UpdateUi();

            auto segmentationDialog = DatasetBuilder::ImageSegmenterDialog(QString::fromStdString(file));
            if (segmentationDialog.exec())
            {
                auto paragraphs = segmentationDialog.GetParagraphs();

                if (!paragraphs.empty())
                {
                    for(auto& paragraph : paragraphs)
                    {
                        if(!paragraph.empty())
                        {
                            m_paragraphsFragments.emplace_back(paragraph);
                        }
                    }
                }
                ui->m_pbLaunchAnalysis->setEnabled(true);
                UpdateUi();
            }
        }
    }

    void CivilRegistryAnalyzer::ExtractText()
    {
        if(!m_paragraphsFragments.empty())
        {
            ui->m_pbLaunchAnalysis->setEnabled(false);

            m_workerThread = new TextRecognitionThread(this, m_paragraphsFragments);

            qRegisterMetaType<std::vector<std::vector<std::string>>>("std::vector<std::vector<std::string>>");
            qRegisterMetaType<std::map<std::string, std::string>>("std::map<std::string, std::string>");

            // Connect our signal and slot
            QObject::connect(m_workerThread, &TextRecognitionThread::onNewOutput,
                             this, [&](const QString& content){
                        ui->m_tePromptOutput->moveCursor (QTextCursor::End);
                        ui->m_tePromptOutput->insertPlainText(content + '\n');
                        ui->m_tePromptOutput->moveCursor (QTextCursor::End);
                    });

            QObject::connect(m_workerThread, &TextRecognitionThread::progressChanged,
                             this, &CivilRegistryAnalyzer::onProgressChanged, Qt::BlockingQueuedConnection);

            QObject::connect(m_workerThread, &TextRecognitionThread::onNewCorrectedText,
                             this, &CivilRegistryAnalyzer::onNewCorrectedText, Qt::BlockingQueuedConnection);

            QObject::connect(m_workerThread, &TextRecognitionThread::onNewAnalysis,
                             this, &CivilRegistryAnalyzer::onNewAnalysis, Qt::BlockingQueuedConnection);

            QObject::connect(m_workerThread, &TextRecognitionThread::finish,
                             this, &CivilRegistryAnalyzer::extractTextFinished, Qt::BlockingQueuedConnection);

            QObject::connect(m_workerThread, &QThread::finished,
                             m_workerThread, &TextRecognitionThread::deleteLater, Qt::DirectConnection);

            m_workerThread->start();
        }

    }

    CivilRegistryAnalyzer::~CivilRegistryAnalyzer()
    {
        delete ui;
    }
}