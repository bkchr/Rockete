#include "ProjectManager.h"

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QXmlStreamWriter>
#include <QMessageBox>
#include "LocalizationManagerInterface.h"

#ifdef Q_WS_MAC
# include <CoreFoundation/CoreFoundation.h>
#endif

QString XML_utf8( QString t )
{
  //the following checks are necessary before exporting
  //strings to XML. see http://hdf.ncsa.uiuc.edu/HDF5/XML/xml_escape_chars.html for details
  QString text = t;

  text.replace("\"","&quot;");
  text.replace("'", "&apos;");
  text.replace("<", "&lt;");
  text.replace(">", "&gt;");
  text.replace("\n", "&#10;");
  text.replace("\r", "&#13;");
  text.replace("&", "&amp;");   /*  qt4 toUtf8 dont replace && */
  return text;
}

ProjectManager::ProjectManager()
{
}

ProjectManager::~ProjectManager()
{
}

void ProjectManager::Initialize(const QString &filename)
{
    qDebug() << "Initialize project manager at: " << filename;

    QFileInfo file_info(filename);
    QFile file(filename);

    QDomDocument domDocument;

    fontPaths.clear();
    texturePaths.clear();
    interfacePaths.clear();

    if (QFile::exists(filename)) {
        if (!file.open(QIODevice::ReadOnly))
            return;
        QString errmsg; int errcol, errrow;
        if (!domDocument.setContent(file.readAll(), &errmsg, &errrow, &errcol)) {
            file.close();
            qDebug() << "Error parsing project: " << errmsg << ": line" << errrow << ",column" << errcol;
            return;
        }
        file.close();
    }

    projectName = file_info.baseName();

#ifdef Q_WS_MAC
    CFURLRef appUrlRef = CFBundleCopyBundleURL(CFBundleGetMainBundle());
    CFStringRef macPath = CFURLCopyFileSystemPath(appUrlRef, kCFURLPOSIXPathStyle);
    const char *bndlPathPtr = CFStringGetCStringPtr(macPath, kCFStringEncodingUTF8)?:".";
    // char path[PATH_MAX]; CFStringGetFileSystemRepresentation(s, path, sizeof(path)); // TODO: check if this is needed.
    if (CFStringGetCStringPtr(macPath, kCFStringEncodingUTF8))
        qDebug() << "Using mac bundle path: " << bndlPathPtr;
#endif

    QDomNodeList node_list = domDocument.elementsByTagName("Font");

    if (node_list.count()) for(int i = 0; i < node_list.count(); i++)
    {
        QDomNode node = node_list.at(i);
        if(node.firstChild().isText())
        {
            const QString& nodeText = node.firstChild().toText().data();
            if(!(nodeText.contains(":") || nodeText.startsWith("/")))
            {
	            fontPaths << file_info.path() + "/" + nodeText;
            } 
            else
            {
	            fontPaths << nodeText;
            }
            if(!fontPaths.last().endsWith('/'))
            {
                fontPaths.last().append("/");
            }
        }
    } else {
#ifdef Q_WS_MAC
        fontPaths << (QString(bndlPathPtr) + "/fonts/");
#endif
    }

    node_list = domDocument.elementsByTagName("Texture");

    if (node_list.count()) for(int i = 0; i < node_list.count(); i++)
    {
        QDomNode node = node_list.at(i);
        if(node.firstChild().isText())
        {
            const QString& nodeText = node.firstChild().toText().data();
            if(!(nodeText.contains(":") || nodeText.startsWith("/")))
            {
                texturePaths << file_info.path() + "/" + nodeText;
            } 
            else
            {
                texturePaths << nodeText;
            }
            if(!texturePaths.last().endsWith('/'))
            {
                texturePaths.last().append("/");
            }
        }
    } else {
#ifdef Q_WS_MAC
        texturePaths << (QString(bndlPathPtr) + "/textures/");
#endif
    }

    node_list = domDocument.elementsByTagName("Interface");

    if (node_list.count()) for(int i = 0; i < node_list.count(); i++)
    {
        QDomNode node = node_list.at(i);
        if(node.firstChild().isText())
        {
            const QString& nodeText = node.firstChild().toText().data();
            if(!(nodeText.contains(":") || nodeText.startsWith("/")))
            {
                interfacePaths << file_info.path() + "/" + nodeText;
            } 
            else
            {
                interfacePaths << nodeText;
            }
            if(!interfacePaths.last().endsWith('/'))
            {
                interfacePaths.last().append("/");
            }
        }
    } else {
#ifdef Q_WS_MAC
        interfacePaths << (QString(bndlPathPtr) + "/interface/");
#endif
    }

    node_list = domDocument.elementsByTagName("WordList");

    if(!node_list.isEmpty())
    {
        QDomNode node = node_list.at(0);
        if(node.firstChild().isText())
        {
            const QString& nodeText = node.firstChild().toText().data();
            if(!(nodeText.contains(":") || nodeText.startsWith("/")))
            {
                wordListsPath = file_info.path() + "/" + nodeText;
            } 
            else
            {
                wordListsPath = nodeText;
            }
            if(!wordListsPath.endsWith('/'))
            {
                wordListsPath.append("/");
            }
        }
    }
    else
    {
        wordListsPath = "word_lists/";
    }

    node_list = domDocument.elementsByTagName("LocalizationFile");

    if(!node_list.isEmpty())
    {
        QDomNode node = node_list.at(0);
        if(node.firstChild().isText())
        {
            if(LocalizationManagerInterface::hasInstance())
            {
                LocalizationManagerInterface::getInstance().initialize(node.firstChild().toText().data());
            }
        }
    }

    node_list = domDocument.elementsByTagName("LocalizationOpeningTag");

    if(!node_list.isEmpty())
    {
        QDomNode node = node_list.at(0);
        if(node.firstChild().isText())
        {
            localizationOpeningTag = node.firstChild().toText().data();
        }
    }

    node_list = domDocument.elementsByTagName("LocalizationClosingTag");

    if(!node_list.isEmpty())
    {
        QDomNode node = node_list.at(0);
        if(node.firstChild().isText())
        {
            localizationClosingTag = node.firstChild().toText().data();
        }
    }

    node_list = domDocument.elementsByTagName("SnippetsFolder");

    if(!node_list.isEmpty())
    {
        QDomNode node = node_list.at(0);
        if(node.firstChild().isText())
        {
            const QString& nodeText = node.firstChild().toText().data();
            if(!(nodeText.contains(":") || nodeText.startsWith("/")))
            {
                snippetsFolderPath = file_info.path() + "/" + nodeText;
            } 
            else
            {
                snippetsFolderPath = nodeText;
            }
            if(!snippetsFolderPath.endsWith('/'))
            {
                snippetsFolderPath.append("/");
            }
        }
    }
    else
    {
        snippetsFolderPath = "snippets/";
    }

#ifdef Q_WS_MAC
    CFRelease(appUrlRef);
    CFRelease(macPath);
#endif
}

static void _saveQStringList(QXmlStreamWriter &xmlWriter, const QString &sec, const QStringList &list)
{
    foreach(const QString &str, list) {
        xmlWriter.writeStartElement(sec);
        xmlWriter.writeCharacters(str);
        xmlWriter.writeEndElement();
    }
}

void ProjectManager::Serialize(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        /* show wrror message if not able to open file */
        QMessageBox::warning(0, "Read only", "The file is in read only mode");
    }
    else
    {
        qDebug() << "Serialize project at: " << filename;

        QXmlStreamWriter xmlWriter(&file);
        xmlWriter.setAutoFormatting(true);

        xmlWriter.writeStartDocument();
        xmlWriter.writeStartElement("Project");

        _saveQStringList(xmlWriter, "Font", fontPaths);
        _saveQStringList(xmlWriter, "Texture", texturePaths);
        _saveQStringList(xmlWriter, "Interface", interfacePaths);
        xmlWriter.writeStartElement("SnippetsFolder");
        xmlWriter.writeCharacters(snippetsFolderPath);
        xmlWriter.writeEndElement();
        xmlWriter.writeStartElement("WordList");
        xmlWriter.writeCharacters(wordListsPath);
        xmlWriter.writeEndElement();
        xmlWriter.writeStartElement("LocalizationOpeningTag");
        xmlWriter.writeCharacters(localizationOpeningTag);
        xmlWriter.writeEndElement();
        xmlWriter.writeStartElement("LocalizationClosingTag");
        xmlWriter.writeCharacters(localizationClosingTag);
        xmlWriter.writeEndElement();

        xmlWriter.writeEndElement();
        xmlWriter.writeEndDocument();
    }
}
