#include "Rockete.h"
#include <QDebug>
#include <QDir>
#include <QtGui/QApplication>
#include "RocketSystem.h"
#include "ToolManager.h"
#include "EditionHelper.h"
#ifdef Q_WS_MAC
# include <CoreFoundation/CoreFoundation.h>
#endif

int main(int argc, char *argv[])
{
    int result;

    QApplication a( argc, argv );

#ifdef Q_WS_MAC
    CFURLRef appUrlRef = CFBundleCopyBundleURL(CFBundleGetMainBundle());
    CFStringRef macPath = CFURLCopyFileSystemPath(appUrlRef, kCFURLPOSIXPathStyle);
    const char *bndlPathPtr = CFStringGetCStringPtr(macPath, kCFStringEncodingUTF8);
    if (bndlPathPtr) {
        qDebug() << "Using mac bundle path for cwd: " << bndlPathPtr;
        QDir::setCurrent(bndlPathPtr);
    }
#endif

    qDebug() << "Working directory: " << QDir::currentPath();

    if(!RocketSystem::getInstance().initialize())
    {
        return -1;
    }

    Rockete w;
    w.show();
    result = a.exec();

    RocketSystem::getInstance().finalize();

    return result;
}
