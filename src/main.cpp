#include <iostream>

#include <QApplication>

#include <DatasetBuilder/MainWindow.hpp>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

int main(int argc, char** argv)
{
    // deserialize from standard input
    json j = json::parse("{ \"happy\": true, \"pi\": 3.141 }");

    // serialize to standard output
    std::cout << j;

    // the setw manipulator was overloaded to set the indentation for pretty printing
    std::cout << std::setw(4) << j << std::endl;

    QApplication app {argc, argv};
    DatasetBuilder::MainWindow window;
    window.show();

    return app.exec();
}