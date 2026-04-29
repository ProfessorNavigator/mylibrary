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

#include <BaseKeeper.h>
#include <CollectionBaseImportWindow.h>
#include <QFileDialog>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QPainter>
#include <QPushButton>
#include <QScreen>
#include <QStyleOption>
#include <QVBoxLayout>
#include <StyledWindow.h>
#include <iostream>

CollectionBaseImportWindow::CollectionBaseImportWindow(QWidget *parent)
    : QWidget(parent)
{
  this->setWindowFlag(Qt::Window, true);
  this->setAttribute(Qt::WA_DeleteOnClose);
  this->setWindowTitle(tr("Collection base import"));
  this->setWindowModality(Qt::WindowModal);

  this->setObjectName("Window");
}

void
CollectionBaseImportWindow::createWindow()
{
  QVBoxLayout *v_box = new QVBoxLayout;
  this->setLayout(v_box);

  QLabel *lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("New collection name:"));
  v_box->addWidget(lab, 0, Qt::AlignLeft);

  collection_name = new QLineEdit;
  v_box->addWidget(collection_name);

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Path to source base:"));
  v_box->addWidget(lab, 0, Qt::AlignLeft);

  src_path = new QLineEdit;
  src_path->setObjectName("LineEdit");
  v_box->addWidget(src_path);

  QPushButton *open = new QPushButton;
  open->setText(tr("Open"));
  QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(open);
  shadow->setBlurRadius(12);
  shadow->setOffset(0, 2);
  shadow->setColor(QColor(0, 0, 0, 120));
  open->setGraphicsEffect(shadow);
  open->setObjectName("ApplyButton");
  connect(open, &QPushButton::clicked, this,
          [this]
            {
              openDialog(OpenDialogType::BaseSrc);
            });
  v_box->addWidget(open, 0, Qt::AlignRight);

  QHBoxLayout *h_box = new QHBoxLayout;
  v_box->addLayout(h_box);

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Anchor object:"));
  h_box->addWidget(lab, 0, Qt::AlignLeft);

  anchor_fnm = new QLabel;
  anchor_fnm->setObjectName("Label");
  QFont font = anchor_fnm->font();
  font.setItalic(true);
  anchor_fnm->setFont(font);
  h_box->addWidget(anchor_fnm, 0, Qt::AlignLeft);

  h_box->addStretch();

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Anchor object (see name above) full path:"));
  v_box->addWidget(lab, 0, Qt::AlignLeft);

  anchor_path = new QLineEdit;
  anchor_path->setObjectName("LineEdit");
  anchor_path->setEnabled(false);
  v_box->addWidget(anchor_path);

  open = new QPushButton;
  open->setText(tr("Open"));
  shadow = new QGraphicsDropShadowEffect(open);
  shadow->setBlurRadius(12);
  shadow->setOffset(0, 2);
  shadow->setColor(QColor(0, 0, 0, 120));
  open->setGraphicsEffect(shadow);
  open->setObjectName("ApplyButton");
  open->setEnabled(false);
  connect(open, &QPushButton::clicked, this,
          [this]
            {
              openDialog(OpenDialogType::Anchor);
            });
  v_box->addWidget(open, 0, Qt::AlignRight);

  h_box = new QHBoxLayout;
  v_box->addLayout(h_box);

  QPushButton *import = new QPushButton;
  import->setText(tr("Import"));
  shadow = new QGraphicsDropShadowEffect(import);
  shadow->setBlurRadius(12);
  shadow->setOffset(0, 2);
  shadow->setColor(QColor(0, 0, 0, 120));
  import->setGraphicsEffect(shadow);
  import->setObjectName("ApplyButton");
  connect(import, &QPushButton::clicked, this,
          &CollectionBaseImportWindow::checkResult);
  h_box->addWidget(import, 0, Qt::AlignCenter);

  QPushButton *cancel = new QPushButton;
  cancel->setText(tr("Cancel"));
  shadow = new QGraphicsDropShadowEffect(cancel);
  shadow->setBlurRadius(12);
  shadow->setOffset(0, 2);
  shadow->setColor(QColor(0, 0, 0, 120));
  cancel->setGraphicsEffect(shadow);
  cancel->setObjectName("CancelButton");
  connect(cancel, &QPushButton::clicked, this,
          &CollectionBaseImportWindow::close);
  h_box->addWidget(cancel, 0, Qt::AlignCenter);

  connect(src_path, &QLineEdit::textChanged, this,
          [open, this](const QString &txt)
            {
              std::string str = txt.toStdString();
              std::filesystem::path p = std::u8string(str.begin(), str.end());
              if(std::filesystem::exists(p))
                {
                  UDBElement el;
                  try
                    {
                      el = BaseKeeper::getBaseAnchorFileName(p);
                      BaseID bid;
                      if(bid.getId(el) != BaseID::AnchorBasePath)
                        {
                          throw std::runtime_error(
                              "CollectionBaseImportWindow::createWindow: "
                              "incorrect id");
                        }
                      auto it = std::find_if(el.subelements.begin(),
                                             el.subelements.end(),
                                             [bid](const UDBElement &el)
                                               {
                                                 switch(bid.getId(el))
                                                   {
                                                   case BaseID::File:
                                                   case BaseID::Dir:
                                                   case BaseID::Symlink:
                                                     {
                                                       return true;
                                                     }
                                                   default:
                                                     return false;
                                                   }
                                               });
                      if(it == el.subelements.end())
                        {
                          throw std::runtime_error(
                              "CollectionBaseImportWindow::createWindow: "
                              "incorrect object");
                        }
                      else
                        {
                          anchor_object_type = bid.getId(*it);
                        }
                    }
                  catch(std::exception &er)
                    {
                      std::cout << er.what() << std::endl;
                      anchor_fnm->setText(tr("error"));
                      anchor_path->setText("");
                      anchor_path->setEnabled(false);
                      anchor_path->setObjectName("");
                      open->setEnabled(false);
                      anchor_object_type = BaseID::Error;
                      return void();
                    }
                  anchor_fnm->setText(el.content.c_str());
                  anchor_path->setEnabled(true);
                  open->setEnabled(true);
                }
              else
                {
                  anchor_fnm->setText("");
                  anchor_path->setText("");
                  anchor_path->setEnabled(false);
                  open->setEnabled(false);
                  anchor_object_type = BaseID::Error;
                }
            });

  QScreen *screen = this->parentWidget()->screen();
  QSize av_sz = screen->availableSize();
  av_sz.setWidth(av_sz.width() * 0.5);
  av_sz.setHeight(this->sizeHint().height());
  this->resize(av_sz);
}

void
CollectionBaseImportWindow::openDialog(const OpenDialogType &type)
{
  QFileDialog *fd = new QFileDialog(this);
  fd->setAttribute(Qt::WA_DeleteOnClose);
  fd->setWindowModality(Qt::WindowModal);

  fd->setAcceptMode(QFileDialog::AcceptOpen);
  fd->setDirectory(QDir::homePath());

  switch(type)
    {
    case OpenDialogType::BaseSrc:
      {
        connect(fd, &QFileDialog::fileSelected, this,
                [this](const QString &file)
                  {
                    src_path->setText(file);
                  });
        break;
      }
    case OpenDialogType::Anchor:
      {
        if(anchor_object_type == BaseID::Dir)
          {
            fd->setFileMode(QFileDialog::Directory);
          }
        connect(fd, &QFileDialog::fileSelected, this,
                [this](const QString &file)
                  {
                    anchor_path->setText(file);
                  });
        break;
      }
    default:
      break;
    }

  fd->show();
}

void
CollectionBaseImportWindow::checkResult()
{
  QString str = collection_name->text();
  if(str.isEmpty())
    {
      errorDialog(ErrorType::ColNameEmpty);
      return void();
    }
  std::string l_str = QDir::homePath().toStdString();
  std::filesystem::path base_path = std::u8string(l_str.begin(), l_str.end());
  base_path /= std::filesystem::path(u8".local");
  base_path /= std::filesystem::path(u8"share");
  base_path /= std::filesystem::path(u8"MyLibrary");
  base_path /= std::filesystem::path(u8"Collections");
  l_str = str.toStdString();
  base_path
      /= std::filesystem::path(std::u8string(l_str.begin(), l_str.end()));
  base_path /= std::filesystem::path(u8"base");

  if(std::filesystem::exists(base_path.parent_path()))
    {
      errorDialog(ErrorType::CollectionExists);
      return void();
    }

  str = anchor_path->text();
  if(str.isEmpty())
    {
      errorDialog(ErrorType::AnchorPathEmpty);
      return void();
    }

  l_str = str.toStdString();
  std::filesystem::path anchor_obj = std::u8string(l_str.begin(), l_str.end());
  if(!std::filesystem::exists(anchor_obj))
    {
      errorDialog(ErrorType::AnchorNotExists);
      return void();
    }

  std::u8string u8str = anchor_obj.filename().u8string();
  l_str = std::string(u8str.begin(), u8str.end());
  str = anchor_fnm->text();
  if(l_str != str.toStdString())
    {
      errorDialog(ErrorType::AnchorNamesNotEqual);
      return void();
    }

  str = src_path->text();
  l_str = str.toStdString();
  std::filesystem::path source_p = std::u8string(l_str.begin(), l_str.end());

  try
    {
      BaseKeeper::importCollectionBase(source_p, base_path, anchor_obj);
    }
  catch(std::exception &er)
    {
      errorDialog(ErrorType::ImportError, er.what());
      return void();
    }
  emit signalCollectionImported(base_path);
  errorDialog(ErrorType::Success);
}

void
CollectionBaseImportWindow::errorDialog(const ErrorType &er,
                                        const QString &txt)
{
  StyledWindow *window = nullptr;

  QVBoxLayout *v_box = new QVBoxLayout;

  switch(er)
    {
    case ErrorType::ColNameEmpty:
      {
        window = new StyledWindow(this);

        QLabel *lab = new QLabel;
        lab->setObjectName("Label");
        lab->setText(tr("Collection name is empty!"));
        v_box->addWidget(lab);
        break;
      }
    case ErrorType::CollectionExists:
      {
        window = new StyledWindow(this);

        QLabel *lab = new QLabel;
        lab->setObjectName("Label");
        lab->setText(tr("Collection already exists!"));
        v_box->addWidget(lab);
        break;
      }
    case ErrorType::AnchorPathEmpty:
      {
        window = new StyledWindow(this);

        QLabel *lab = new QLabel;
        lab->setObjectName("Label");
        lab->setText(tr("Anchor object path is empty!"));
        v_box->addWidget(lab);
        break;
      }
    case ErrorType::AnchorNotExists:
      {
        window = new StyledWindow(this);

        QLabel *lab = new QLabel;
        lab->setObjectName("Label");
        lab->setText(tr("Anchor object does not exist!"));
        v_box->addWidget(lab);
        break;
      }
    case ErrorType::AnchorNamesNotEqual:
      {
        window = new StyledWindow(this);

        QLabel *lab = new QLabel;
        lab->setObjectName("Label");
        lab->setText(tr("Anchor object path is incorrect!"));
        v_box->addWidget(lab);
        break;
      }
    case ErrorType::ImportError:
      {
        window = new StyledWindow(this);

        QLabel *lab = new QLabel;
        lab->setObjectName("Label");
        lab->setText(tr("Error!"));
        v_box->addWidget(lab);

        lab = new QLabel;
        lab->setObjectName("Label");
        lab->setText(txt);
        v_box->addWidget(lab);
        break;
      }
    case ErrorType::Success:
      {
        window = new StyledWindow(this->parentWidget());

        QLabel *lab = new QLabel;
        lab->setObjectName("Label");
        lab->setText(
            tr("Collection has been imported!") + "\n"
            + tr("Note: it is recommended to refresh new collection"));
        v_box->addWidget(lab);

        lab = new QLabel;
        lab->setObjectName("Label");
        lab->setText(txt);
        v_box->addWidget(lab);

        this->close();
        break;
      }
    default:
      break;
    }

  if(window == nullptr)
    {
      delete v_box;
      return void();
    }
  window->setWindowModality(Qt::WindowModal);
  window->setObjectName("Window");
  window->setLayout(v_box);

  QPushButton *close = new QPushButton;
  close->setText(tr("Close"));
  QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(close);
  shadow->setBlurRadius(12);
  shadow->setOffset(0, 2);
  shadow->setColor(QColor(0, 0, 0, 120));
  close->setGraphicsEffect(shadow);
  close->setObjectName("ClearButton");
  connect(close, &QPushButton::clicked, window, &StyledWindow::close);
  v_box->addWidget(close, 0, Qt::AlignCenter);

  window->show();
}

void
CollectionBaseImportWindow::paintEvent(QPaintEvent *event)
{
  QStyleOption opt;
  opt.initFrom(this);
  QPainter p(this);
  style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
