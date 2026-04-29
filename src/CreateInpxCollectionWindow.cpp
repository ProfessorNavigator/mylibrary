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

#include <CreateCollection.h>
#include <CreateInpxCollectionWindow.h>
#include <MainWindow.h>
#include <QApplication>
#include <QDir>
#include <QFileDialog>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QScreen>
#include <QStyleOption>
#include <QVBoxLayout>
#include <StyledWindow.h>
#include <filesystem>

CreateInpxCollectionWindow::CreateInpxCollectionWindow(
    QWidget *parent, const std::shared_ptr<MLBookProc> &mlbp)
    : QWidget(parent)
{
  this->mlbp = mlbp;

  this->setWindowTitle(tr("Collection creation"));
  this->setAttribute(Qt::WA_DeleteOnClose);
  this->setWindowFlag(Qt::Window, true);
  this->setWindowModality(Qt::WindowModality::WindowModal);

  this->setObjectName("Window");
}

void
CreateInpxCollectionWindow::createWindow()
{
  QVBoxLayout *v_box = new QVBoxLayout;
  this->setLayout(v_box);

  QLabel *lab = new QLabel;
  lab->setObjectName("Label");
  QFont font = lab->font();
  font.setBold(true);
  lab->setFont(font);
  lab->setText(tr("Inpx collection creation"));
  v_box->addWidget(lab, 0, Qt::AlignCenter);

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Collection name") + ":");
  v_box->addWidget(lab, 0, Qt::AlignLeft);

  collection_name = new QLineEdit;
  collection_name->setObjectName("LineEdit");
  v_box->addWidget(collection_name);

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Path to inpx file") + ":");
  v_box->addWidget(lab, 0, Qt::AlignLeft);

  inpx_path = new QLineEdit;
  inpx_path->setObjectName("LineEdit");
  v_box->addWidget(inpx_path);

  QPushButton *open = new QPushButton;
  open->setText(tr("Open"));
  QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(open);
  shadow->setBlurRadius(12);
  shadow->setOffset(0, 2);
  shadow->setColor(QColor(0, 0, 0, 120));
  open->setGraphicsEffect(shadow);
  open->setObjectName("ApplyButton");
  connect(open, &QPushButton::clicked, this,
          &CreateInpxCollectionWindow::openInpxDialog);
  v_box->addWidget(open, 0, Qt::AlignRight);

  QHBoxLayout *h_box = new QHBoxLayout;
  v_box->addLayout(h_box);

  QPushButton *create = new QPushButton;
  create->setText(tr("Create collection"));
  shadow = new QGraphicsDropShadowEffect(create);
  shadow->setBlurRadius(12);
  shadow->setOffset(0, 2);
  shadow->setColor(QColor(0, 0, 0, 120));
  create->setGraphicsEffect(shadow);
  create->setObjectName("ApplyButton");
  connect(create, &QPushButton::clicked, this,
          &CreateInpxCollectionWindow::checkInput);
  h_box->addWidget(create, 0, Qt::AlignCenter);

  QPushButton *cancel = new QPushButton;
  cancel->setText(tr("Cancel"));
  shadow = new QGraphicsDropShadowEffect(cancel);
  shadow->setBlurRadius(12);
  shadow->setOffset(0, 2);
  shadow->setColor(QColor(0, 0, 0, 120));
  cancel->setGraphicsEffect(shadow);
  cancel->setObjectName("CancelButton");
  connect(cancel, &QPushButton::clicked, this,
          &CreateInpxCollectionWindow::close);
  h_box->addWidget(cancel, 0, Qt::AlignCenter);
}

void
CreateInpxCollectionWindow::showEvent(QShowEvent *event)
{
  QScreen *screen = this->screen();
  QSize av_sz = screen->availableSize();
  QSize sz = this->size();
  if(sz.width() < av_sz.width() * 0.5)
    {
      sz.setWidth(av_sz.width() * 0.5);
      this->resize(sz);
    }

  QWidget::showEvent(event);
}

void
CreateInpxCollectionWindow::openInpxDialog()
{
  QFileDialog *fd = new QFileDialog(this);
  fd->setAttribute(Qt::WA_DeleteOnClose);
  fd->setWindowModality(Qt::WindowModal);

  fd->setAcceptMode(QFileDialog::AcceptOpen);
  fd->setDirectory(QDir::homePath());
  fd->setFileMode(QFileDialog::ExistingFile);
  fd->setNameFilter("*.inpx");

  connect(fd, &QFileDialog::fileSelected, this,
          [this](const QString &path)
            {
              inpx_path->setText(path);
            });

  fd->show();
}

void
CreateInpxCollectionWindow::checkInput()
{
  QString str = collection_name->text();
  if(str.isEmpty())
    {
      errorDialog(Error::CollectionNameEmpty);
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
      errorDialog(Error::CollectionExists);
      return void();
    }

  str = inpx_path->text();
  if(str.isEmpty())
    {
      errorDialog(Error::InpxEmpty);
      return void();
    }

  l_str = str.toStdString();
  std::filesystem::path path_to_inpx
      = std::u8string(l_str.begin(), l_str.end());
  if(!std::filesystem::exists(path_to_inpx)
     || path_to_inpx.extension().u8string() != u8".inpx")
    {
      errorDialog(Error::IncorrectInpx);
      return void();
    }

  std::unique_ptr<CreateCollection> cr_col(new CreateCollection(mlbp));
  try
    {
      cr_col->createInpxCollection(path_to_inpx, base_path);
    }
  catch(std::exception &er)
    {
      str = er.what();
      errorDialog(Error::CreationError, str);
      return void();
    }
  QWidgetList list = qApp->topLevelWidgets();
  for(qsizetype i = 0; i < list.size(); i++)
    {
      MainWindow *mw = dynamic_cast<MainWindow *>(list[i]);
      if(mw != nullptr)
        {
          emit mw->signalCollectionCreated(base_path);
          break;
        }
    }
  str = base_path.parent_path().filename().u8string().c_str();
  errorDialog(Error::Success, str);
}

void
CreateInpxCollectionWindow::errorDialog(const Error &er, const QString &det)
{
  StyledWindow *window = new StyledWindow(this);
  window->setWindowModality(Qt::WindowModal);
  window->setObjectName("Window");

  QVBoxLayout *v_box = new QVBoxLayout;
  window->setLayout(v_box);

  switch(er)
    {
    case Error::CollectionNameEmpty:
      {
        QLabel *lab = new QLabel;
        lab->setObjectName("Label");
        lab->setText(tr("Collection name is empty!"));
        v_box->addWidget(lab);
        break;
      }
    case Error::CollectionExists:
      {
        QLabel *lab = new QLabel;
        lab->setObjectName("Label");
        lab->setText(tr("Collection already exists!"));
        v_box->addWidget(lab);
        break;
      }
    case Error::InpxEmpty:
      {
        QLabel *lab = new QLabel;
        lab->setObjectName("Label");
        lab->setText(tr("Path to inpx file is empty!"));
        v_box->addWidget(lab);
        break;
      }
    case Error::IncorrectInpx:
      {
        QLabel *lab = new QLabel;
        lab->setObjectName("Label");
        lab->setText(tr("Incorrect inpx file!"));
        v_box->addWidget(lab);
        break;
      }
    case Error::CreationError:
      {
        QLabel *lab = new QLabel;
        lab->setObjectName("Label");
        lab->setText(tr("Collection creation error!"));
        v_box->addWidget(lab, 0, Qt::AlignCenter);

        lab = new QLabel;
        lab->setObjectName("Label");
        lab->setText(det);
        v_box->addWidget(lab, 0, Qt::AlignCenter);
        break;
      }
    case Error::Success:
      {
        QLabel *lab = new QLabel;
        lab->setObjectName("Label");
        lab->setText(tr("Collection has been created!"));
        v_box->addWidget(lab, 0, Qt::AlignCenter);

        QHBoxLayout *h_box = new QHBoxLayout;
        v_box->addLayout(h_box);

        h_box->addStretch();

        lab = new QLabel;
        lab->setObjectName("Label");
        lab->setText(tr("Collection") + ":");
        h_box->addWidget(lab);

        lab = new QLabel;
        QFont font = lab->font();
        font.setItalic(true);
        lab->setFont(font);
        lab->setObjectName("Label");
        lab->setText(det);
        h_box->addWidget(lab);

        h_box->addStretch();

        connect(window, &StyledWindow::destroyed, this,
                &CreateInpxCollectionWindow::close);
        this->setVisible(false);
        break;
      }
    default:
      break;
    }

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

void
CreateInpxCollectionWindow::paintEvent(QPaintEvent *event)
{
  QStyleOption opt;
  opt.initFrom(this);
  QPainter p(this);
  style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
