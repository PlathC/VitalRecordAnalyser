//
// Created by Platholl on 06/05/2020.
//

#include "DatasetBuilder/MainWindow.hpp"
#include "ui_MainWindow.h"

namespace DatasetBuilder
{
    MainWindow::MainWindow(QWidget *parent) :
            QMainWindow(parent),
            ui(new Ui::MainWindow)
    {
        ui->setupUi(this);
        QObject::connect(ui->m_acInputFolders,
                &QAction::triggered,
                [&](){ SelectInputFolders(); }
                );
        QObject::connect(ui->m_acOutputFolder,
                 &QAction::triggered,
                 [&](){ SelectOutputFolder(); }
        );
    }

    void MainWindow::UpdateUi()
    {
        if(m_currentImg)
        {
            auto img = m_currentImg->Image();
            cv::Mat nImg;
            int width  = img.cols,
                height = img.rows;

            int target_width = ui->m_lblImg->width();
            cv::Mat square   = cv::Mat::zeros( target_width, target_width, img.type() );

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

            cv::resize( img, square( roi ), roi.size());

            ui->m_lblImg->setPixmap(
                    QPixmap::fromImage(
                            ImageUtil::CvMatToQImage(square)
                            )
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
            ui->m_lwRemainingFolders->addItem(QString::fromStdString(set.FolderPath()));
        }
    }

    void MainWindow::SelectInputFolders()
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
                m_sets.emplace_back(DatasetBuilder::ImageSet(file.toStdString(), m_outputFolder));
                QMessageBox::information(this, "Success", "File " + file + " extracted !");
            }

            if(!m_currentImg)
            {
                m_currentSet = &m_sets.back();
                m_currentSet->SetOutputFolder(m_outputFolder);
                m_currentImg = &m_currentSet->CurrentImage();
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

    void MainWindow::SelectOutputFolder()
    {
        auto* dialog = new QFileDialog(this);
        dialog->setFileMode(QFileDialog::DirectoryOnly);
        while(!dialog->exec())
        {}

        QStringList files = dialog->selectedFiles();
        m_outputFolder = files.takeFirst().toStdString();
        m_outputFolderSelected = true;
        if(m_currentSet)
            m_currentSet->SetOutputFolder(m_outputFolder);
    }

    void MainWindow::SkipCurrentImage()
    {
        NextImage(false);
    }

    void MainWindow::Save()
    {
        m_currentImg->Text(ui->m_leTextImg->text().toStdString());
        NextImage(true);
    }

    void MainWindow::keyPressEvent(QKeyEvent *event)
    {
        if( event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
        {
            if(ui->m_leTextImg->hasFocus())
            {
                Save();
                ui->m_leTextImg->clear();
            }
        }
        else if(event->key() == Qt::Key_Escape)
        {
            SkipCurrentImage();
        }
    }

    void MainWindow::NextImage(bool save)
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
                    m_currentImg = &m_currentSet->CurrentImage();
                }
                else
                {
                    m_currentSet = nullptr;
                    m_currentImg = nullptr;
                }
            }
            else
            {
                m_currentImg = &m_currentSet->CurrentImage();
            }
            UpdateUi();
        }
    }

    MainWindow::~MainWindow()
    {
        delete ui;
    }
}