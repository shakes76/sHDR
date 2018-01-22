#ifndef MILXQTHDRIMAGE_H
#define MILXQTHDRIMAGE_H

//milxQt
#include "milxQtImage.h"

class MILXQT_EXPORT milxQtHDRImage : public milxQtImage
{
    Q_OBJECT

public:
    /*!
        \fn milxQtHDRImage::milxQtHDRImage(QWidget *parent = 0)
        \brief The standard constructor
    */
    milxQtHDRImage(QWidget *theParent = 0);
    /*!
        \fn milxQtHDRImage::~milxQtMain()
        \brief The standard destructor
    */
    virtual ~milxQtHDRImage();

public slots:
    /*!
        \fn milxQtHDRImage::createMenu(QMenu *menu)
        \brief Create the menu for the data in this object. Used for context menu and file menus.
    */
    virtual void createMenu(QMenu *menu);


protected:
    coordinate center;
    double radius;

    /*!
        \fn milxQtHDRImage::contextMenuEvent(QContextMenuEvent *event)
        \brief The context menu setup member
    */
    void contextMenuEvent(QContextMenuEvent *event);

};

#endif // MILXQTHDRIMAGE_H
