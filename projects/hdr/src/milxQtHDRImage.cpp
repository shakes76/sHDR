
#include "milxQtHDRImage.h"

milxQtHDRImage::milxQtHDRImage(QWidget *theParent) : milxQtImage(theParent, false)
{
    center.fill(0);
    radius = 10;

    milxQtRenderWindow::reset();
}

milxQtHDRImage::~milxQtHDRImage()
{

    printDebug("HDR Image Destroyed.");
}

void milxQtHDRImage::createMenu(QMenu *menu)
{
    if(!menu)
        return;

    menu->clear();

    foreach(QAction *currAct, milxQtWindow::actionsToAdd)
    {
      menu->addAction(currAct);
    }
    foreach(QMenu *currMenu, milxQtWindow::menusToAdd)
    {
      menu->addMenu(currMenu);
    }
    contourPolyDataAct->setDisabled(!contourAct->isChecked());
    contourInitAct->setDisabled(!contourAct->isChecked());

    menu->addMenu(contourMenu);
    menu->addAction(backgroundAct);
    menu->addAction(axesAct);
    menu->addAction(lightingAct);
    menu->addAction(lineAct);
    menu->addAction(distanceAct);
    menu->addAction(biDirectionAct);
    menu->addAction(angleAct);
    menu->addAction(planeAct);
    menu->addAction(boxAct);
    menu->addAction(sphereAct);
    menu->addAction(humanAct);
    menu->addAction(textAct);

    ///Change View of Volume
    menu->addMenu(viewMenu);
    viewMenu->addAction(viewXY);
    viewMenu->addAction(viewZX);
    viewMenu->addAction(viewZY);
    viewMenu->addAction(saveViewAct);
    viewMenu->addAction(saveViewFileAct);
    viewMenu->addAction(loadViewAct);
    viewMenu->addAction(loadViewFileAct);
    enableActionBasedOnView();
    colourMapsMenu();

    menu->addSeparator();
    foreach(QAction *currAct, milxQtWindow::actionsToAppend)
    {
      menu->addAction(currAct);
    }
    foreach(QMenu *currMenu, milxQtWindow::menusToAppend)
    {
      menu->addMenu(currMenu);
    }

    menu->addAction(refreshAct);
    menu->addAction(resetAct);
}

void milxQtHDRImage::contextMenuEvent(QContextMenuEvent *currentEvent)
{
//    milxQtRenderWindow::contextMenuEvent(currentEvent);
}
