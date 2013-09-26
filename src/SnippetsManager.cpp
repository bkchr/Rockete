#include "SnippetsManager.h"

#include <QMimeData>
#include <QDirIterator>
#include <QXmlSimpleReader>
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QDebug>
#include "Rockete.h"
#include "ProjectManager.h"

// Public:

SnippetsManager::SnippetsManager(QWidget *parent)
    : QListWidget(parent),
      QXmlDefaultHandler()
{
}

void SnippetsManager::initialize()
{
    if(!QFile::exists(ProjectManager::getInstance().getSnippetsFolderPath()))
    {
        return;
    }

    clear();

    QDirIterator directory_walker(ProjectManager::getInstance().getSnippetsFolderPath(), QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);
    QStringList snippetsFileList;

    while(directory_walker.hasNext())
    {
        directory_walker.next();
        if(!directory_walker.fileInfo().isDir() && !directory_walker.fileInfo().isHidden() && directory_walker.fileInfo().suffix()=="snippet")
        {
            snippetsFileList << directory_walker.fileInfo().filePath();
        }
    }


    QXmlSimpleReader xmlReader;
    QXmlInputSource *source;
    bool ok;

    xmlReader.setContentHandler(this);
    xmlReader.setErrorHandler(this);

    foreach(QString fileName, snippetsFileList)
    {
        QFile file(fileName);
        currentFilePath = fileName;
        source = new QXmlInputSource(&file);
        ok = xmlReader.parse(source);
        delete(source);
        if(!ok)
        {
            qDebug() << fileName << " could not be parsed as snippet\n";
        }
    }
}

bool SnippetsManager::startDocument()
{
    return true;
}

bool SnippetsManager::startElement(const QString &/*namespaceURI*/, const QString &localName, const QString &/*qName*/, const QXmlAttributes &atts)
{
    currentTag=localName;
    if(currentTag == "Code")
    {
        currentCodeSnippet->Language= atts.value("Language").toLower();
    }
    else if(currentTag == "CodeSnippet")
    {
        currentCodeSnippet = new CodeSnippet;
        currentCodeSnippet->FilePath = currentFilePath;
    }
    return true;
}

bool SnippetsManager::endElement(const QString &/*namespaceURI*/, const QString &localName, const QString &/*qName*/)
{
    if(localName == "CodeSnippet")
    {
        QListWidgetItem *item = new QListWidgetItem();

        item->setText(currentCodeSnippet->Title);
        item->setToolTip(currentCodeSnippet->ToolTip);
        item->setData(Qt::UserRole, qVariantFromValue<void*>(currentCodeSnippet));
        addItem(item);
    }
    return true;
}

bool SnippetsManager::characters(const QString &ch)
{
    QString usable_string = ch.trimmed();
    usable_string = usable_string.remove("\t");
    if(usable_string.isEmpty())
        return true;


    if(currentTag=="Title")
    {
        currentCodeSnippet->Title = usable_string;
    }
    else if(currentTag=="ToolTip")
    {
        currentCodeSnippet->ToolTip = usable_string;
    }
    else if(currentTag=="Code")
    {
        currentCodeSnippet->Code = usable_string;
    }

    return true;
}

bool SnippetsManager::endDocument()
{
    return true;
}

QString SnippetsManager::addSnippet()
{
    QDir dir = ProjectManager::getInstance().getSnippetsFolderPath();

    if(!dir.exists())
    {
        if(!dir.mkpath(ProjectManager::getInstance().getSnippetsFolderPath()))
        {
            dir.mkdir(ProjectManager::getInstance().getSnippetsFolderPath());
        }
    }

    Q_ASSERT(dir.exists());

    bool ok;
    QString text = QInputDialog::getText(this, "New Snippet",
        "File name", QLineEdit::Normal,
        "my snippet", &ok);

    if (!ok || text.isEmpty())
        return "";

    if (!text.contains(".snippet"))
    {
        text += ".snippet";
    }

    QFile file(ProjectManager::getInstance().getSnippetsFolderPath() + text);

    if(file.exists())
    {
        Rockete::getInstance().openFile(text);
        return "";
    }

    if(!file.open(QIODevice::WriteOnly|QIODevice::Text))
        return "";

    QString content;

    content = "\
<CodeSnippet>\n \
    <Title>\n\
        new snippet\n\
    </Title>\n\
    <ToolTip>\n\
        new tooltip\n\
    </ToolTip>\n\
    <Code Language=\"\">\n\
        <![CDATA[\n\
        \n\
        ]]>\n\
    </Code>\n\
</CodeSnippet>";

    file.write(content.toUtf8().data());
    file.close();

    Rockete::getInstance().openFile(text);

    initialize();
    return text;
}

QString SnippetsManager::removeSnippet()
{
    if(QMessageBox::question(this, "Please confirm.", "Are you really really sure?", QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
    {
        QListWidgetItem *item = takeItem(currentRow());
        QString file_name = ((CodeSnippet *)item->data(Qt::UserRole).value<void*>())->FilePath;
        delete((CodeSnippet *)item->data(Qt::UserRole).value<void*>());
        delete(item);
        return file_name;
    }

    return "";
}

void SnippetsManager::filterSnippetsForLanguage(QString language)
{
    language = language.toLower();

    if(language != "rml" && language != "rcss" && language != "lua")
        language = "none";

    for(int i=0;i<count();i++)
    {
        if(((CodeSnippet *)item(i)->data(Qt::UserRole).value<void*>())->Language == language)
        {
            item(i)->setHidden(false);
        }
        else
        {
            
            item(i)->setHidden(language!="none");
        }
    }
}

// public slots:

// protected:

QStringList SnippetsManager::mimeTypes() const
{
    QStringList types;
    types << "text/plain";
    return types;
}

QMimeData *SnippetsManager::mimeData(const QList<QListWidgetItem *> items) const
{
    QString text = "";
    QMimeData *mimeData = new QMimeData();

    foreach(QListWidgetItem *item, items){
        text += ((CodeSnippet *)item->data(Qt::UserRole).value<void*>())->Code;
    }

    mimeData->setText(text);
    return mimeData;
}

// private slots

// private
