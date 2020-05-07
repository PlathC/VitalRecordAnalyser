//
// Created by Platholl on 06/05/2020.
//

#ifndef CIVILREGISTRYANALYSER_MAINWINDOW_HPP
#define CIVILREGISTRYANALYSER_MAINWINDOW_HPP

#include <stack>
#include <vector>

#include <QDebug>
#include <QFileDialog>
#include <QKeyEvent>
#include <QListView>
#include <QMainWindow>
#include <QMessageBox>
#include <QTreeView>

#include "ImageUtil.hpp"

#include "DatasetBuilder/ImageSet.hpp"


namespace Ui
{
    class MainWindow;
}

namespace DatasetBuilder
{
    class MainWindow : public QMainWindow
    {
    Q_OBJECT
    public:
        explicit MainWindow(QWidget* parent = nullptr);
        void UpdateUi();
        void SelectInputFolders();
        void SelectOutputFolder();

        ~MainWindow() override;

    private slots:
        void SkipCurrentImage();
        void Save();
        void keyPressEvent(QKeyEvent *event) override;

    private:
        void NextImage(bool save);

    private:
        Ui::MainWindow *ui;
        std::vector<DatasetBuilder::ImageSet> m_sets;
        DatasetBuilder::DatasetImage* m_currentImg = nullptr;
        DatasetBuilder::ImageSet* m_currentSet = nullptr;

        bool m_outputFolderSelected = false;
        std::string m_outputFolder;
    };

}

#endif // CIVILREGISTRYANALYSER_MAINWINDOW_HPP