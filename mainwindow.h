#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <boost/filesystem.hpp>

#include <opencv2/core/core.hpp>

#include <QMainWindow>

using namespace boost::filesystem;
using namespace cv;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    path FileToOpen;
    path CurrentDir;
    path InputDirectory;

    Mat ImIn;
    Mat ImShow;

    int maxX;
    int maxY;

    int tileShape;
    int maxTileX;
    int maxTileY;
    int shiftTileX;
    int shiftTileY;
    int offsetTileX;
    int offsetTileY;

    int tileLineThickness;

    int *TilesX;
    int *TilesY;
    float


private slots:
    void on_pushButton_clicked();

    void on_FileListWidget_currentTextChanged(const QString &currentText);

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
