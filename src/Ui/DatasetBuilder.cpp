//
// Created by Platholl on 06/05/2020.
//

#include "Ui/DatasetBuilder.hpp"
#include "ui_DatasetBuilder.h"

namespace DatasetBuilder
{
    DatasetBuilder::DatasetBuilder(QWidget *parent) :
            QMainWindow(parent),
            ui(new Ui::DatasetBuilder)
    {
        ui->setupUi(this);
        ui->m_lblImg->setMinimumSize(1, 1);

        QObject::connect(ui->m_acInputFolders,
                &QAction::triggered,
                [&](){ SelectInputsImages(); }
                );

        QObject::connect(ui->m_acOutputFolder,
                 &QAction::triggered,
                 [&](){ SelectOutputFolder(); }
        );
    }

    void DatasetBuilder::UpdateUi()
    {
        if(m_currentImg)
        {
            ui->m_lblImg->setPixmap(
                    m_pixmapSrc.scaled(ui->m_lblImg->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation)
            );
        }
        else
        {
            ui->m_lblImg->setText("No selected file");
            ui->m_pbSkip->setEnabled(false);
            ui->m_pbSave->setEnabled(false);
        }
        ui->m_lwRemainingFolders->clear();
        for(const auto& set : m_sets)
        {
            ui->m_lwRemainingFolders->addItem(QString::fromStdString(set.SourceImage().filename().string())
                    + "(" + QString::number(set.Size()) + ")");
        }
    }

    void DatasetBuilder::SelectInputsImages()
    {
        auto* dialog = new QFileDialog(this);

        // Change dialog behaviour to accept multi-folders selection
        dialog->setFileMode(QFileDialog::FileMode::ExistingFiles);
        dialog->setWindowTitle("Select raw dataset images.");
        if(dialog->exec())
        {
            QStringList files = dialog->selectedFiles();
            for(const auto& file : files)
            {
                auto segmenterDialog = ImageSegmenterDialog(file);
                //segmenterDialog->setAttribute(Qt::WA_DeleteOnClose);

                if(segmenterDialog.exec())
                {
                    auto paragraphs = segmenterDialog.GetParagraphs();
                    if(!paragraphs.empty())
                    {
                        std::vector<cv::Mat> flattenPage;
                        for(const auto& paragraph : paragraphs)
                        {
                            flattenPage.insert(flattenPage.end(), paragraph.rbegin(), paragraph.rend());
                        }

                        m_sets.emplace_back(file.toStdString(), flattenPage, m_outputFolder);
                    }
                }
            }

            if(m_sets.empty())
                return;

            if(!m_currentImg)
            {
                m_currentSet = &m_sets.back();
                m_currentSet->SetOutputFolder(m_outputFolder);
                m_currentSet->SetTranscriptionPath(m_transcriptionPath);
                m_currentImg = &m_currentSet->CurrentImage();
                m_pixmapSrc = QPixmap::fromImage(ImageUtil::CvMatToQImage(m_currentImg->Image()));
            }

            UpdateUi();

            if(!m_outputFolderSelected)
            {
                QMessageBox::information(this,
                        "Missing data",
                        "You did not select the output folder yet which is needed for saving files",
                        QMessageBox::Ok);
                SelectOutputFolder();
            }

            ui->m_pbSave->setEnabled(true);
            ui->m_pbSkip->setEnabled(true);
        }
    }

    void DatasetBuilder::SelectOutputFolder()
    {
        auto* dialog = new QFileDialog(this);
        dialog->setFileMode(QFileDialog::DirectoryOnly);
        while(!dialog->exec())
        {}

        QStringList files = dialog->selectedFiles();
        m_outputFolder = files.takeFirst().toStdString();
        m_transcriptionPath = m_outputFolder + "/transcription.json";
        if(!fs::exists(m_transcriptionPath))
        {
            std::ofstream transcriptionFile(m_transcriptionPath);
            transcriptionFile.close();
        }
        m_outputFolderSelected = true;
        if(m_currentSet)
        {
            m_currentSet->SetOutputFolder(m_outputFolder);
            m_currentSet->SetTranscriptionPath(m_transcriptionPath);
        }
    }

    void DatasetBuilder::resizeEvent(QResizeEvent* event)
    {
        QMainWindow::resizeEvent(event);
        UpdateUi();
    }

    void DatasetBuilder::SkipCurrentImage()
    {
        NextImage(false);
    }

    void DatasetBuilder::Save()
    {
        m_currentImg->Text(ui->m_leTextImg->text().toStdString());
        NextImage(true);
        ui->m_leTextImg->clear();
    }

    void DatasetBuilder::keyPressEvent(QKeyEvent *event)
    {
        if( event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
        {
            if(ui->m_leTextImg->hasFocus())
            {
                Save();
            }
        }
        else if(event->key() == Qt::Key_Escape)
        {
            SkipCurrentImage();
        }
    }

    void DatasetBuilder::NextImage(bool save)
    {
        if(m_currentSet)
        {
            if(!m_currentSet->Skip(save))
            {
                m_sets.pop_back();
                if(!m_sets.empty())
                {
                    m_currentSet = &m_sets.back();
                    m_currentSet->SetOutputFolder(m_outputFolder);
                    m_currentSet->SetTranscriptionPath(m_transcriptionPath);
                    m_currentImg = &m_currentSet->CurrentImage();
                    m_pixmapSrc = QPixmap::fromImage(ImageUtil::CvMatToQImage(m_currentImg->Image()));
                }
                else
                {
                    m_currentSet = nullptr;
                    m_currentImg = nullptr;
                    m_pixmapSrc = QPixmap();
                }
            }
            else
            {
                m_currentImg = &m_currentSet->CurrentImage();
                m_pixmapSrc = QPixmap::fromImage(ImageUtil::CvMatToQImage(m_currentImg->Image()));
            }
            UpdateUi();
        }
    }

    DatasetBuilder::~DatasetBuilder()
    {
        delete ui;
    }
}