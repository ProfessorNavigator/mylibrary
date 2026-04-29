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

#include <QDir>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QVBoxLayout>
#include <RemoveCollectionWindow.h>
#include <StyledWindow.h>
#include <filesystem>

RemoveCollectionWindow::RemoveCollectionWindow(QWidget *parent)
    : QWidget(parent)
{
  this->setAttribute(Qt::WA_DeleteOnClose, true);
  this->setWindowTitle(tr("Collection removing"));
  this->setWindowFlag(Qt::Window, true);
  this->setWindowModality(Qt::WindowModality::WindowModal);

  this->setObjectName("Window");
}

void
RemoveCollectionWindow::createWindow()
{
  QVBoxLayout *v_box = new QVBoxLayout;
  this->setLayout(v_box);

  QLabel *lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Remove collection:"));
  v_box->addWidget(lab, 0, Qt::AlignHCenter);

  collections = new QComboBox;
  collections->setObjectName("ComboBox");
  v_box->addWidget(collections, 0, Qt::AlignHCenter);

  std::string str = QDir::homePath().toStdString();
  std::filesystem::path collections_path
      = std::u8string(str.begin(), str.end());
  collections_path /= std::filesystem::path(u8".local");
  collections_path /= std::filesystem::path(u8"share");
  collections_path /= std::filesystem::path(u8"MyLibrary");
  collections_path /= std::filesystem::path(u8"Collections");

  if(std::filesystem::exists(collections_path))
    {
      for(auto &dir_it : std::filesystem::directory_iterator(collections_path))
        {
          std::filesystem::path p = dir_it.path();
          if(std::filesystem::is_directory(p))
            {
              QString str(p.filename().u8string().c_str());
              p /= std::filesystem::path(u8"base");
              QVariant var;
              var.setValue(p);
              collections->addItem(str, var);
            }
        }
    }

  QHBoxLayout *h_box = new QHBoxLayout;
  v_box->addLayout(h_box);

  QPushButton *remove = new QPushButton;
  remove->setText(tr("Remove"));
  QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(remove);
  shadow->setBlurRadius(12);
  shadow->setOffset(0, 2);
  shadow->setColor(QColor(0, 0, 0, 120));
  remove->setGraphicsEffect(shadow);
  remove->setObjectName("ApplyButton");
  connect(remove, &QPushButton::clicked, this,
          &RemoveCollectionWindow::confirmationDialog);
  h_box->addWidget(remove, 0, Qt::AlignCenter);

  QPushButton *cancel = new QPushButton;
  cancel->setText(tr("Cancel"));
  shadow = new QGraphicsDropShadowEffect(cancel);
  shadow->setBlurRadius(12);
  shadow->setOffset(0, 2);
  shadow->setColor(QColor(0, 0, 0, 120));
  cancel->setGraphicsEffect(shadow);
  cancel->setObjectName("CancelButton");
  connect(cancel, &QPushButton::clicked, this, &RemoveCollectionWindow::close);
  h_box->addWidget(cancel);
}

void
RemoveCollectionWindow::confirmationDialog()
{
  QString col_name = collections->currentText();
  QVariant var = collections->currentData();
  if(col_name.isEmpty() || var.isNull())
    {
      return void();
    }

  StyledWindow *window = new StyledWindow(this);
  window->setWindowModality(Qt::WindowModal);
  window->setObjectName("Window");

  QVBoxLayout *v_box = new QVBoxLayout;
  window->setLayout(v_box);

  QHBoxLayout *h_box = new QHBoxLayout;
  v_box->addLayout(h_box);

  h_box->addStretch();

  QLabel *lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Collection for removing:"));
  h_box->addWidget(lab);

  lab = new QLabel;
  lab->setObjectName("Label");
  QFont font = lab->font();
  font.setItalic(true);
  lab->setFont(font);
  lab->setText(col_name);
  h_box->addWidget(lab);

  h_box->addStretch();

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Are you sure?"));
  v_box->addWidget(lab, 0, Qt::AlignCenter);

  h_box = new QHBoxLayout;
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
          [this, window, col_name, var]
            {
              std::filesystem::path p = var.value<std::filesystem::path>();
              std::filesystem::remove_all(p.parent_path());
              resultMessage(col_name);
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
RemoveCollectionWindow::resultMessage(const QString &col_name)
{
  emit signalCollectionRemoved(col_name);

  StyledWindow *window = new StyledWindow(this->parentWidget());
  window->setWindowModality(Qt::WindowModal);
  window->setObjectName("Window");

  QVBoxLayout *v_box = new QVBoxLayout;
  window->setLayout(v_box);

  QLabel *lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Collection has been removed!"));
  v_box->addWidget(lab, 0, Qt::AlignCenter);

  QHBoxLayout *h_box = new QHBoxLayout;
  v_box->addLayout(h_box);

  h_box->addStretch();

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Collection") + ":");
  h_box->addWidget(lab);

  lab = new QLabel;
  lab->setObjectName("Label");
  QFont font = lab->font();
  font.setItalic(true);
  lab->setFont(font);
  lab->setText(col_name);
  h_box->addWidget(lab);

  h_box->addStretch();

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

  this->close();
}

void
RemoveCollectionWindow::paintEvent(QPaintEvent *event)
{
  QStyleOption opt;
  opt.initFrom(this);
  QPainter p(this);
  style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
