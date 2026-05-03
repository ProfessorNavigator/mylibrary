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

#include <CollectionRefreshingProcWindow.h>
#include <QDir>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QVBoxLayout>
#include <RefreshCollectionWindow.h>
#include <StyledWindow.h>
#include <filesystem>
#include <iostream>

RefreshCollectionWindow::RefreshCollectionWindow(
    QWidget *parent, const std::shared_ptr<MLBookProc> &mlbp)
    : QWidget(parent)
{
  this->mlbp = mlbp;

  this->setWindowTitle(tr("Collection refreshing"));
  this->setAttribute(Qt::WA_DeleteOnClose);
  this->setWindowFlag(Qt::Window, true);
  this->setWindowModality(Qt::WindowModality::WindowModal);

  this->setObjectName("Window");
}

void
RefreshCollectionWindow::createWindow()
{
  QVBoxLayout *v_box = new QVBoxLayout;
  this->setLayout(v_box);

  QLabel *lab = new QLabel;
  lab->setObjectName("Label");
  QFont font = lab->font();
  font.setBold(true);
  lab->setFont(font);
  lab->setText(tr("Refresh collection:"));
  v_box->addWidget(lab, 0, Qt::AlignCenter);

  collections = new QComboBox;
  collections->setObjectName("ComboBox");
  std::string l_str = QDir::homePath().toStdString();
  std::filesystem::path home = std::u8string(l_str.begin(), l_str.end());
  home /= std::filesystem::path(u8".local");
  home /= std::filesystem::path(u8"share");
  home /= std::filesystem::path(u8"MyLibrary");
  home /= std::filesystem::path(u8"Collections");
  std::error_code ec;
  for(auto &dir_it : std::filesystem::directory_iterator(home, ec))
    {
      std::filesystem::path p = dir_it.path();
      if(std::filesystem::is_directory(p))
        {
          std::filesystem::path base_path
              = p / std::filesystem::path(u8"base");
          if(std::filesystem::exists(base_path))
            {
              QVariant var = QVariant::fromValue(base_path);
              collections->addItem(p.filename().u8string().c_str(), var);
            }
        }
    }
  if(ec)
    {
      std::cout << "RefreshCollectionWindow::createWindow: \"" << ec.message()
                << "\" " << home << std::endl;
      this->deleteLater();
      return void();
    }
  else if(collections->model()->rowCount() <= 0)
    {
      this->deleteLater();
      return void();
    }

  v_box->addWidget(collections, 0, Qt::AlignHCenter);

  fast = new QCheckBox;
  fast->setObjectName("CheckBox");
  fast->setText(tr("Fast refresh (without files hashing)"));
  fast->setCheckState(Qt::Unchecked);
  v_box->addWidget(fast, 0, Qt::AlignLeft);

  QHBoxLayout *h_box = new QHBoxLayout;
  v_box->addLayout(h_box);

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Threads number:"));
  h_box->addWidget(lab);

  threads = new QLineEdit;
  threads->setObjectName("LineEdit");
  QString str;
  str.setNum(omp_get_max_threads());
  QFontMetrics mtr = threads->fontMetrics();
  threads->setMaximumWidth(
      mtr.horizontalAdvance(str) + mtr.averageCharWidth() * 2
      + threads->textMargins().left() + threads->textMargins().right());
  threads->setAlignment(Qt::AlignCenter);
  threads->setText(str);
  h_box->addWidget(threads);

  h_box->addStretch();

  h_box = new QHBoxLayout;
  v_box->addLayout(h_box);

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Note: legacy and inpx collections will be removed and "
                  "recreated as native"));
  v_box->addWidget(lab, 0, Qt::AlignLeft);

  QPushButton *refresh = new QPushButton;
  refresh->setText(tr("Refresh"));
  QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(refresh);
  shadow->setBlurRadius(12);
  shadow->setOffset(0, 2);
  shadow->setColor(QColor(0, 0, 0, 120));
  refresh->setGraphicsEffect(shadow);
  refresh->setObjectName("ApplyButton");
  connect(refresh, &QPushButton::clicked, this,
          &RefreshCollectionWindow::confirmationDialog);
  h_box->addWidget(refresh, 0, Qt::AlignCenter);

  QPushButton *cancel = new QPushButton;
  cancel->setText(tr("Cancel"));
  shadow = new QGraphicsDropShadowEffect(cancel);
  shadow->setBlurRadius(12);
  shadow->setOffset(0, 2);
  shadow->setColor(QColor(0, 0, 0, 120));
  cancel->setGraphicsEffect(shadow);
  cancel->setObjectName("CancelButton");
  connect(cancel, &QPushButton::clicked, this, &QWidget::close);
  h_box->addWidget(cancel, 0, Qt::AlignCenter);
}

void
RefreshCollectionWindow::confirmationDialog()
{
  StyledWindow *window = new StyledWindow(this);
  window->setWindowTitle(tr("Confirmation")); 
  window->setWindowModality(Qt::WindowModality::WindowModal);
  window->setObjectName("Window");

  QVBoxLayout *v_box = new QVBoxLayout;
  window->setLayout(v_box);

  QLabel *lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Collection for refreshing:"));
  v_box->addWidget(lab, 1, Qt::AlignCenter);

  lab = new QLabel;
  lab->setObjectName("Label");
  QFont font = lab->font();
  font.setItalic(true);
  lab->setFont(font);
  lab->setText(collections->currentText());
  v_box->addWidget(lab, 1, Qt::AlignCenter);

  v_box->addStretch(1);

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Are you sure?"));
  v_box->addWidget(lab, 1, Qt::AlignCenter);

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
          [this, window]
            {
              QVariant var = collections->currentData();
              if(var.isNull())
                {
                  return void();
                }
              std::filesystem::path base_path
                  = var.value<std::filesystem::path>();
              if(!std::filesystem::exists(base_path))
                {
                  return void();
                }
              QString str = threads->text();
              int thr = str.toInt();
              if(thr <= 0)
                {
                  thr = 1;
                }
              bool f;
              if(fast->checkState() == Qt::Checked)
                {
                  f = true;
                }
              else
                {
                  f = false;
                }
              CollectionRefreshingProcWindow *crpw
                  = new CollectionRefreshingProcWindow(
                      this->parentWidget(), mlbp, base_path, thr, f);
              window->close();
              this->close();
              (void)(crpw);
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
  connect(no, &QPushButton::clicked, window, &QWidget::close);
  h_box->addWidget(no, 0, Qt::AlignCenter);

  window->show();
}

void
RefreshCollectionWindow::paintEvent(QPaintEvent *event)
{
  QStyleOption opt;
  opt.initFrom(this);
  QPainter p(this);
  style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
