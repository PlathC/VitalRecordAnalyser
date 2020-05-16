#include <QApplication>

#include "include/Ui/DatasetBuilder.hpp"

int main(int argc, char** argv) {
    QApplication app{argc, argv};
    DatasetBuilder::DatasetBuilder window;
    window.show();

    return app.exec();
}