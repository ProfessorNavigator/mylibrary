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

#include <CollectionBaseExportWindow.h>
#include <QFileDialog>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QScreen>
#include <QStyleOption>
#include <QVBoxLayout>
#include <StyledWindow.h>

CollectionBaseExportWindow::CollectionBaseExportWindow(
    QWidget *parent, const std::shared_ptr<BaseKeeper> &base_keeper)
    : QWidget(parent)
{
  this->base_keeper = base_keeper;

  this->setWindowFlag(Qt::Window, true);
  this->setWindowTitle(tr("Collection export"));
  this->setAttribute(Qt::WA_DeleteOnClose);
  this->setWindowModality(Qt::WindowModal);

  this->setObjectName("Window");
}

void
CollectionBaseExportWindow::showWindow()
{
  std::filesystem::path base_path
      = base_keeper->getCurrentCollectionBasePath();
  if(base_path.empty() || !std::filesystem::exists(base_path))
    {
      this->deleteLater();
      return void();
    }
  QVBoxLayout *v_box = new QVBoxLayout;
  this->setLayout(v_box);

  QHBoxLayout *h_box = new QHBoxLayout;
  v_box->addLayout(h_box);

  h_box->addStretch();

  QLabel *lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Collection to export:"));
  h_box->addWidget(lab, 0, Qt::AlignRight);

  lab = new QLabel;
  lab->setObjectName("Label");
  QFont font = lab->font();
  font.setItalic(true);
  lab->setFont(font);
  lab->setText(base_path.parent_path().filename().u8string().c_str());
  h_box->addWidget(lab, 0, Qt::AlignLeft);

  h_box->addStretch();

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Path to result:"));
  v_box->addWidget(lab, 0, Qt::AlignLeft);

  result_path = new QLineEdit;
  result_path->setObjectName("LineEdit");
  v_box->addWidget(result_path);

  QPushButton *save_as = new QPushButton;
  save_as->setText(tr("Save as..."));
  QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(save_as);
  shadow->setBlurRadius(12);
  shadow->setOffset(0, 2);
  shadow->setColor(QColor(0, 0, 0, 120));
  save_as->setGraphicsEffect(shadow);
  save_as->setObjectName("ApplyButton");
  connect(save_as, &QPushButton::clicked, this,
          &CollectionBaseExportWindow::saveAsDialog);
  v_box->addWidget(save_as, 0, Qt::AlignRight);

  h_box = new QHBoxLayout;
  v_box->addLayout(h_box);

  QPushButton *export_col = new QPushButton;
  export_col->setText(tr("Export"));
  shadow = new QGraphicsDropShadowEffect(export_col);
  shadow->setBlurRadius(12);
  shadow->setOffset(0, 2);
  shadow->setColor(QColor(0, 0, 0, 120));
  export_col->setGraphicsEffect(shadow);
  export_col->setObjectName("ApplyButton");
  connect(export_col, &QPushButton::clicked, this,
          &CollectionBaseExportWindow::exportBase);
  h_box->addWidget(export_col, 0, Qt::AlignCenter);

  QPushButton *cancel = new QPushButton;
  cancel->setText(tr("Cancel"));
  shadow = new QGraphicsDropShadowEffect(cancel);
  shadow->setBlurRadius(12);
  shadow->setOffset(0, 2);
  shadow->setColor(QColor(0, 0, 0, 120));
  cancel->setGraphicsEffect(shadow);
  cancel->setObjectName("CancelButton");
  connect(cancel, &QPushButton::clicked, this,
          &CollectionBaseExportWindow::close);
  h_box->addWidget(cancel, 0, Qt::AlignCenter);

  QScreen *screen = this->parentWidget()->screen();
  QSize av_sz = screen->availableSize();
  av_sz.setHeight(this->sizeHint().height());
  av_sz.setWidth(av_sz.width() * 0.5);
  this->resize(av_sz);

  this->show();
}

void
CollectionBaseExportWindow::saveAsDialog()
{
  QFileDialog *fd = new QFileDialog(this);
  fd->setAttribute(Qt::WA_DeleteOnClose);
  fd->setWindowModality(Qt::WindowModal);

  fd->selectFile("base");
  fd->setAcceptMode(QFileDialog::AcceptSave);
  fd->setDirectory(QDir::homePath());

  connect(fd, &QFileDialog::fileSelected, this,
          [this](const QString &fp)
            {
              result_path->setText(fp);
            });

  fd->show();
}

void
CollectionBaseExportWindow::exportBase()
{
  QString str = result_path->text();
  if(str.isEmpty())
    {
      errorDialog(Errors::EmptyPath, "");
      return void();
    }
  std::string l_str = str.toStdString();
  std::filesystem::path p = std::u8string(l_str.begin(), l_str.end());
  try
    {
      base_keeper->exportBase(p);
      errorDialog(Errors::Success, "");
    }
  catch(std::exception &er)
    {
      errorDialog(Errors::ErrorSaving, er.what());
    }
}

void
CollectionBaseExportWindow::errorDialog(const Errors &er, const QString &txt)
{
  StyledWindow *window = nullptr;

  QVBoxLayout *v_box = new QVBoxLayout;
  switch(er)
    {
    case Errors::EmptyPath:
      {
        window = new StyledWindow(this);

        QLabel *lab = new QLabel;
        lab->setObjectName("Label");
        lab->setText(tr("Result path is empty!"));
        v_box->addWidget(lab);
        break;
      }
    case Errors::ErrorSaving:
      {
        window = new StyledWindow(this);

        QLabel *lab = new QLabel;
        lab->setObjectName("Label");
        lab->setText(tr("Error!"));
        v_box->addWidget(lab, 0, Qt::AlignCenter);

        lab = new QLabel;
        lab->setObjectName("Label");
        lab->setText(txt);
        v_box->addWidget(lab);
        break;
      }
    case Errors::Success:
      {
        window = new StyledWindow(this->parentWidget());

        QLabel *lab = new QLabel;
        lab->setObjectName("Label");
        lab->setText(tr("Base has been exported!"));
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
CollectionBaseExportWindow::paintEvent(QPaintEvent *event)
{
  QStyleOption opt;
  opt.initFrom(this);
  QPainter p(this);
  style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
