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

#include <ByteOrder.h>
#include <CoverWidget.h>
#include <CoverWindow.h>
#include <Magick++.h>
#include <QDir>
#include <QFileDialog>
#include <QGraphicsDropShadowEffect>
#include <QLabel>
#include <QMenu>
#include <QPainter>
#include <QPushButton>
#include <QScreen>
#include <QStringList>
#include <QTextCursor>
#include <QTextDocument>
#include <QVBoxLayout>
#include <QWindow>
#include <StyledWindow.h>
#include <algorithm>
#include <fstream>
#include <iostream>

CoverWidget::CoverWidget(
    QWidget *parent,
    const std::shared_ptr<FormatAnnotation> &format_annotation)
    : QWidget(parent)
{
  this->format_annotation = format_annotation;
  uint32_t val;
  unsigned char *ptr = reinterpret_cast<unsigned char *>(&val);
  for(unsigned char i = 0; i < sizeof(val); i++)
    {
      ptr[i] = i;
    }

  ByteOrder bo;
  bo.setLittle(val);

  uint32_t check = bo;
  if(check != val)
    {
      need_byte_conversion = true;
    }

  this->setContextMenuPolicy(Qt::CustomContextMenu);

  connect(this, &CoverWidget::customContextMenuRequested,
          [this](const QPoint &pos)
            {
              QMenu *menu = new QMenu(this);
              menu->setAttribute(Qt::WA_DeleteOnClose);
              menu->setObjectName("Menu");
              QList<QAction *> actions = this->actions();
              menu->addActions(actions);
              menu->popup(this->mapToGlobal(pos));
            });
}

CoverWidget::~CoverWidget()
{
  for(size_t i = 0; i < act_list.size(); i++)
    {
      delete act_list[i];
    }
}

void
CoverWidget::setCover(const UDBElement &cover_obj)
{
  clearCover();

  if(cover_obj.content.empty())
    {
      return void();
    }

  std::vector<UDBElement>::const_iterator it = std::find_if(
      cover_obj.subelements.begin(), cover_obj.subelements.end(),
      [this](const UDBElement &el)
        {
          return bid.getId(el) == BaseID::CoverType;
        });
  if(it == cover_obj.subelements.end())
    {
      return void();
    }

  if(it->content == "base64" || it->content == "image")
    {
      Magick::Blob blob;
      if(it->content == "base64")
        {
          blob.base64(cover_obj.content);
        }
      else
        {
          blob.update(cover_obj.content.c_str(), cover_obj.content.size());
        }
      Magick::Image image;
      size_t buf_sz;
      unsigned char *buf = nullptr;
      try
        {
          image.read(blob);
          buf_sz = image.columns() * image.rows() * 4;
          buf = new unsigned char[buf_sz];
          image.write(0, 0, image.columns(), image.rows(), "RGBA",
                      Magick::CharPixel, buf);
        }
      catch(Magick::Exception &er)
        {
          std::cout << "CoverWidget::setCover: \"" << er.what() << "\""
                    << std::endl;
          delete[] buf;
          return void();
        }

      if(need_byte_conversion)
        {
#pragma omp parallel for
          for(size_t i = 0; i < buf_sz; i += 4)
            {
              uint32_t val;
              unsigned char *ptr = reinterpret_cast<unsigned char *>(&val);
              for(size_t j = 0; j < 4; j++)
                {
                  ptr[j] = buf[i + j];
                }
              ByteOrder bo;
              bo.setLittle(val);
              val = bo;
              for(size_t j = 0; j < 4; j++)
                {
                  buf[i + j] = ptr[j];
                }
            }
        }
      original_image = QImage(
          buf, static_cast<int>(image.columns()),
          static_cast<int>(image.rows()), QImage::Format_RGBA8888,
          [](void *info)
            {
              unsigned char *buf = reinterpret_cast<unsigned char *>(info);
              delete[] buf;
            },
          buf);
    }
  else if(it->content == "text" || it->content == "txt" || it->content == "md")
    {
      QScreen *screen
          = this->parentWidget()->window()->windowHandle()->screen();
      QSize pixels = screen->size();
      QSizeF mm = screen->physicalSize();
      qreal h_pixels_per_mm = static_cast<qreal>(pixels.width()) / mm.width();
      qreal v_pixels_per_mm
          = static_cast<qreal>(pixels.height()) / mm.height();
      int w = static_cast<int>(210.0 * h_pixels_per_mm);
      int h = static_cast<int>(297.0 * v_pixels_per_mm);
      QSize a4_pixels(w, h);
      original_image = QImage(a4_pixels, QImage::Format_ARGB32);
      original_image.fill(Qt::white);

      std::string txt = cover_obj.content;
      if(it->content == "text")
        {
          format_annotation->replaceTags(txt);
          format_annotation->removeEscapeSequences(txt);
          format_annotation->finalCleaning(txt);
        }
      else if(it->content == "txt")
        {
          std::string find_str("\n");
          std::string repl("<br>");
          std::string::size_type n = 0;
          for(;;)
            {
              n = txt.find(find_str, n);
              if(n != std::string::npos)
                {
                  txt.erase(txt.begin() + n,
                            txt.begin() + n + find_str.size());
                  txt.insert(txt.begin() + n, repl.begin(), repl.end());
                }
              else
                {
                  break;
                }
            }
        }
      QTextDocument doc;
      doc.setPageSize(QSizeF(a4_pixels));
      QTextCursor cursor(&doc);
      QTextBlockFormat format;
      format.setAlignment(Qt::AlignJustify);
      format.setLeftMargin(20.0 * h_pixels_per_mm);
      format.setRightMargin(20.0 * h_pixels_per_mm);
      cursor.insertBlock(format);
      if(it->content == "md")
        {
          cursor.insertMarkdown(txt.c_str());
        }
      else
        {
          cursor.insertHtml(txt.c_str());
        }

      QPainter painter;
      if(painter.begin(&original_image))
        {
          doc.drawContents(&painter);
          painter.end();
        }
      else
        {
          original_image = QImage();
        }
    }
  else if(it->content == "ARGB" || it->content == "RGB")
    {
      int w = 0, h = 0;
      auto it_val
          = std::find_if(it->subelements.begin(), it->subelements.end(),
                         [this](const UDBElement &el)
                           {
                             return bid.getId(el) == BaseID::CoverHeight;
                           });
      if(it_val != it->subelements.end())
        {
          if(it_val->content.size() == sizeof(int))
            {
              char *ptr = reinterpret_cast<char *>(&h);
              for(size_t i = 0; i < it_val->content.size(); i++)
                {
                  ptr[i] = it_val->content[i];
                }
            }
        }

      it_val = std::find_if(it->subelements.begin(), it->subelements.end(),
                            [this](const UDBElement &el)
                              {
                                return bid.getId(el) == BaseID::CoverWidth;
                              });
      if(it_val != it->subelements.end())
        {
          if(it_val->content.size() == sizeof(int))
            {
              char *ptr = reinterpret_cast<char *>(&w);
              for(size_t i = 0; i < it_val->content.size(); i++)
                {
                  ptr[i] = it_val->content[i];
                }
            }
        }
      if(w > 0 && h > 0)
        {
          size_t buf_sz = cover_obj.content.size();
          const unsigned char *src = reinterpret_cast<const unsigned char *>(
              cover_obj.content.c_str());
          unsigned char *buf = new unsigned char[buf_sz];

          for(size_t i = 0; i < buf_sz; i++)
            {
              buf[i] = src[i];
            }
          if(it->content == "ARGB")
            {
              original_image = QImage(
                  buf, w, h, QImage::Format_ARGB32,
                  [](void *info)
                    {
                      unsigned char *buf
                          = reinterpret_cast<unsigned char *>(info);
                      delete[] buf;
                    },
                  buf);
            }
          else
            {
              original_image = QImage(
                  buf, w, h, QImage::Format_RGBX8888,
                  [](void *info)
                    {
                      unsigned char *buf
                          = reinterpret_cast<unsigned char *>(info);
                      delete[] buf;
                    },
                  buf);
            }
        }
    }
  if(!original_image.isNull())
    {
      cover = original_image.scaled(current_size, Qt::KeepAspectRatio,
                                    Qt::SmoothTransformation);
      QAction *act = new QAction(tr("Save as..."));
      connect(act, &QAction::triggered, this, &CoverWidget::saveImageDialog);
      this->addAction(act);
      act_list.push_back(act);
    }
}

void
CoverWidget::clearCover()
{
  original_image = QImage();
  cover = QImage();
  for(size_t i = 0; i < act_list.size(); i++)
    {
      delete act_list[i];
    }
  act_list.clear();
  this->update();
}

void
CoverWidget::paintEvent(QPaintEvent *event)
{
  if(!original_image.isNull())
    {
      QRect rect = event->rect();
      if(cover.height() != rect.height() || cover.width() != rect.width())
        {
          current_size = event->rect().size();
          cover = original_image.scaled(current_size, Qt::KeepAspectRatio,
                                        Qt::SmoothTransformation);
        }
      QPoint point;
      if(rect.width() > cover.width())
        {
          point.setX((rect.width() - cover.width()) / 2);
        }
      if(rect.height() > cover.height())
        {
          point.setY((rect.height() - cover.height()) / 2);
        }
      QPainter painter;
      if(painter.begin(this))
        {
          painter.drawImage(point, cover);
          painter.end();
        }
    }
  else
    {
      current_size = event->rect().size();
    }
}

void
CoverWidget::mousePressEvent(QMouseEvent *event)
{
  if(event->button() == Qt::LeftButton && !original_image.isNull())
    {
      CoverWindow *win = new CoverWindow(this->window(), original_image);
      win->createWindow();
      win->show();
    }
  QWidget::mousePressEvent(event);
}

void
CoverWidget::saveImageDialog()
{
  if(original_image.isNull())
    {
      return void();
    }

  QFileDialog *fd = new QFileDialog(this->window());
  fd->setAttribute(Qt::WA_DeleteOnClose);
  fd->setWindowModality(Qt::WindowModal);

  fd->setAcceptMode(QFileDialog::AcceptSave);
  std::vector<Magick::CoderInfo> list;
  try
    {
      Magick::coderInfoList(&list, Magick::CoderInfo::AnyMatch,
                            Magick::CoderInfo::TrueMatch,
                            Magick::CoderInfo::AnyMatch);
    }
  catch(Magick::Exception &er)
    {
      std::cout << "CoverWidget::saveImageDialog: \"" << er.what() << "\""
                << std::endl;
    }

  QStringList filters;
  std::string find_str("jpeg");
  QString default_filter;
  std::string::size_type n;
  for(auto it = list.begin(); it != list.end(); it++)
    {
      std::string mime = it->mimeType();
      if(mime.empty())
        {
          continue;
        }

      filters.append(mime.c_str());
      n = mime.find(find_str);
      if(n != std::string::npos)
        {
          default_filter = mime.c_str();
        }
    }
  filters.removeDuplicates();

  std::shared_ptr<QString> default_suffix(new QString);

  connect(fd, &QFileDialog::filterSelected,
          [fd, default_suffix](const QString &filter)
            {
              QString local = filter;
              QString search = " ";
              qsizetype n = local.indexOf(search);
              if(n >= 0)
                {
                  local.erase(local.begin() + n, local.end());
                }
              search = "*.";
              n = local.indexOf(search);
              if(n >= 0)
                {
                  local.erase(local.begin(),
                              local.begin() + n + search.size());
                }
              if(!local.isEmpty())
                {
                  *default_suffix = local;
                  fd->setDefaultSuffix(local);
                }
            });

  fd->setMimeTypeFilters(filters);
  fd->selectMimeTypeFilter(default_filter);
  fd->setDirectory(QDir::homePath());
  *default_suffix = find_str.c_str();
  fd->setDefaultSuffix(default_suffix->toLower());
  fd->selectFile("Cover");

  connect(fd, &QFileDialog::fileSelected, this,
          [this, default_suffix](const QString &file)
            {
              std::string str = file.toStdString();
              std::filesystem::path p = std::u8string(str.begin(), str.end());
              str = "." + default_suffix->toLower().toStdString();
              p.replace_extension(std::u8string(str.begin(), str.end()));
              saveImage(p);
            });

  fd->show();
}

void
CoverWidget::saveImage(const std::filesystem::path &result)
{
  QImage loc = original_image.convertToFormat(QImage::Format_RGBA8888);

  qsizetype sz = loc.sizeInBytes();
  if(sz <= 0 || sz % 4 != 0)
    {
      return void();
    }
  unsigned char *src = loc.bits();
  Magick::Image img;
  if(need_byte_conversion)
    {
      std::vector<unsigned char> buf;
      buf.resize(static_cast<size_t>(sz));
#pragma omp parallel for
      for(qsizetype i = 0; i < sz; i += 4)
        {
          uint32_t val;
          unsigned char *ptr = reinterpret_cast<unsigned char *>(&val);
          for(qsizetype j = 0; j < 4; j++)
            {
              ptr[j] = src[i + j];
            }
          ByteOrder bo(val);
          bo.getLittle(val);
          for(qsizetype j = 0; j < 4; j++)
            {
              buf[i + j] = ptr[j];
            }
        }
      try
        {
          img.read(static_cast<size_t>(loc.width()),
                   static_cast<size_t>(loc.height()), "RGBA",
                   Magick::CharPixel, buf.data());
        }
      catch(Magick::Exception &er)
        {
          std::cout << "CoverWidget::saveImage: \"" << er.what() << "\""
                    << std::endl;
          errorDialog(er.what());
          return void();
        }
    }
  else
    {
      try
        {
          img.read(static_cast<size_t>(loc.width()),
                   static_cast<size_t>(loc.height()), "RGBA",
                   Magick::CharPixel, src);
        }
      catch(Magick::Exception &er)
        {
          std::cout << "CoverWidget::saveImage: \"" << er.what() << "\""
                    << std::endl;
          errorDialog(er.what());
          return void();
        }
    }

  std::filesystem::remove_all(result);
  std::u8string u8str = result.extension().u8string();
  std::u8string find_str(u8".");
  std::u8string::size_type n = u8str.rfind(find_str);
  if(n != std::u8string::npos)
    {
      u8str.erase(u8str.begin() + n, u8str.begin() + n + find_str.size());
    }
  std::string str(u8str.begin(), u8str.end());

  Magick::Blob blob;
  try
    {
      img.write(&blob, str);
    }
  catch(Magick::Exception &er)
    {
      std::cout << "CoverWidget::saveImage: \"" << er.what() << "\""
                << std::endl;
      errorDialog(er.what());
      return void();
    }

  std::fstream f;
  f.open(result, std::ios_base::out | std::ios_base::binary);
  if(f.is_open())
    {
      f.write(reinterpret_cast<const char *>(blob.data()), blob.length());
      f.close();
    }
}

void
CoverWidget::errorDialog(const std::string &er)
{
  StyledWindow *window = new StyledWindow(this->window());
  window->setWindowModality(Qt::WindowModal);
  window->setObjectName("Window");

  QVBoxLayout *v_box = new QVBoxLayout;
  window->setLayout(v_box);

  QLabel *lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Error on cover saving!"));
  v_box->addWidget(lab, 0, Qt::AlignCenter);

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setWordWrap(true);
  lab->setText(er.c_str());
  v_box->addWidget(lab);

  QPushButton *close = new QPushButton;
  close->setText(tr("Close"));
  QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(close);
  shadow->setBlurRadius(12);
  shadow->setOffset(0, 2);
  shadow->setColor(QColor(0, 0, 0, 120));
  close->setGraphicsEffect(shadow);
  close->setObjectName("ApplyButton");
  connect(close, &QPushButton::clicked, window, &StyledWindow::close);
  v_box->addWidget(close, 0, Qt::AlignCenter);

  window->show();
}
