#ifndef EXAMPLEPLUGIN_H
#define EXAMPLEPLUGIN_H

#include <MLPlugin.h>

class ExamplePlugin : public MLPlugin
{
public:
  ExamplePlugin(void *af_ptr);

  void
  createWindow(Gtk::Window *parent_window) override;
};

extern "C"
{
#ifdef __linux
  MLPlugin *
  create(void *af_ptr)
  {
    return new ExamplePlugin(af_ptr);
  }
#endif
#ifdef _WIN32
  __declspec(dllexport) MLPlugin *
  create(void *af_ptr)
  {
    return new ExamplePlugin(af_ptr);
  }
#endif
}
#endif // EXAMPLEPLUGIN_H
