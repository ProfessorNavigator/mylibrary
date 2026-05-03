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

#include <NoteWindow.h>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QScreen>
#include <QStyleOption>
#include <QVBoxLayout>
#include <StyledWindow.h>
#include <algorithm>
#include <fstream>
#include <iostream>

NoteWindow::NoteWindow(QWidget *parent, const UDBElement &book_search_result,
                       const std::shared_ptr<NotesKeeper> &notes)
    : QWidget(parent)
{
  this->book_search_result = book_search_result;
  this->notes = notes;

  this->setAttribute(Qt::WA_DeleteOnClose);
  this->setWindowTitle(tr("Notes"));
  this->setWindowFlag(Qt::Window, true);
  this->setWindowModality(Qt::WindowModality::WindowModal);

  this->setObjectName("Window");
}

void
NoteWindow::createWindow()
{
  QVBoxLayout *v_box = new QVBoxLayout;
  this->setLayout(v_box);

  QHBoxLayout *h_box = new QHBoxLayout;
  v_box->addLayout(h_box);

  QLabel *lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("File:"));
  h_box->addWidget(lab, 0, Qt::AlignLeft);

  auto it_fl = std::find_if(book_search_result.subelements.begin(),
                            book_search_result.subelements.end(),
                            [this](const UDBElement &el)
                              {
                                return bid.getId(el) == BaseID::File;
                              });
  if(it_fl == book_search_result.subelements.end())
    {
      this->deleteLater();
      return void();
    }

  lab = new QLabel;
  lab->setObjectName("Label");
  QFont font = lab->font();
  font.setItalic(true);
  lab->setFont(font);
  lab->setText(it_fl->content.c_str());
  lab->setTextInteractionFlags(Qt::TextSelectableByMouse
                               | Qt::TextSelectableByKeyboard);
  h_box->addWidget(lab, 0, Qt::AlignLeft);

  h_box->addStretch();

  auto it_book = std::find_if(book_search_result.subelements.begin(),
                              book_search_result.subelements.end(),
                              [this](const UDBElement &el)
                                {
                                  return bid.getId(el) == BaseID::Book;
                                });
  if(it_book == book_search_result.subelements.end())
    {
      this->deleteLater();
      return void();
    }

  auto it_path
      = std::find_if(it_book->subelements.begin(), it_book->subelements.end(),
                     [this](const UDBElement &el)
                       {
                         return bid.getId(el) == BaseID::PathInFile;
                       });
  if(it_path != it_book->subelements.end())
    {
      h_box = new QHBoxLayout;
      v_box->addLayout(h_box);

      lab = new QLabel;
      lab->setObjectName("Label");
      lab->setText(tr("Path in file:"));
      h_box->addWidget(lab, 0, Qt::AlignLeft);

      QString str = it_path->content.c_str();
      while(it_path->subelements.size() > 0)
        {
          auto it_p = std::find_if(
              it_path->subelements.begin(), it_path->subelements.end(),
              [this](const UDBElement &el)
                {
                  return bid.getId(el) == BaseID::PathInFile;
                });
          if(it_p == it_path->subelements.end())
            {
              break;
            }
          else
            {
              str += "/";
              str += it_p->content.c_str();
              it_path = it_p;
            }
        }

      lab = new QLabel;
      lab->setObjectName("Label");
      font = lab->font();
      font.setItalic(true);
      lab->setFont(font);
      lab->setText(str);
      h_box->addWidget(lab, 0, Qt::AlignLeft);

      h_box->addStretch();
    }

  QString txt;
  UDBase notes_base = notes->searchNotes(book_search_result);
  std::vector<UDBElement> *raw_base = notes_base.getRawBase();
  std::string find_str("\n\n");

  for(auto it = raw_base->begin(); it != raw_base->end(); it++)
    {
      auto it_nf
          = std::find_if(it->subelements.begin(), it->subelements.end(),
                         [this](const UDBElement &el)
                           {
                             return bid.getId(el) == BaseID::BookNoteFile;
                           });
      if(it_nf == it->subelements.end())
        {
          continue;
        }
      std::filesystem::path p
          = std::u8string(it_nf->content.begin(), it_nf->content.end());
      std::fstream f;
      f.open(p, std::ios_base::in | std::ios_base::binary);
      if(!f.is_open())
        {
          continue;
        }
      std::string buf;
      f.seekg(0, std::ios_base::end);
      buf.resize(static_cast<size_t>(f.tellg()));
      f.seekg(0, std::ios_base::beg);
      f.read(buf.data(), buf.size());
      f.close();

      std::string::size_type n = buf.find(find_str);
      if(n != std::string::npos)
        {
          buf.erase(0, n + find_str.size());
        }
      if(!txt.isEmpty())
        {
          txt += "\n\n";
        }
      txt += buf.c_str();
    }

  note_txt = new QTextEdit;
  note_txt->setObjectName("TextEdit");
  note_txt->setText(txt);
  v_box->addWidget(note_txt);

  h_box = new QHBoxLayout;
  v_box->addLayout(h_box);

  QPushButton *save = new QPushButton;
  save->setText(tr("Save"));
  QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(save);
  shadow->setBlurRadius(12);
  shadow->setOffset(0, 2);
  shadow->setColor(QColor(0, 0, 0, 120));
  save->setGraphicsEffect(shadow);
  save->setObjectName("ApplyButton");
  connect(save, &QPushButton::clicked, this, &NoteWindow::saveDialog);
  h_box->addWidget(save, 0, Qt::AlignCenter);

  QPushButton *close = new QPushButton;
  close->setText(tr("Close"));
  shadow = new QGraphicsDropShadowEffect(close);
  shadow->setBlurRadius(12);
  shadow->setOffset(0, 2);
  shadow->setColor(QColor(0, 0, 0, 120));
  close->setGraphicsEffect(shadow);
  close->setObjectName("CancelButton");
  connect(close, &QPushButton::clicked, this, &QWidget::close);
  h_box->addWidget(close, 0, Qt::AlignCenter);

  QScreen *screen = this->parentWidget()->screen();
  QSize av_sz = screen->availableSize();
  av_sz.setHeight(av_sz.height() * 0.7);
  av_sz.setWidth(av_sz.width() * 0.7);
  this->resize(av_sz);
}

void
NoteWindow::saveDialog()
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
          [window, this]
            {
              QString txt = note_txt->toPlainText();
              try
                {
                  notes->addNote(book_search_result, txt.toStdString());
                }
              catch(std::exception &er)
                {
                  std::cout << "NoteWindow::saveDialog: \"" << er.what()
                            << "\"" << std::endl;
                }
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
NoteWindow::paintEvent(QPaintEvent *event)
{
  QStyleOption opt;
  opt.initFrom(this);
  QPainter p(this);
  style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
