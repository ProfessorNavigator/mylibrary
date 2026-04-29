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

#include <AboutWindow.h>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QScrollArea>
#include <QStyleOption>
#include <QVBoxLayout>

AboutWindow::AboutWindow(QWidget *parent,
                         const std::shared_ptr<MLBookProc> &mlbp)
    : QWidget(parent)
{
  this->mlbp = mlbp;

  this->setWindowFlag(Qt::Window);
  this->setAttribute(Qt::WA_DeleteOnClose);
  this->setWindowModality(Qt::WindowModal);
  this->setWindowTitle(tr("About"));

  this->setObjectName("Window");
}

void
AboutWindow::createWindow()
{
  QVBoxLayout *main_box = new QVBoxLayout;
  this->setLayout(main_box);

  QScrollArea *scrl = new QScrollArea;
  scrl->setObjectName("ScrollArea");
  scrl->setWidgetResizable(true);
  main_box->addWidget(scrl);

  QWidget *viewport = new QWidget;
  viewport->setObjectName("ScrollAreaViewport");

  QVBoxLayout *v_box = new QVBoxLayout;
  viewport->setLayout(v_box);

  QLabel *lab = new QLabel;
  lab->setObjectName("Label");
  QFont font = lab->font();
  font.setBold(true);
  lab->setFont(font);
  lab->setText("MyLibrary");
  v_box->addWidget(lab, 0, Qt::AlignCenter);

  QPixmap map(":/icons/mylibrary.svg");

  lab = new QLabel;
  lab->setPixmap(map);
  lab->setAlignment(Qt::AlignCenter);
  v_box->addWidget(lab);

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setWordWrap(true);
  lab->setText(tr(
      "MyLibrary is an application for managing electronic book collections"));
  v_box->addWidget(lab);

  QHBoxLayout *h_box = new QHBoxLayout;
  v_box->addLayout(h_box);

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Supported books formats:"));
  h_box->addWidget(lab);

  std::vector<std::string> formats = mlbp->getSupportedFileTypes();
  QString str;
  for(auto it = formats.begin(); it != formats.end(); it++)
    {
      if(*it == "zip")
        {
          break;
        }
      if(!str.isEmpty())
        {
          str += ", ";
        }
      str += it->c_str();
    }

  str += ", fbd (";
  str += tr("fbd - any files, not only books");
  str += ")";

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setWordWrap(true);
  lab->setText(str);
  h_box->addWidget(lab);

  h_box->addStretch();

  h_box = new QHBoxLayout;
  v_box->addLayout(h_box);

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Supported archives formats:"));
  h_box->addWidget(lab);

  formats = mlbp->getSupportedArchivesTypesUnpacking();
  str.clear();
  for(auto it = formats.begin(); it != formats.end(); it++)
    {
      if(!str.isEmpty())
        {
          str += ", ";
        }
      str += it->c_str();
      if(*it == "rar")
        {
          str += " (";
          str += tr("rar - only reading");
          str += ")";
        }
    }

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setWordWrap(true);
  lab->setText(str);
  h_box->addWidget(lab);

  h_box->addStretch();

  h_box = new QHBoxLayout;
  v_box->addLayout(h_box);

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Author:"));
  h_box->addWidget(lab);

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Yury Bobylev") + " <bobilev_yury@mail.ru>");
  lab->setTextInteractionFlags(Qt::TextSelectableByMouse
                               | Qt::TextSelectableByKeyboard);
  h_box->addWidget(lab);

  h_box->addStretch();

  h_box = new QHBoxLayout;
  v_box->addLayout(h_box);

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Icon design:"));
  h_box->addWidget(lab);

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText("Felix <f11091877@gmail.com>");
  lab->setTextInteractionFlags(Qt::TextSelectableByMouse
                               | Qt::TextSelectableByKeyboard);
  h_box->addWidget(lab);

  h_box->addStretch();

  QFrame *h_line = new QFrame;
  h_line->setFrameShape(QFrame::HLine);
  v_box->addWidget(h_line);

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("MyLibrary uses following libraries:"));
  v_box->addWidget(lab, 0, Qt::AlignLeft);

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText("Qt (" + tr("see About Qt for details") + ")");
  v_box->addWidget(lab, 0, Qt::AlignCenter);

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setTextInteractionFlags(Qt::TextSelectableByMouse
                               | Qt::TextSelectableByKeyboard);
  lab->setText("LibUDB https://github.com/ProfessorNavigator/libudb");
  v_box->addWidget(lab, 0, Qt::AlignCenter);

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setTextInteractionFlags(Qt::TextSelectableByMouse
                               | Qt::TextSelectableByKeyboard);
  lab->setText("ICU https://icu.unicode.org");
  v_box->addWidget(lab, 0, Qt::AlignCenter);

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setTextInteractionFlags(Qt::TextSelectableByMouse
                               | Qt::TextSelectableByKeyboard);
  lab->setText("Libgcrypt https://gnupg.org/software/libgcrypt/");
  v_box->addWidget(lab, 0, Qt::AlignCenter);

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setTextInteractionFlags(Qt::TextSelectableByMouse
                               | Qt::TextSelectableByKeyboard);
  lab->setText("Libgpg-error https://www.gnupg.org/software/libgpg-error/");
  v_box->addWidget(lab, 0, Qt::AlignCenter);

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setTextInteractionFlags(Qt::TextSelectableByMouse
                               | Qt::TextSelectableByKeyboard);
  lab->setText("libarchive https://libarchive.org/");
  v_box->addWidget(lab, 0, Qt::AlignCenter);

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setTextInteractionFlags(Qt::TextSelectableByMouse
                               | Qt::TextSelectableByKeyboard);
  lab->setText("Poppler https://poppler.freedesktop.org/");
  v_box->addWidget(lab, 0, Qt::AlignCenter);

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setTextInteractionFlags(Qt::TextSelectableByMouse
                               | Qt::TextSelectableByKeyboard);
  lab->setText("DjVuLibre https://djvu.sourceforge.net/");
  v_box->addWidget(lab, 0, Qt::AlignCenter);

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setTextInteractionFlags(Qt::TextSelectableByMouse
                               | Qt::TextSelectableByKeyboard);
  lab->setText("Magick++ https://imagemagick.org/magick++/");
  v_box->addWidget(lab, 0, Qt::AlignCenter);

  scrl->setWidget(viewport);

  QPushButton *close = new QPushButton;
  close->setText(tr("Close"));
  QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(close);
  shadow->setBlurRadius(12);
  shadow->setOffset(0, 2);
  shadow->setColor(QColor(0, 0, 0, 120));
  close->setGraphicsEffect(shadow);
  close->setObjectName("ApplyButton");
  connect(close, &QPushButton::clicked, this, &AboutWindow::close);
  main_box->addWidget(close, 0, Qt::AlignCenter);
}

void
AboutWindow::paintEvent(QPaintEvent *event)
{
  QStyleOption opt;
  opt.initFrom(this);
  QPainter p(this);
  style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
