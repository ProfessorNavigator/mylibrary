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

#include <QColorDialog>
#include <QDir>
#include <QFileDialog>
#include <QFontDialog>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QImageReader>
#include <QLabel>
#include <QPainter>
#include <QScreen>
#include <QScrollArea>
#include <QStyleOption>
#include <QVBoxLayout>
#include <SettingsWindow.h>
#include <StyledWindow.h>

SettingsWindow::SettingsWindow(
    QWidget *parent, const std::shared_ptr<SettingsManager> &settings)
    : QWidget(parent)
{
  this->settings = settings;
  settings_copy = std::make_shared<SettingsManager>(*settings);

  this->setWindowFlag(Qt::Window);
  this->setAttribute(Qt::WA_DeleteOnClose);
  this->setWindowTitle(tr("Settings"));
  this->setWindowModality(Qt::WindowModal);

  this->setObjectName("Window");
}

void
SettingsWindow::createWindow()
{
  QVBoxLayout *v_box = new QVBoxLayout;
  this->setLayout(v_box);

  QWidget *w = new QWidget;
  w->setObjectName("ScrollAreaViewport");
  QVBoxLayout *w_layout = new QVBoxLayout;
  w->setLayout(w_layout);

  QScrollArea *scrl = new QScrollArea;
  scrl->setObjectName("ScrollArea");
  scrl->setWidgetResizable(true);
  v_box->addWidget(scrl);

  QHBoxLayout *h_box = new QHBoxLayout;
  w_layout->addLayout(h_box);

  QLabel *lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Font:"));
  h_box->addWidget(lab);

  QPushButton *font_button = new QPushButton;
  font_button->setText(settings->getFont().family());
  QGraphicsDropShadowEffect *shadow
      = new QGraphicsDropShadowEffect(font_button);
  shadow->setBlurRadius(12);
  shadow->setOffset(0, 2);
  shadow->setColor(QColor(0, 0, 0, 120));
  font_button->setGraphicsEffect(shadow);
  font_button->setObjectName("ApplyButton");
  connect(font_button, &QPushButton::clicked, this,
          [this, font_button]
            {
              fontSelectionDialog(font_button);
            });
  h_box->addWidget(font_button);

  h_box->addStretch();

  w_layout->addWidget(windowsWidget());

  h_box = new QHBoxLayout;
  w_layout->addLayout(h_box);

  h_box->addStretch();
  h_box->addWidget(comboboxWidget());
  h_box->addWidget(labelWidget());
  h_box->addWidget(lineeditWidget());
  h_box->addWidget(sliderWidget());
  h_box->addStretch();

  h_box = new QHBoxLayout;
  w_layout->addLayout(h_box);

  h_box->addStretch();
  h_box->addWidget(clearbuttonWidget());
  h_box->addWidget(applybuttonWidget());
  h_box->addWidget(cancelbuttonWidget());
  h_box->addWidget(progressbarWidget());
  h_box->addStretch();

  h_box = new QHBoxLayout;
  w_layout->addLayout(h_box);

  h_box->addStretch();
  h_box->addWidget(tableWidget());
  h_box->addWidget(texteditWidget());

  QVBoxLayout *l_v_box = new QVBoxLayout;
  h_box->addLayout(l_v_box);

  l_v_box->addWidget(menuWidget());
  l_v_box->addWidget(checkboxWidget());
  l_v_box->addWidget(frameWidget());

  l_v_box->addStretch();

  h_box->addStretch();

  w_layout->addWidget(scrollareaWidget(), 0, Qt::AlignCenter);

  scrl->setWidget(w);

  h_box = new QHBoxLayout;
  v_box->addLayout(h_box);

  QPushButton *apply = new QPushButton;
  apply->setText(tr("Apply"));
  shadow = new QGraphicsDropShadowEffect(apply);
  shadow->setBlurRadius(12);
  shadow->setOffset(0, 2);
  shadow->setColor(QColor(0, 0, 0, 120));
  apply->setGraphicsEffect(shadow);
  apply->setObjectName("ApplyButton");
  connect(apply, &QPushButton::clicked, this,
          [this]
            {
              *settings = *settings_copy;
              settings->applySettings();
            });
  h_box->addWidget(apply, 0, Qt::AlignCenter);

  QPushButton *apply_close = new QPushButton;
  apply_close->setText(tr("Apply and close"));
  shadow = new QGraphicsDropShadowEffect(apply_close);
  shadow->setBlurRadius(12);
  shadow->setOffset(0, 2);
  shadow->setColor(QColor(0, 0, 0, 120));
  apply_close->setGraphicsEffect(shadow);
  apply_close->setObjectName("ApplyButton");
  connect(apply_close, &QPushButton::clicked, this,
          [this]
            {
              *settings = *settings_copy;
              settings->applySettings();
              this->close();
            });
  h_box->addWidget(apply_close, 0, Qt::AlignCenter);

  QPushButton *reset_to_default = new QPushButton;
  reset_to_default->setText(tr("Reset to default"));
  shadow = new QGraphicsDropShadowEffect(reset_to_default);
  shadow->setBlurRadius(12);
  shadow->setOffset(0, 2);
  shadow->setColor(QColor(0, 0, 0, 120));
  reset_to_default->setGraphicsEffect(shadow);
  reset_to_default->setObjectName("ClearButton");
  connect(reset_to_default, &QPushButton::clicked, this,
          [this]
            {
              resetToDafaultDialog(false);
            });
  h_box->addWidget(reset_to_default, 0, Qt::AlignCenter);

  QPushButton *reset_to_system_default = new QPushButton;
  reset_to_system_default->setText(tr("Reset to system default"));
  shadow = new QGraphicsDropShadowEffect(reset_to_system_default);
  shadow->setBlurRadius(12);
  shadow->setOffset(0, 2);
  shadow->setColor(QColor(0, 0, 0, 120));
  reset_to_system_default->setGraphicsEffect(shadow);
  reset_to_system_default->setObjectName("ClearButton");
  connect(reset_to_system_default, &QPushButton::clicked, this,
          [this]
            {
              resetToDafaultDialog(true);
            });
  h_box->addWidget(reset_to_system_default, 0, Qt::AlignCenter);

  QPushButton *cancel = new QPushButton;
  cancel->setText(tr("Close"));
  shadow = new QGraphicsDropShadowEffect(cancel);
  shadow->setBlurRadius(12);
  shadow->setOffset(0, 2);
  shadow->setColor(QColor(0, 0, 0, 120));
  cancel->setGraphicsEffect(shadow);
  cancel->setObjectName("CancelButton");
  connect(cancel, &QPushButton::clicked, this, &SettingsWindow::close);
  h_box->addWidget(cancel, 0, Qt::AlignCenter);

  QScreen *screen = this->parentWidget()->screen();
  QSize av_sz = screen->availableSize();
  av_sz.setHeight(this->sizeHint().height());
  av_sz.setWidth(av_sz.width() * 0.5);
  this->resize(av_sz);
}

QWidget *
SettingsWindow::windowsWidget()
{
  QFrame *frame = new QFrame;
  frame->setObjectName("Frame");
  frame->setFrameShape(QFrame::Box);

  QGridLayout *grid = new QGridLayout;
  grid->setColumnStretch(1, 1);
  frame->setLayout(grid);

  int row = 0;
  QLabel *lab = new QLabel;
  lab->setObjectName("Label");
  QFont font = lab->font();
  font.setBold(true);
  lab->setFont(font);
  lab->setText(tr("Windows style"));
  grid->addWidget(lab, row, 0, 1, 2, Qt::AlignCenter);
  row++;

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Background color:"));
  grid->addWidget(lab, row, 0, Qt::AlignLeft | Qt::AlignVCenter);

  ColorButton *cb = new ColorButton;
  grid->addWidget(cb, row, 1, Qt::AlignVCenter | Qt::AlignLeft);
  row++;

  std::string id = "MainWindow";
  std::string attr = "background-color";
  QString str = settings->getStyleAttributeValue(id, attr);
  cb->setBakcroundColor(str);

  connect(cb, &ColorButton::clicked, this,
          [this, attr, id, cb]
            {
              colorDialog(cb,
                          settings->stringToColor(
                              cb->getBackGroundColor().toStdString()),
                          id, attr);
            });

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Menu bar background color:"));
  grid->addWidget(lab, row, 0, Qt::AlignLeft | Qt::AlignVCenter);

  cb = new ColorButton;
  grid->addWidget(cb, row, 1, Qt::AlignVCenter | Qt::AlignLeft);
  row++;

  id = "MenuBar";
  attr = "background-color";
  str = settings->getStyleAttributeValue(id, attr);
  cb->setBakcroundColor(str);

  connect(cb, &ColorButton::clicked, this,
          [this, attr, id, cb]
            {
              colorDialog(cb,
                          settings->stringToColor(
                              cb->getBackGroundColor().toStdString()),
                          id, attr);
            });

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Menu bar font color:"));
  grid->addWidget(lab, row, 0, Qt::AlignLeft | Qt::AlignVCenter);

  cb = new ColorButton;
  grid->addWidget(cb, row, 1, Qt::AlignVCenter | Qt::AlignLeft);
  row++;

  id = "MenuBar::item";
  attr = "color";
  str = settings->getStyleAttributeValue(id, attr);
  cb->setBakcroundColor(str);

  connect(cb, &ColorButton::clicked, this,
          [this, attr, id, cb]
            {
              colorDialog(cb,
                          settings->stringToColor(
                              cb->getBackGroundColor().toStdString()),
                          id, attr);
            });

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Background image path:"));
  grid->addWidget(lab, row, 0, 1, 2, Qt::AlignLeft);
  row++;

  QLineEdit *image_path = new QLineEdit;
  image_path->setObjectName("LineEdit");
  id = "MainWindow";
  attr = "border-image";
  str = settings->getStyleAttributeValue(id, attr);
  QString find_str("url");
  qsizetype n = str.indexOf(find_str);
  if(n >= 0)
    {
      str.erase(str.begin(), str.begin() + n + find_str.size());
    }
  find_str = "(";
  n = str.indexOf(find_str);
  if(n >= 0)
    {
      str.erase(str.begin(), str.begin() + n + find_str.size());
    }
  find_str = ")";
  n = str.indexOf(find_str);
  if(n >= 0)
    {
      str.erase(str.begin() + n, str.end());
    }
  image_path->setText(str);
  connect(image_path, &QLineEdit::textChanged, this,
          [this, id, attr](const QString &pth)
            {
              std::string str = pth.toStdString();
              if(str.empty())
                {
                  settings_copy->removeStyleAttribute(id, attr);
                  settings_copy->removeStyleAttribute("Window",
                                                      "background-image");
                  return void();
                }
              std::filesystem::path p = std::u8string(str.begin(), str.end());
              if(!std::filesystem::exists(p))
                {
                  return void();
                }
              if(std::filesystem::is_directory(p))
                {
                  return void();
                }
              std::string value = "url(" + str + ")";
              settings_copy->addStyleAttribute("Window", "background-image",
                                               value);
              value += " 0 0 0 0 stretch stretch";
              settings_copy->addStyleAttribute(id, attr, value);
            });
  grid->addWidget(image_path, row, 0, 1, 2);
  row++;

  QPushButton *open = new QPushButton;
  QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(open);
  shadow->setBlurRadius(12);
  shadow->setOffset(0, 2);
  shadow->setColor(QColor(0, 0, 0, 120));
  open->setGraphicsEffect(shadow);
  open->setObjectName("ApplyButton");
  open->setText(tr("Open"));
  connect(open, &QPushButton::clicked, this,
          [this, image_path]
            {
              openFileDialog(image_path);
            });
  grid->addWidget(open, row, 1, Qt::AlignVCenter | Qt::AlignRight);
  row++;

  return frame;
}

QWidget *
SettingsWindow::comboboxWidget()
{
  QFrame *frame = new QFrame;
  frame->setObjectName("Frame");
  frame->setFrameShape(QFrame::Box);

  QGridLayout *grid = new QGridLayout;
  frame->setLayout(grid);

  int row = 0;
  QLabel *lab = new QLabel;
  lab->setObjectName("Label");
  QFont font = lab->font();
  font.setBold(true);
  lab->setFont(font);
  lab->setText(tr("Combo boxes style"));
  grid->addWidget(lab, row, 0, 1, 2, Qt::AlignCenter);
  row++;

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Background color:"));
  grid->addWidget(lab, row, 0, Qt::AlignLeft | Qt::AlignVCenter);

  ColorButton *cb = new ColorButton;
  grid->addWidget(cb, row, 1, Qt::AlignVCenter | Qt::AlignLeft);
  row++;

  std::string id = "ComboBox";
  std::string attr = "background-color";
  QString str = settings->getStyleAttributeValue(id, attr);
  cb->setBakcroundColor(str);

  connect(cb, &ColorButton::clicked, this,
          [this, attr, id, cb]
            {
              colorDialog(cb,
                          settings->stringToColor(
                              cb->getBackGroundColor().toStdString()),
                          id, attr);
            });

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Font color:"));
  grid->addWidget(lab, row, 0, Qt::AlignLeft | Qt::AlignVCenter);

  cb = new ColorButton;
  grid->addWidget(cb, row, 1, Qt::AlignVCenter | Qt::AlignLeft);
  row++;

  attr = "color";
  str = settings->getStyleAttributeValue(id, attr);
  cb->setBakcroundColor(str);

  connect(cb, &ColorButton::clicked, this,
          [this, attr, id, cb]
            {
              colorDialog(cb,
                          settings->stringToColor(
                              cb->getBackGroundColor().toStdString()),
                          id, attr);
            });

  return frame;
}

QWidget *
SettingsWindow::labelWidget()
{
  QFrame *frame = new QFrame;
  frame->setObjectName("Frame");
  frame->setFrameShape(QFrame::Box);

  QGridLayout *grid = new QGridLayout;
  frame->setLayout(grid);

  int row = 0;
  QLabel *lab = new QLabel;
  lab->setObjectName("Label");
  QFont font = lab->font();
  font.setBold(true);
  lab->setFont(font);
  lab->setText(tr("Labels style"));
  grid->addWidget(lab, row, 0, 1, 2, Qt::AlignCenter);
  row++;

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Background color:"));
  grid->addWidget(lab, row, 0, Qt::AlignLeft | Qt::AlignVCenter);

  ColorButton *cb = new ColorButton;
  grid->addWidget(cb, row, 1, Qt::AlignVCenter | Qt::AlignLeft);
  row++;

  std::string id = "Label";
  std::string attr = "background-color";
  QString str = settings->getStyleAttributeValue(id, attr);
  cb->setBakcroundColor(str);

  connect(cb, &ColorButton::clicked, this,
          [this, attr, id, cb]
            {
              colorDialog(cb,
                          settings->stringToColor(
                              cb->getBackGroundColor().toStdString()),
                          id, attr);
            });

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Font color:"));
  grid->addWidget(lab, row, 0, Qt::AlignLeft | Qt::AlignVCenter);

  cb = new ColorButton;
  grid->addWidget(cb, row, 1, Qt::AlignVCenter | Qt::AlignLeft);
  row++;

  attr = "color";
  str = settings->getStyleAttributeValue(id, attr);
  cb->setBakcroundColor(str);

  connect(cb, &ColorButton::clicked, this,
          [this, attr, id, cb]
            {
              colorDialog(cb,
                          settings->stringToColor(
                              cb->getBackGroundColor().toStdString()),
                          id, attr);
            });

  return frame;
}

QWidget *
SettingsWindow::lineeditWidget()
{
  QFrame *frame = new QFrame;
  frame->setObjectName("Frame");
  frame->setFrameShape(QFrame::Box);

  QGridLayout *grid = new QGridLayout;
  frame->setLayout(grid);

  int row = 0;
  QLabel *lab = new QLabel;
  lab->setObjectName("Label");
  QFont font = lab->font();
  font.setBold(true);
  lab->setFont(font);
  lab->setText(tr("Input lines style"));
  grid->addWidget(lab, row, 0, 1, 2, Qt::AlignCenter);
  row++;

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Background color:"));
  grid->addWidget(lab, row, 0, Qt::AlignLeft | Qt::AlignVCenter);

  ColorButton *cb = new ColorButton;
  grid->addWidget(cb, row, 1, Qt::AlignVCenter | Qt::AlignLeft);
  row++;

  std::string id = "LineEdit";
  std::string attr = "background-color";
  QString str = settings->getStyleAttributeValue(id, attr);
  cb->setBakcroundColor(str);

  connect(cb, &ColorButton::clicked, this,
          [this, attr, id, cb]
            {
              colorDialog(cb,
                          settings->stringToColor(
                              cb->getBackGroundColor().toStdString()),
                          id, attr);
            });

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Font color:"));
  grid->addWidget(lab, row, 0, Qt::AlignLeft | Qt::AlignVCenter);

  cb = new ColorButton;
  grid->addWidget(cb, row, 1, Qt::AlignVCenter | Qt::AlignLeft);
  row++;

  attr = "color";
  str = settings->getStyleAttributeValue(id, attr);
  cb->setBakcroundColor(str);

  connect(cb, &ColorButton::clicked, this,
          [this, attr, id, cb]
            {
              colorDialog(cb,
                          settings->stringToColor(
                              cb->getBackGroundColor().toStdString()),
                          id, attr);
            });

  return frame;
}

QWidget *
SettingsWindow::clearbuttonWidget()
{
  QFrame *frame = new QFrame;
  frame->setObjectName("Frame");
  frame->setFrameShape(QFrame::Box);

  QGridLayout *grid = new QGridLayout;
  frame->setLayout(grid);

  int row = 0;
  QLabel *lab = new QLabel;
  lab->setObjectName("Label");
  QFont font = lab->font();
  font.setBold(true);
  lab->setFont(font);
  lab->setText(tr("\"Clear\" buttons style"));
  grid->addWidget(lab, row, 0, 1, 2, Qt::AlignCenter);
  row++;

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Background color:"));
  grid->addWidget(lab, row, 0, Qt::AlignLeft | Qt::AlignVCenter);

  ColorButton *cb = new ColorButton;
  grid->addWidget(cb, row, 1, Qt::AlignVCenter | Qt::AlignLeft);
  row++;

  std::string id = "ClearButton";
  std::string attr = "background-color";
  QString str = settings->getStyleAttributeValue(id, attr);
  cb->setBakcroundColor(str);

  connect(cb, &ColorButton::clicked, this,
          [this, attr, id, cb]
            {
              colorDialog(cb,
                          settings->stringToColor(
                              cb->getBackGroundColor().toStdString()),
                          id, attr);
            });

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Font color:"));
  grid->addWidget(lab, row, 0, Qt::AlignLeft | Qt::AlignVCenter);

  cb = new ColorButton;
  grid->addWidget(cb, row, 1, Qt::AlignVCenter | Qt::AlignLeft);
  row++;

  attr = "color";
  str = settings->getStyleAttributeValue(id, attr);
  cb->setBakcroundColor(str);

  connect(cb, &ColorButton::clicked, this,
          [this, attr, id, cb]
            {
              colorDialog(cb,
                          settings->stringToColor(
                              cb->getBackGroundColor().toStdString()),
                          id, attr);
            });

  return frame;
}

QWidget *
SettingsWindow::sliderWidget()
{
  QFrame *frame = new QFrame;
  frame->setObjectName("Frame");
  frame->setFrameShape(QFrame::Box);

  QGridLayout *grid = new QGridLayout;
  frame->setLayout(grid);

  int row = 0;
  QLabel *lab = new QLabel;
  lab->setObjectName("Label");
  QFont font = lab->font();
  font.setBold(true);
  lab->setFont(font);
  lab->setText(tr("Sliders style"));
  grid->addWidget(lab, row, 0, 1, 2, Qt::AlignCenter);
  row++;

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("\"Befor handle\" bar background color:"));
  grid->addWidget(lab, row, 0, Qt::AlignLeft | Qt::AlignVCenter);

  ColorButton *cb = new ColorButton;
  grid->addWidget(cb, row, 1, Qt::AlignVCenter | Qt::AlignLeft);
  row++;

  std::string id = "Slider::sub-page:horizontal";
  std::string attr = "background-color";
  QString str = settings->getStyleAttributeValue(id, attr);
  cb->setBakcroundColor(str);

  connect(cb, &ColorButton::clicked, this,
          [this, attr, id, cb]
            {
              colorDialog(cb,
                          settings->stringToColor(
                              cb->getBackGroundColor().toStdString()),
                          id, attr);
            });

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("\"After handle\" bar background color:"));
  grid->addWidget(lab, row, 0, Qt::AlignLeft | Qt::AlignVCenter);

  cb = new ColorButton;
  grid->addWidget(cb, row, 1, Qt::AlignVCenter | Qt::AlignLeft);
  row++;

  id = "Slider::add-page:horizontal";
  str = settings->getStyleAttributeValue(id, attr);
  cb->setBakcroundColor(str);

  connect(cb, &ColorButton::clicked, this,
          [this, attr, id, cb]
            {
              colorDialog(cb,
                          settings->stringToColor(
                              cb->getBackGroundColor().toStdString()),
                          id, attr);
            });

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Handle color:"));
  grid->addWidget(lab, row, 0, Qt::AlignLeft | Qt::AlignVCenter);

  cb = new ColorButton;
  grid->addWidget(cb, row, 1, Qt::AlignVCenter | Qt::AlignLeft);
  row++;

  id = "Slider::handle:horizontal";
  str = settings->getStyleAttributeValue(id, attr);
  cb->setBakcroundColor(str);

  connect(cb, &ColorButton::clicked, this,
          [this, attr, id, cb]
            {
              colorDialog(cb,
                          settings->stringToColor(
                              cb->getBackGroundColor().toStdString()),
                          id, attr);
            });

  return frame;
}

QWidget *
SettingsWindow::applybuttonWidget()
{
  QFrame *frame = new QFrame;
  frame->setObjectName("Frame");
  frame->setFrameShape(QFrame::Box);

  QGridLayout *grid = new QGridLayout;
  frame->setLayout(grid);

  int row = 0;
  QLabel *lab = new QLabel;
  lab->setObjectName("Label");
  QFont font = lab->font();
  font.setBold(true);
  lab->setFont(font);
  lab->setText(tr("\"Apply\" buttons style"));
  grid->addWidget(lab, row, 0, 1, 2, Qt::AlignCenter);
  row++;

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Background color:"));
  grid->addWidget(lab, row, 0, Qt::AlignLeft | Qt::AlignVCenter);

  ColorButton *cb = new ColorButton;
  grid->addWidget(cb, row, 1, Qt::AlignVCenter | Qt::AlignLeft);
  row++;

  std::string id = "ApplyButton";
  std::string attr = "background-color";
  QString str = settings->getStyleAttributeValue(id, attr);
  cb->setBakcroundColor(str);

  connect(cb, &ColorButton::clicked, this,
          [this, attr, id, cb]
            {
              colorDialog(cb,
                          settings->stringToColor(
                              cb->getBackGroundColor().toStdString()),
                          id, attr);
            });

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Font color:"));
  grid->addWidget(lab, row, 0, Qt::AlignLeft | Qt::AlignVCenter);

  cb = new ColorButton;
  grid->addWidget(cb, row, 1, Qt::AlignVCenter | Qt::AlignLeft);
  row++;

  attr = "color";
  str = settings->getStyleAttributeValue(id, attr);
  cb->setBakcroundColor(str);

  connect(cb, &ColorButton::clicked, this,
          [this, attr, id, cb]
            {
              colorDialog(cb,
                          settings->stringToColor(
                              cb->getBackGroundColor().toStdString()),
                          id, attr);
            });

  return frame;
}

QWidget *
SettingsWindow::cancelbuttonWidget()
{
  QFrame *frame = new QFrame;
  frame->setObjectName("Frame");
  frame->setFrameShape(QFrame::Box);

  QGridLayout *grid = new QGridLayout;
  frame->setLayout(grid);

  int row = 0;
  QLabel *lab = new QLabel;
  lab->setObjectName("Label");
  QFont font = lab->font();
  font.setBold(true);
  lab->setFont(font);
  lab->setText(tr("\"Cancel\" buttons style"));
  grid->addWidget(lab, row, 0, 1, 2, Qt::AlignCenter);
  row++;

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Background color:"));
  grid->addWidget(lab, row, 0, Qt::AlignLeft | Qt::AlignVCenter);

  ColorButton *cb = new ColorButton;
  grid->addWidget(cb, row, 1, Qt::AlignVCenter | Qt::AlignLeft);
  row++;

  std::string id = "CancelButton";
  std::string attr = "background-color";
  QString str = settings->getStyleAttributeValue(id, attr);
  cb->setBakcroundColor(str);

  connect(cb, &ColorButton::clicked, this,
          [this, attr, id, cb]
            {
              colorDialog(cb,
                          settings->stringToColor(
                              cb->getBackGroundColor().toStdString()),
                          id, attr);
            });

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Font color:"));
  grid->addWidget(lab, row, 0, Qt::AlignLeft | Qt::AlignVCenter);

  cb = new ColorButton;
  grid->addWidget(cb, row, 1, Qt::AlignVCenter | Qt::AlignLeft);
  row++;

  attr = "color";
  str = settings->getStyleAttributeValue(id, attr);
  cb->setBakcroundColor(str);

  connect(cb, &ColorButton::clicked, this,
          [this, attr, id, cb]
            {
              colorDialog(cb,
                          settings->stringToColor(
                              cb->getBackGroundColor().toStdString()),
                          id, attr);
            });

  return frame;
}

QWidget *
SettingsWindow::tableWidget()
{
  QFrame *frame = new QFrame;
  frame->setObjectName("Frame");
  frame->setFrameShape(QFrame::Box);

  QGridLayout *grid = new QGridLayout;
  frame->setLayout(grid);

  int row = 0;
  QLabel *lab = new QLabel;
  lab->setObjectName("Label");
  QFont font = lab->font();
  font.setBold(true);
  lab->setFont(font);
  lab->setText(tr("Tables style"));
  grid->addWidget(lab, row, 0, 1, 2, Qt::AlignCenter);
  row++;

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Background color:"));
  grid->addWidget(lab, row, 0, Qt::AlignLeft | Qt::AlignVCenter);

  ColorButton *cb = new ColorButton;
  grid->addWidget(cb, row, 1, Qt::AlignVCenter | Qt::AlignLeft);
  row++;

  std::string id = "Table";
  std::string attr = "background-color";
  QString str = settings->getStyleAttributeValue(id, attr);
  cb->setBakcroundColor(str);

  connect(cb, &ColorButton::clicked, this,
          [this, attr, id, cb]
            {
              colorDialog(cb,
                          settings->stringToColor(
                              cb->getBackGroundColor().toStdString()),
                          id, attr);
            });

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Font color:"));
  grid->addWidget(lab, row, 0, Qt::AlignLeft | Qt::AlignVCenter);

  cb = new ColorButton;
  grid->addWidget(cb, row, 1, Qt::AlignVCenter | Qt::AlignLeft);
  row++;

  attr = "color";
  str = settings->getStyleAttributeValue(id, attr);
  cb->setBakcroundColor(str);

  connect(cb, &ColorButton::clicked, this,
          [this, attr, id, cb]
            {
              colorDialog(cb,
                          settings->stringToColor(
                              cb->getBackGroundColor().toStdString()),
                          id, attr);
            });

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Selection backgorund color:"));
  grid->addWidget(lab, row, 0, Qt::AlignLeft | Qt::AlignVCenter);

  cb = new ColorButton;
  grid->addWidget(cb, row, 1, Qt::AlignVCenter | Qt::AlignLeft);
  row++;

  attr = "selection-background-color";
  str = settings->getStyleAttributeValue(id, attr);
  cb->setBakcroundColor(str);

  connect(cb, &ColorButton::clicked, this,
          [this, attr, id, cb]
            {
              colorDialog(cb,
                          settings->stringToColor(
                              cb->getBackGroundColor().toStdString()),
                          id, attr);
            });

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Selection font color:"));
  grid->addWidget(lab, row, 0, Qt::AlignLeft | Qt::AlignVCenter);

  cb = new ColorButton;
  grid->addWidget(cb, row, 1, Qt::AlignVCenter | Qt::AlignLeft);
  row++;

  attr = "selection-color";
  str = settings->getStyleAttributeValue(id, attr);
  cb->setBakcroundColor(str);

  connect(cb, &ColorButton::clicked, this,
          [this, attr, id, cb]
            {
              colorDialog(cb,
                          settings->stringToColor(
                              cb->getBackGroundColor().toStdString()),
                          id, attr);
            });

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Border color:"));
  grid->addWidget(lab, row, 0, Qt::AlignLeft | Qt::AlignVCenter);

  cb = new ColorButton;
  grid->addWidget(cb, row, 1, Qt::AlignVCenter | Qt::AlignLeft);
  row++;

  id = "Table";
  attr = "border-color";
  str = settings->getStyleAttributeValue(id, attr);
  cb->setBakcroundColor(str);

  connect(cb, &ColorButton::clicked, this,
          [this, attr, id, cb]
            {
              colorDialog(cb,
                          settings->stringToColor(
                              cb->getBackGroundColor().toStdString()),
                          id, attr);
            });

  lab = new QLabel;
  lab->setObjectName("Label");
  font = lab->font();
  font.setBold(true);
  lab->setFont(font);
  lab->setText(tr("Tables auxiliary elements style"));
  grid->addWidget(lab, row, 0, 1, 2, Qt::AlignCenter);
  row++;

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Headers and scroll bars background color:"));
  grid->addWidget(lab, row, 0, Qt::AlignLeft | Qt::AlignVCenter);

  cb = new ColorButton;
  grid->addWidget(cb, row, 1, Qt::AlignVCenter | Qt::AlignLeft);
  row++;

  id = "Table QHeaderView";
  attr = "background-color";
  str = settings->getStyleAttributeValue(id, attr);
  cb->setBakcroundColor(str);

  connect(cb, &ColorButton::clicked, this,
          [this, attr, id, cb]
            {
              colorDialog(cb,
                          settings->stringToColor(
                              cb->getBackGroundColor().toStdString()),
                          id, attr);
            });

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Headers and scroll bars font color:"));
  grid->addWidget(lab, row, 0, Qt::AlignLeft | Qt::AlignVCenter);

  cb = new ColorButton;
  grid->addWidget(cb, row, 1, Qt::AlignVCenter | Qt::AlignLeft);
  row++;

  id = "Table QHeaderView";
  attr = "color";
  str = settings->getStyleAttributeValue(id, attr);
  cb->setBakcroundColor(str);

  connect(cb, &ColorButton::clicked, this,
          [this, attr, id, cb]
            {
              colorDialog(cb,
                          settings->stringToColor(
                              cb->getBackGroundColor().toStdString()),
                          id, attr);
            });

  return frame;
}

QWidget *
SettingsWindow::texteditWidget()
{
  QFrame *frame = new QFrame;
  frame->setObjectName("Frame");
  frame->setFrameShape(QFrame::Box);

  QGridLayout *grid = new QGridLayout;
  frame->setLayout(grid);

  int row = 0;
  QLabel *lab = new QLabel;
  lab->setObjectName("Label");
  QFont font = lab->font();
  font.setBold(true);
  lab->setFont(font);
  lab->setText(tr("Text fields style"));
  grid->addWidget(lab, row, 0, 1, 2, Qt::AlignCenter);
  row++;

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Background color:"));
  grid->addWidget(lab, row, 0, Qt::AlignLeft | Qt::AlignVCenter);

  ColorButton *cb = new ColorButton;
  grid->addWidget(cb, row, 1, Qt::AlignVCenter | Qt::AlignLeft);
  row++;

  std::string id = "TextEdit";
  std::string attr = "background-color";
  QString str = settings->getStyleAttributeValue(id, attr);
  cb->setBakcroundColor(str);

  connect(cb, &ColorButton::clicked, this,
          [this, attr, id, cb]
            {
              colorDialog(cb,
                          settings->stringToColor(
                              cb->getBackGroundColor().toStdString()),
                          id, attr);
            });

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Font color:"));
  grid->addWidget(lab, row, 0, Qt::AlignLeft | Qt::AlignVCenter);

  cb = new ColorButton;
  grid->addWidget(cb, row, 1, Qt::AlignVCenter | Qt::AlignLeft);
  row++;

  attr = "color";
  str = settings->getStyleAttributeValue(id, attr);
  cb->setBakcroundColor(str);

  connect(cb, &ColorButton::clicked, this,
          [this, attr, id, cb]
            {
              colorDialog(cb,
                          settings->stringToColor(
                              cb->getBackGroundColor().toStdString()),
                          id, attr);
            });

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Border color:"));
  grid->addWidget(lab, row, 0, Qt::AlignLeft | Qt::AlignVCenter);

  cb = new ColorButton;
  grid->addWidget(cb, row, 1, Qt::AlignVCenter | Qt::AlignLeft);
  row++;

  id = "TextEdit";
  attr = "border-color";
  str = settings->getStyleAttributeValue(id, attr);
  cb->setBakcroundColor(str);

  connect(cb, &ColorButton::clicked, this,
          [this, attr, id, cb]
            {
              colorDialog(cb,
                          settings->stringToColor(
                              cb->getBackGroundColor().toStdString()),
                          id, attr);
            });

  lab = new QLabel;
  lab->setObjectName("Label");
  font = lab->font();
  font.setBold(true);
  lab->setFont(font);
  lab->setText(tr("Text fields scroll bars style"));
  grid->addWidget(lab, row, 0, 1, 2, Qt::AlignCenter);
  row++;

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Background color:"));
  grid->addWidget(lab, row, 0, Qt::AlignLeft | Qt::AlignVCenter);

  cb = new ColorButton;
  grid->addWidget(cb, row, 1, Qt::AlignVCenter | Qt::AlignLeft);
  row++;

  id = "TextEdit QScrollBar";
  attr = "background-color";
  str = settings->getStyleAttributeValue(id, attr);
  cb->setBakcroundColor(str);

  connect(cb, &ColorButton::clicked, this,
          [this, attr, id, cb]
            {
              colorDialog(cb,
                          settings->stringToColor(
                              cb->getBackGroundColor().toStdString()),
                          id, attr);
            });

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Arrows color:"));
  grid->addWidget(lab, row, 0, Qt::AlignLeft | Qt::AlignVCenter);

  cb = new ColorButton;
  grid->addWidget(cb, row, 1, Qt::AlignVCenter | Qt::AlignLeft);
  row++;

  attr = "color";
  str = settings->getStyleAttributeValue(id, attr);
  cb->setBakcroundColor(str);

  connect(cb, &ColorButton::clicked, this,
          [this, attr, id, cb]
            {
              colorDialog(cb,
                          settings->stringToColor(
                              cb->getBackGroundColor().toStdString()),
                          id, attr);
            });

  return frame;
}

QWidget *
SettingsWindow::progressbarWidget()
{
  QFrame *frame = new QFrame;
  frame->setObjectName("Frame");
  frame->setFrameShape(QFrame::Box);

  QGridLayout *grid = new QGridLayout;
  frame->setLayout(grid);

  int row = 0;
  QLabel *lab = new QLabel;
  lab->setObjectName("Label");
  QFont font = lab->font();
  font.setBold(true);
  lab->setFont(font);
  lab->setText(tr("Progress bars style"));
  grid->addWidget(lab, row, 0, 1, 2, Qt::AlignCenter);
  row++;

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Background color:"));
  grid->addWidget(lab, row, 0, Qt::AlignLeft | Qt::AlignVCenter);

  ColorButton *cb = new ColorButton;
  grid->addWidget(cb, row, 1, Qt::AlignVCenter | Qt::AlignLeft);
  row++;

  std::string id = "ProgressBar::chunk";
  std::string attr = "background-color";
  QString str = settings->getStyleAttributeValue(id, attr);
  cb->setBakcroundColor(str);

  connect(cb, &ColorButton::clicked, this,
          [this, attr, id, cb]
            {
              colorDialog(cb,
                          settings->stringToColor(
                              cb->getBackGroundColor().toStdString()),
                          id, attr);
            });

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Font color:"));
  grid->addWidget(lab, row, 0, Qt::AlignLeft | Qt::AlignVCenter);

  cb = new ColorButton;
  grid->addWidget(cb, row, 1, Qt::AlignVCenter | Qt::AlignLeft);
  row++;

  id = "ProgressBar";
  attr = "color";
  str = settings->getStyleAttributeValue(id, attr);
  cb->setBakcroundColor(str);

  connect(cb, &ColorButton::clicked, this,
          [this, attr, id, cb]
            {
              colorDialog(cb,
                          settings->stringToColor(
                              cb->getBackGroundColor().toStdString()),
                          id, attr);
            });

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Border color:"));
  grid->addWidget(lab, row, 0, Qt::AlignLeft | Qt::AlignVCenter);

  cb = new ColorButton;
  grid->addWidget(cb, row, 1, Qt::AlignVCenter | Qt::AlignLeft);
  row++;

  id = "ProgressBar";
  attr = "border-color";
  str = settings->getStyleAttributeValue(id, attr);
  cb->setBakcroundColor(str);

  connect(cb, &ColorButton::clicked, this,
          [this, attr, id, cb]
            {
              colorDialog(cb,
                          settings->stringToColor(
                              cb->getBackGroundColor().toStdString()),
                          id, attr);
            });

  return frame;
}

QWidget *
SettingsWindow::menuWidget()
{
  QFrame *frame = new QFrame;
  frame->setObjectName("Frame");
  frame->setFrameShape(QFrame::Box);

  QGridLayout *grid = new QGridLayout;
  frame->setLayout(grid);

  int row = 0;
  QLabel *lab = new QLabel;
  lab->setObjectName("Label");
  QFont font = lab->font();
  font.setBold(true);
  lab->setFont(font);
  lab->setText(tr("Menus style"));
  grid->addWidget(lab, row, 0, 1, 2, Qt::AlignCenter);
  row++;

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Background color:"));
  grid->addWidget(lab, row, 0, Qt::AlignLeft | Qt::AlignVCenter);

  ColorButton *cb = new ColorButton;
  grid->addWidget(cb, row, 1, Qt::AlignVCenter | Qt::AlignLeft);
  row++;

  std::string id = "Menu";
  std::string attr = "background-color";
  QString str = settings->getStyleAttributeValue(id, attr);
  cb->setBakcroundColor(str);

  connect(cb, &ColorButton::clicked, this,
          [this, attr, id, cb]
            {
              colorDialog(cb,
                          settings->stringToColor(
                              cb->getBackGroundColor().toStdString()),
                          id, attr);
            });

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Font color:"));
  grid->addWidget(lab, row, 0, Qt::AlignLeft | Qt::AlignVCenter);

  cb = new ColorButton;
  grid->addWidget(cb, row, 1, Qt::AlignVCenter | Qt::AlignLeft);
  row++;

  attr = "color";
  str = settings->getStyleAttributeValue(id, attr);
  cb->setBakcroundColor(str);

  connect(cb, &ColorButton::clicked, this,
          [this, attr, id, cb]
            {
              colorDialog(cb,
                          settings->stringToColor(
                              cb->getBackGroundColor().toStdString()),
                          id, attr);
            });

  return frame;
}

QWidget *
SettingsWindow::checkboxWidget()
{
  QFrame *frame = new QFrame;
  frame->setObjectName("Frame");
  frame->setFrameShape(QFrame::Box);

  QGridLayout *grid = new QGridLayout;
  frame->setLayout(grid);

  int row = 0;
  QLabel *lab = new QLabel;
  lab->setObjectName("Label");
  QFont font = lab->font();
  font.setBold(true);
  lab->setFont(font);
  lab->setText(tr("Check boxes style"));
  grid->addWidget(lab, row, 0, 1, 2, Qt::AlignCenter);
  row++;

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Font color:"));
  grid->addWidget(lab, row, 0, Qt::AlignLeft | Qt::AlignVCenter);

  ColorButton *cb = new ColorButton;
  grid->addWidget(cb, row, 1, Qt::AlignVCenter | Qt::AlignLeft);
  row++;

  std::string id = "CheckBox";
  std::string attr = "color";
  QString str = settings->getStyleAttributeValue(id, attr);
  cb->setBakcroundColor(str);

  connect(cb, &ColorButton::clicked, this,
          [this, attr, id, cb]
            {
              colorDialog(cb,
                          settings->stringToColor(
                              cb->getBackGroundColor().toStdString()),
                          id, attr);
            });

  return frame;
}

QWidget *
SettingsWindow::frameWidget()
{
  QFrame *frame = new QFrame;
  frame->setObjectName("Frame");
  frame->setFrameShape(QFrame::Box);

  QGridLayout *grid = new QGridLayout;
  frame->setLayout(grid);

  int row = 0;
  QLabel *lab = new QLabel;
  lab->setObjectName("Label");
  QFont font = lab->font();
  font.setBold(true);
  lab->setFont(font);
  lab->setText(tr("Frames style"));
  grid->addWidget(lab, row, 0, 1, 2, Qt::AlignCenter);
  row++;

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Border color:"));
  grid->addWidget(lab, row, 0, Qt::AlignLeft | Qt::AlignVCenter);

  ColorButton *cb = new ColorButton;
  grid->addWidget(cb, row, 1, Qt::AlignVCenter | Qt::AlignLeft);
  row++;

  std::string id = "Frame";
  std::string attr = "border-color";
  QString str = settings->getStyleAttributeValue(id, attr);
  cb->setBakcroundColor(str);

  connect(cb, &ColorButton::clicked, this,
          [this, attr, id, cb]
            {
              colorDialog(cb,
                          settings->stringToColor(
                              cb->getBackGroundColor().toStdString()),
                          id, attr);
            });

  return frame;
}

QWidget *
SettingsWindow::scrollareaWidget()
{
  QFrame *frame = new QFrame;
  frame->setObjectName("Frame");
  frame->setFrameShape(QFrame::Box);

  QGridLayout *grid = new QGridLayout;
  frame->setLayout(grid);

  int row = 0;
  QLabel *lab = new QLabel;
  lab->setObjectName("Label");
  QFont font = lab->font();
  font.setBold(true);
  lab->setFont(font);
  lab->setText(tr("Scroll areas style"));
  grid->addWidget(lab, row, 0, 1, 2, Qt::AlignCenter);
  row++;

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Background color:"));
  grid->addWidget(lab, row, 0, Qt::AlignLeft | Qt::AlignVCenter);

  ColorButton *cb = new ColorButton;
  grid->addWidget(cb, row, 1, Qt::AlignVCenter | Qt::AlignLeft);
  row++;

  std::string id = "ScrollArea";
  std::string attr = "background-color";
  QString str = settings->getStyleAttributeValue(id, attr);
  cb->setBakcroundColor(str);

  connect(cb, &ColorButton::clicked, this,
          [this, attr, id, cb]
            {
              colorDialog(cb,
                          settings->stringToColor(
                              cb->getBackGroundColor().toStdString()),
                          id, attr);
            });

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Scroll bars background color:"));
  grid->addWidget(lab, row, 0, Qt::AlignLeft | Qt::AlignVCenter);

  cb = new ColorButton;
  grid->addWidget(cb, row, 1, Qt::AlignVCenter | Qt::AlignLeft);
  row++;

  id = "ScrollArea QScrollBar";
  attr = "background-color";
  str = settings->getStyleAttributeValue(id, attr);
  cb->setBakcroundColor(str);

  connect(cb, &ColorButton::clicked, this,
          [this, attr, id, cb]
            {
              colorDialog(cb,
                          settings->stringToColor(
                              cb->getBackGroundColor().toStdString()),
                          id, attr);
            });

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Scroll bars arrows color:"));
  grid->addWidget(lab, row, 0, Qt::AlignLeft | Qt::AlignVCenter);

  cb = new ColorButton;
  grid->addWidget(cb, row, 1, Qt::AlignVCenter | Qt::AlignLeft);
  row++;

  attr = "color";
  str = settings->getStyleAttributeValue(id, attr);
  cb->setBakcroundColor(str);

  connect(cb, &ColorButton::clicked, this,
          [this, attr, id, cb]
            {
              colorDialog(cb,
                          settings->stringToColor(
                              cb->getBackGroundColor().toStdString()),
                          id, attr);
            });

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Border color:"));
  grid->addWidget(lab, row, 0, Qt::AlignLeft | Qt::AlignVCenter);

  cb = new ColorButton;
  grid->addWidget(cb, row, 1, Qt::AlignVCenter | Qt::AlignLeft);
  row++;

  id = "ScrollArea";
  attr = "border-color";
  str = settings->getStyleAttributeValue(id, attr);
  cb->setBakcroundColor(str);

  connect(cb, &ColorButton::clicked, this,
          [this, attr, id, cb]
            {
              colorDialog(cb,
                          settings->stringToColor(
                              cb->getBackGroundColor().toStdString()),
                          id, attr);
            });

  return frame;
}

void
SettingsWindow::colorDialog(ColorButton *cb, const QColor &color,
                            const std::string &id,
                            const std::string &attribute)
{
  QColorDialog *cd = new QColorDialog(color, this);
  cd->setAttribute(Qt::WA_DeleteOnClose);
  cd->setWindowModality(Qt::WindowModal);
  cd->setOption(QColorDialog::ShowAlphaChannel);

  connect(cd, &QColorDialog::colorSelected, this,
          [cb, this, id, attribute](const QColor &color)
            {
              int a, r, g, b;
              color.getRgb(&r, &g, &b, &a);

              std::string str = "rgba(";
              std::stringstream strm;
              strm.imbue(std::locale("C"));
              strm << r;
              str += strm.str();

              strm.clear();
              strm.str("");
              strm << g;
              str += ", ";
              str += strm.str();

              strm.clear();
              strm.str("");
              strm << b;
              str += ", ";
              str += strm.str();

              strm.clear();
              strm.str("");
              strm << a;
              str += ", ";
              str += strm.str();

              str += ")";

              settings_copy->setStyleAttributeValue(id, attribute, str);
              if(id == "MainWindow")
                {
                  settings_copy->setStyleAttributeValue("Window", attribute,
                                                        str);
                }
              if(id == "Table QHeaderView")
                {
                  settings_copy->setStyleAttributeValue(
                      "Table QTableCornerButton", attribute, str);
                  settings_copy->setStyleAttributeValue(
                      "Table QHeaderView::section", attribute, str);
                  settings_copy->setStyleAttributeValue("Table QScrollBar",
                                                        attribute, str);
                }
              if(id == "ScrollArea QScrollBar")
                {
                  settings_copy->setStyleAttributeValue(
                      "CoverWindowScrollArea QScrollBar", attribute, str);
                }
              cb->setBakcroundColor(str.c_str());
            });

  cd->show();
}

void
SettingsWindow::openFileDialog(QLineEdit *line)
{
  QFileDialog *fd = new QFileDialog(this);
  fd->setAttribute(Qt::WA_DeleteOnClose);
  fd->setWindowModality(Qt::WindowModal);

  fd->setDirectory(QDir::homePath());
  fd->setAcceptMode(QFileDialog::AcceptOpen);

  QList<QByteArray> list = QImageReader::supportedMimeTypes();
  QStringList mime_list;
  for(qsizetype i = 0; i < list.size(); i++)
    {
      QString str(list[i]);
      mime_list.append(str);
    }
  fd->setMimeTypeFilters(mime_list);

  connect(fd, &QFileDialog::fileSelected, line, &QLineEdit::setText);

  fd->show();
}

void
SettingsWindow::paintEvent(QPaintEvent *event)
{
  QStyleOption opt;
  opt.initFrom(this);
  QPainter p(this);
  style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void
SettingsWindow::resetToDafaultDialog(const bool &system_default)
{
  StyledWindow *window = new StyledWindow(this);
  window->setWindowModality(Qt::WindowModal);
  window->setObjectName("Window");

  QVBoxLayout *v_box = new QVBoxLayout;
  window->setLayout(v_box);

  QLabel *lab = new QLabel;
  lab->setText(tr("Are you sure?"));
  lab->setObjectName("Label");
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
          [this, window, system_default]
            {
              if(system_default)
                {
                  settings->resetColorsToSystemDefault();
                }
              else
                {
                  settings->resetToDefault();
                }
              SettingsWindow *sw
                  = new SettingsWindow(this->parentWidget(), settings);
              sw->createWindow();

              sw->show();

              window->close();
              this->close();
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
SettingsWindow::fontSelectionDialog(QPushButton *font_button)
{
  QFontDialog *fd = new QFontDialog(this);
  fd->setAttribute(Qt::WA_DeleteOnClose);
  fd->setWindowModality(Qt::WindowModal);

  fd->setCurrentFont(settings_copy->getFont());
  fd->setOption(QFontDialog::ScalableFonts);

  connect(fd, &QFontDialog::fontSelected, this,
          [this, font_button](const QFont &font)
            {
              font_button->setText(font.family());
              settings_copy->setFont(font);
            });

  fd->show();
}
