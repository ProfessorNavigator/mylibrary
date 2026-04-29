/*
 * Copyright (C) 2026 Yury Bobylev <bobilev_yury@mail.ru>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include <PluginsWindow.h>
#include <QFileDialog>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QScreen>
#include <QScrollArea>
#include <QStyleOption>
#include <QVBoxLayout>
#include <StyledWindow.h>

PluginsWindow::PluginsWindow(QWidget *parent,
                             const std::shared_ptr<PluginManager> &plugins)
    : QWidget(parent)
{
  this->plugins = plugins;

  this->setWindowFlag(Qt::Window, true);
  this->setWindowTitle(tr("Plugins"));
  this->setAttribute(Qt::WA_DeleteOnClose);
  this->setWindowModality(Qt::WindowModal);

  this->setObjectName("Window");
}

void
PluginsWindow::createWindow()
{
  QVBoxLayout *v_box = new QVBoxLayout;
  this->setLayout(v_box);

  QWidget *scrl_widget = new QWidget;

  QVBoxLayout *scrl_box = new QVBoxLayout;
  scrl_widget->setLayout(scrl_box);

  connect(this, &PluginsWindow::signalRemovePlugin, scrl_box,
          [scrl_box](QWidget *w)
            {
              scrl_box->removeWidget(w);
              w->deleteLater();
            });

  connect(this, &PluginsWindow::signalAddPlugin, scrl_box,
          [scrl_box, this](const std::shared_ptr<Plugin> &pl)
            {
              for(int i = 0; i < scrl_box->count(); i++)
                {
                  QLayoutItem *item = scrl_box->itemAt(i);
                  if(item == nullptr)
                    {
                      scrl_box->insertWidget(i, formPluginWidget(pl));
                      break;
                    }
                  else
                    {
                      QWidget *w = item->widget();
                      if(w == nullptr)
                        {
                          scrl_box->insertWidget(i, formPluginWidget(pl));
                          break;
                        }
                    }
                }
            });

  std::vector<std::shared_ptr<Plugin>> pl_v = plugins->getPlugins();
  for(auto it = pl_v.begin(); it != pl_v.end(); it++)
    {
      scrl_box->addWidget(formPluginWidget(*it));
    }
  scrl_box->addStretch();

  QScrollArea *scrl = new QScrollArea;
  scrl->setObjectName("ScrollArea");
  scrl->setWidgetResizable(true);
  scrl_widget->setObjectName("ScrollAreaViewport");
  scrl->setWidget(scrl_widget);
  v_box->addWidget(scrl);

  QHBoxLayout *h_box = new QHBoxLayout;
  v_box->addLayout(h_box);

  QPushButton *add_plugin = new QPushButton;
  add_plugin->setText(tr("Add plugin"));
  QGraphicsDropShadowEffect *shadow
      = new QGraphicsDropShadowEffect(add_plugin);
  shadow->setBlurRadius(12);
  shadow->setOffset(0, 2);
  shadow->setColor(QColor(0, 0, 0, 120));
  add_plugin->setGraphicsEffect(shadow);
  add_plugin->setObjectName("ApplyButton");
  connect(add_plugin, &QPushButton::clicked, this,
          &PluginsWindow::pluginAddDialog);
  h_box->addWidget(add_plugin, 0, Qt::AlignCenter);

  QPushButton *close = new QPushButton;
  close->setText(tr("Close"));
  shadow = new QGraphicsDropShadowEffect(close);
  shadow->setBlurRadius(12);
  shadow->setOffset(0, 2);
  shadow->setColor(QColor(0, 0, 0, 120));
  close->setGraphicsEffect(shadow);
  close->setObjectName("CancelButton");
  connect(close, &QPushButton::clicked, this, &PluginsWindow::close);
  h_box->addWidget(close, 0, Qt::AlignCenter);

  QScreen *screen = this->parentWidget()->screen();
  QSize av_sz = screen->availableSize();
  av_sz.setHeight(av_sz.height() * 0.5);
  av_sz.setWidth(av_sz.width() * 0.5);
  this->resize(av_sz);
}

QWidget *
PluginsWindow::formPluginWidget(const std::shared_ptr<Plugin> &plugin)
{
  QFrame *frame = new QFrame;
  frame->setObjectName("Frame");
  frame->setFrameShape(QFrame::Box);

  QVBoxLayout *v_box = new QVBoxLayout;
  frame->setLayout(v_box);

  MLPlugin *pl = plugin->getPlugin();

  QLabel *lab = new QLabel;
  lab->setObjectName("Label");
  QFont font = lab->font();
  font.setBold(true);
  lab->setFont(font);
  lab->setText(pl->getPluginName());
  v_box->addWidget(lab, 0, Qt::AlignCenter);

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setWordWrap(true);
  lab->setAlignment(Qt::AlignCenter);
  lab->setText(pl->getPluginDescription());
  v_box->addWidget(lab);

  QHBoxLayout *h_box = new QHBoxLayout;
  v_box->addLayout(h_box);

  QPushButton *launch = new QPushButton;
  launch->setText(tr("Launch"));
  QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(launch);
  shadow->setBlurRadius(12);
  shadow->setOffset(0, 2);
  shadow->setColor(QColor(0, 0, 0, 120));
  launch->setGraphicsEffect(shadow);
  launch->setObjectName("ApplyButton");
  connect(launch, &QPushButton::clicked, this,
          [pl, this]
            {
              pl->createWindow(this);
            });
  h_box->addWidget(launch, 0, Qt::AlignCenter);

  QPushButton *disable = new QPushButton;
  disable->setText(tr("Disable"));
  shadow = new QGraphicsDropShadowEffect(disable);
  shadow->setBlurRadius(12);
  shadow->setOffset(0, 2);
  shadow->setColor(QColor(0, 0, 0, 120));
  disable->setGraphicsEffect(shadow);
  disable->setObjectName("ClearButton");
  connect(disable, &QPushButton::clicked, this,
          [plugin, frame, this]
            {
              removeConfirmationDialog(plugin, frame);
            });
  h_box->addWidget(disable, 0, Qt::AlignCenter);

  return frame;
}

void
PluginsWindow::removeConfirmationDialog(const std::shared_ptr<Plugin> &plugin,
                                        QWidget *plugin_widget)
{
  StyledWindow *window = new StyledWindow(this);
  window->setWindowModality(Qt::WindowModal);
  window->setObjectName("Window");

  QVBoxLayout *v_box = new QVBoxLayout;
  window->setLayout(v_box);

  QLabel *lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Are you sure?"));
  v_box->addWidget(lab, 0, Qt::AlignCenter);

  QHBoxLayout *h_box = new QHBoxLayout;
  v_box->addLayout(h_box);

  QPushButton *yes = new QPushButton;
  yes->setText(tr("Yes"));
  QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(yes);
  shadow->setBlurRadius(12);
  shadow->setOffset(0, 2);
  shadow->setColor(QColor(0, 0, 0, 120));
  yes->setGraphicsEffect(shadow);
  yes->setObjectName("ApplyButton");
  connect(yes, &QPushButton::clicked, this,
          [this, plugin, plugin_widget, window]
            {
              plugins->removePlugin(plugin);
              emit signalRemovePlugin(plugin_widget);
              window->close();
            });
  h_box->addWidget(yes, 0, Qt::AlignCenter);

  QPushButton *no = new QPushButton;
  no->setText(tr("No"));
  shadow = new QGraphicsDropShadowEffect(no);
  shadow->setBlurRadius(12);
  shadow->setOffset(0, 2);
  shadow->setColor(QColor(0, 0, 0, 120));
  no->setGraphicsEffect(shadow);
  no->setObjectName("CancelButton");
  connect(no, &QPushButton::clicked, window, &StyledWindow::close);
  h_box->addWidget(no, 0, Qt::AlignCenter);

  window->show();
}

void
PluginsWindow::pluginAddDialog()
{
  QFileDialog *fd = new QFileDialog(this);
  fd->setAttribute(Qt::WA_DeleteOnClose);
  fd->setWindowModality(Qt::WindowModal);

  fd->setAcceptMode(QFileDialog::AcceptOpen);
  fd->setDirectory(QDir::homePath());
  fd->setFileMode(QFileDialog::ExistingFile);

#ifdef __linux
  fd->setNameFilter("*.so");
#elif defined(_WIN32)
  fd->setNameFilter("*.dll");
#endif

  connect(fd, &QFileDialog::fileSelected, this,
          [this](const QString &file)
            {
              std::string str = file.toStdString();
              std::filesystem::path p = std::u8string(str.begin(), str.end());
              std::shared_ptr<Plugin> pl = plugins->addPlugin(p);
              if(pl)
                {
                  emit signalAddPlugin(pl);
                }
            });

  fd->show();
}

void
PluginsWindow::paintEvent(QPaintEvent *event)
{
  QStyleOption opt;
  opt.initFrom(this);
  QPainter p(this);
  style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
