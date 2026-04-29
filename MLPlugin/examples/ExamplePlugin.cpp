#include <Examplelugin.h>
#include <QApplication>
#include <QObject>
#include <QTranslator>
#include <QVBoxLayout>
#include <QLabel>

ExamplePlugin::ExamplePlugin(void *bases, void *plugin_path)
: MLPlugin(bases, plugin_path)
{
    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for(const QString &locale : uiLanguages)
    {
        const QString baseName = "ExamplePlugin_" + QLocale(locale).name();
        if(translator.load(":/i18n/" + baseName))
        {
            qApp->installTranslator(&translator);
            break;
        }
    }

    plugin_name = "ExamplePlugin";
    plugin_description = QObject::tr("Example plugin");
}

void
ExamplePlugin::createWindow(QWidget *parent)
{
    QWidget *window = new QWidget(parent);
    window->setWindowFlag(Qt::Window);
    window->setAttribute(Qt::WA_DeleteOnClose);
    window->setWindowModality(Qt::WindowModal);
    window->setWindowTitle(QObject::tr("Example plugin"));

    QVBoxLayout *main_box = new QVBoxLayout;
    window->setLayout(main_box);

    QLabel *lab = new QLabel;
    lab->setObjectName("Label");
    QFont font = lab->font();
    font.setBold(true);
    lab->setFont(font);
    lab->setText(QObject::tr("Example plugin"));
    main_box->addWidget(lab, 0, Qt::AlignCenter);

    window->show();
}
