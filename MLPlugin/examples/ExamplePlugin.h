#ifndef EXAMPLEPLUGIN_H
#define EXAMPLEPLUGIN_H

#include <MLPlugin.h>
#include <QWidget>

class ExamplePlugin : public MLPlugin
{
public:
    ExamplePlugin(void *bases, void *plugin_path);

    void
    createWindow(QWidget *parent) override;
};

extern "C"
{
    #if defined(__linux)
    MLPlugin *
    create(void *bases, void *plugin_path)
    {
        return new ExamplePlugin(bases, plugin_path);
    }
    #elif defined(_WIN32)
    __declspec(dllexport) MLPlugin *
    create(void *bases, void *plugin_path)
    {
        return new ExamplePlugin(bases, plugin_path);
    }
    #endif
}

#endif // EXAMPLEPLUGIN_H
