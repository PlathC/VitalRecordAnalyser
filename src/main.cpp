#include <QApplication>

#include <DatasetBuilder/MainWindow.hpp>

int main(int argc, char** argv)
{
    QApplication app {argc, argv};
    DatasetBuilder::MainWindow window;
    window.show();

    return app.exec();
}