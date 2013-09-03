#include "Settings.h"

#include <QSettings>
#include <QStringList>
#include "GraphicSystem.h"

#define APPNAME Rockete


QStringList Settings::getRecentFileList()
{
    return settings.value("File/Recents").value< QStringList >();
}

QString Settings::getProject()
{
    return settings.value("File/Project").value< QString >();
}

void Settings::setMostRecentFile(const QString &filePath)
{
    QStringList currentList;

    currentList = getRecentFileList();

    foreach(const QString &item, currentList) {
        if (item==filePath) {
            currentList.removeOne(item);
            break;
        }
    }

    currentList.push_front(filePath);

    while( currentList.count() > 30 ) {
        currentList.removeLast();
    }

    settings.setValue("File/Recents", currentList);
}

void Settings::setProject(const QString &filePath)
{
    settings.setValue("File/Project", filePath);
}

int Settings::getTabSize()
{
    // :TODO: Make a setting ;)
    return 4;
}

void Settings::setBackroundFileName(const QString &fileName)
{
    Rocket::Core::Vector2i dimensions;
    
    GraphicSystem::loadTexture( backgroundTextureHandle, dimensions, fileName.toAscii().data() );
    settings.setValue("File/BackgroundFileName", fileName);
}

QString Settings::getBackgroundFileName()
{
    return settings.value("File/BackgroundFileName").value< QString >();
}


Rocket::Core::TextureHandle Settings::getBackroundTextureHandle()
{
    return backgroundTextureHandle;
}

QSettings Settings::settings("FishingCactus", "Rockete");
Rocket::Core::TextureHandle Settings::backgroundTextureHandle = 0;
