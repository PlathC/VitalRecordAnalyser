//
// Created by Platholl on 06/05/2020.
//

#ifndef CIVILREGISTRYANALYSER_DATASETBUILDER_HPP
#define CIVILREGISTRYANALYSER_DATASETBUILDER_HPP

#include <stack>
#include <vector>

#include <QDebug>
#include <QFileDialog>
#include <QKeyEvent>
#include <QListView>
#include <QMainWindow>
#include <QMessageBox>
#include <QTreeView>

#include "include/Model/ImageUtil.hpp"

#include "include/Model/Dataset/ImageSet.hpp"
#include "ImageSegmenterDialog.hpp"

namespace Ui
{
    class DatasetBuilder;
}

namespace DatasetBuilder
{
    class DatasetBuilder : public QMainWindow
    {
    Q_OBJECT
    public:
        explicit DatasetBuilder(QWidget* parent = nullptr);
        void UpdateUi();
        void SelectInputsImages();
        void SelectOutputFolder();

        ~DatasetBuilder() override;

    private slots:
        void SkipCurrentImage();
        void Save();
        void keyPressEvent(QKeyEvent *event) override;

    private:
        void NextImage(bool save);

    private:
        Ui::DatasetBuilder *ui;
        std::vector<ImageSet> m_sets;
        DatasetImage* m_currentImg = nullptr;
        ImageSet* m_currentSet = nullptr;

        bool m_outputFolderSelected = false;
        std::string m_outputFolder;
        fs::path m_transcriptionPath;
    };

}

#endif // CIVILREGISTRYANALYSER_DATASETBUILDER_HPP