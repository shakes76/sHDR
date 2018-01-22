/**
    \brief This program constructs Shape Shifter and executes the milxQtMain
    \author Shekhar S. Chandra, 2010
*/
//Qt
#include <QApplication>

#include "milxQtHDRMain.h"

int main(int argc, char* argv[])
{
    QApplication app(argc,argv);
    milxQtHDRMain Main;

    Main.setWindowTitle("HDR");
        Main.show();
        app.processEvents();

//    QPixmap pixmap(":resources/smilx_splash.png");
    QPixmap icon(":resources/smilx_icon.png");
//    QSplashScreen splash(pixmap);
//        splash.show();
//    app.processEvents();

    app.setWindowIcon(QIcon(icon));
//    splash.showMessage("This software is for research purposes only and is NOT approved for clinical use", Qt::AlignBottom | Qt::AlignHCenter);
    app.processEvents();

    ///Open files if provided
    QStringList files = app.arguments();
        files.erase(files.begin()); //First element is the program name
        Main.loadFiles(files);

    Main.startWizard();

    return app.exec();
}
