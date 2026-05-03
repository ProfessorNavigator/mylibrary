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

#include <AddBooksWindow.h>
#include <CollectionRefreshingProcWindow.h>
#include <QDir>
#include <QFileDialog>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <TableView.h>
#include <StyledWindow.h>
#include <filesystem>
#include <iostream>
#include <thread>

AddBooksWindow::AddBooksWindow(QWidget *parent,
                               const std::shared_ptr<MLBookProc> &mlbp)
    : QWidget(parent)
{
  this->mlbp = mlbp;
  this->setAttribute(Qt::WA_DeleteOnClose, true);
  this->setWindowTitle(tr("Books adding"));
  this->setWindowFlag(Qt::Window, true);
  this->setWindowModality(Qt::WindowModality::WindowModal);

  this->setObjectName("Window");

  model = new FilesModel;
}

AddBooksWindow::~AddBooksWindow()
{
  delete model;
}

void
AddBooksWindow::showWindow()
{
  QVBoxLayout *v_box = new QVBoxLayout;
  this->setLayout(v_box);

  QLabel *lab = new QLabel;
  lab->setObjectName("Label");
  QFont font = lab->font();
  font.setBold(true);
  lab->setFont(font);
  lab->setText(tr("Collection"));
  v_box->addWidget(lab, 0, Qt::AlignCenter);

  collections = new QComboBox;
  collections->setObjectName("ComboBox");
  std::string str = QDir::homePath().toStdString();
  std::filesystem::path home(std::u8string(str.begin(), str.end()));
  home /= std::filesystem::path(u8".local");
  home /= std::filesystem::path(u8"share");
  home /= std::filesystem::path(u8"MyLibrary");
  home /= std::filesystem::path(u8"Collections");

  std::error_code ec;
  for(auto &dir_it : std::filesystem::directory_iterator(home, ec))
    {
      std::filesystem::path p = dir_it;
      if(std::filesystem::is_directory(p))
        {
          p /= std::u8string(u8"base");
          if(std::filesystem::exists(p))
            {
              QVariant var = QVariant::fromValue(p);
              collections->addItem(
                  p.parent_path().filename().u8string().c_str(), var);
            }
        }
    }
  if(ec)
    {
      std::cout << "AddBooksWindow::showWindow: \"" << ec.message() << "\" "
                << home << std::endl;
      this->deleteLater();
      return void();
    }
  else if(collections->model()->rowCount() <= 0)
    {
      this->deleteLater();
      return void();
    }
  v_box->addWidget(collections, 0, Qt::AlignCenter);

  TableView *table = new TableView;
  table->setObjectName("Table");
  table->viewport()->setObjectName("TableViewport");
  connect(table, &TableView::signalResized, table,
          [table](const QSize &sz)
            {
              QHeaderView *hh = table->horizontalHeader();
              hh->resizeSection(0, sz.width());
            });
  QAbstractItemModel *old = table->model();
  table->setModel(model);
  if(old != nullptr)
    {
      old->deleteLater();
    }
  table->setTextElideMode(Qt::ElideLeft);
  table->setContextMenuPolicy(Qt::ActionsContextMenu);
  v_box->addWidget(table);

  std::vector<QAction *> act_list;
  QAction *remove_item_act = new QAction(tr("Remove item"));
  connect(remove_item_act, &QAction::triggered, this,
          [this, table]
            {
              QModelIndex index = table->currentIndex();
              model->removeItem(index);
            });
  table->addAction(remove_item_act);
  act_list.push_back(remove_item_act);

  act_list.shrink_to_fit();
  connect(table, &TableView::destroyed,
          [act_list]
            {
              for(size_t i = 0; i < act_list.size(); i++)
                {
                  delete act_list[i];
                }
            });

  QHBoxLayout *h_box = new QHBoxLayout;
  v_box->addLayout(h_box);

  QPushButton *add_file = new QPushButton;  
  add_file->setText(tr("Add files"));
  QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(add_file);
  shadow->setBlurRadius(12);
  shadow->setOffset(0, 2);
  shadow->setColor(QColor(0, 0, 0, 120));
  add_file->setGraphicsEffect(shadow);
  add_file->setObjectName("ApplyButton");
  connect(add_file, &QPushButton::clicked, this,
          &AddBooksWindow::filesAddDialog);
  h_box->addWidget(add_file, 0, Qt::AlignCenter);

  QPushButton *add_directory = new QPushButton;
  add_directory->setText(tr("Add directory"));
  shadow = new QGraphicsDropShadowEffect(add_directory);
  shadow->setBlurRadius(12);
  shadow->setOffset(0, 2);
  shadow->setColor(QColor(0, 0, 0, 120));
  add_directory->setGraphicsEffect(shadow);
  add_directory->setObjectName("ApplyButton");
  connect(add_directory, &QPushButton::clicked, this,
          &AddBooksWindow::directoriesAddDialog);
  h_box->addWidget(add_directory, 0, Qt::AlignCenter);

  QPushButton *remove_selected = new QPushButton;
  remove_selected->setText(tr("Remove selected item"));
  shadow = new QGraphicsDropShadowEffect(remove_selected);
  shadow->setBlurRadius(12);
  shadow->setOffset(0, 2);
  shadow->setColor(QColor(0, 0, 0, 120));
  remove_selected->setGraphicsEffect(shadow);
  remove_selected->setObjectName("ClearButton");
  connect(remove_selected, &QPushButton::clicked, remove_item_act,
          &QAction::trigger);
  h_box->addWidget(remove_selected, 0, Qt::AlignCenter);

  h_box = new QHBoxLayout;
  v_box->addLayout(h_box);

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Threads number") + ":");
  h_box->addWidget(lab);

  threads = new QLineEdit;
  threads->setObjectName("LineEdit");
  QString qstr;
  qstr.setNum(std::thread::hardware_concurrency());
  threads->setText(qstr);
  QFontMetrics mtr = threads->fontMetrics();
  threads->setMaximumWidth(
      mtr.horizontalAdvance(qstr) + mtr.averageCharWidth() * 2
      + threads->textMargins().left() + threads->textMargins().right());
  threads->setAlignment(Qt::AlignCenter);
  h_box->addWidget(threads);

  h_box->addStretch();

  h_box = new QHBoxLayout;
  v_box->addLayout(h_box);

  QPushButton *next = new QPushButton;
  next->setText(tr("Add"));
  shadow = new QGraphicsDropShadowEffect(next);
  shadow->setBlurRadius(12);
  shadow->setOffset(0, 2);
  shadow->setColor(QColor(0, 0, 0, 120));
  next->setGraphicsEffect(shadow);
  next->setObjectName("ApplyButton");
  connect(next, &QPushButton::clicked, this, &AddBooksWindow::addBooks);
  h_box->addWidget(next, 0, Qt::AlignCenter);

  QPushButton *cancel = new QPushButton;
  cancel->setText(tr("Cancel"));
  shadow = new QGraphicsDropShadowEffect(cancel);
  shadow->setBlurRadius(12);
  shadow->setOffset(0, 2);
  shadow->setColor(QColor(0, 0, 0, 120));
  cancel->setGraphicsEffect(shadow);
  cancel->setObjectName("CancelButton");
  connect(cancel, &QPushButton::clicked, this, &AddBooksWindow::close);
  h_box->addWidget(cancel, 0, Qt::AlignCenter);

  this->show();
}

void
AddBooksWindow::addBooks()
{
  std::filesystem::path base_path
      = collections->currentData().value<std::filesystem::path>();
  if(!std::filesystem::exists(base_path))
    {
      return void();
    }
  std::vector<std::filesystem::path> files = model->getBase();
  if(files.empty())
    {
      return void();
    }
  confirmationDialog(base_path);
}

void
AddBooksWindow::confirmationDialog(const std::filesystem::path &base_path)
{
  StyledWindow *window = new StyledWindow(this);
  window->setObjectName("Window");
  window->setWindowModality(Qt::WindowModal);

  QVBoxLayout *v_box = new QVBoxLayout;
  window->setLayout(v_box);

  QHBoxLayout *h_box = new QHBoxLayout;
  v_box->addLayout(h_box);

  QLabel *lab = new QLabel;
  lab->setText(tr("Collection books to be add to:"));
  lab->setObjectName("Label");
  h_box->addWidget(lab, 0, Qt::AlignCenter);

  lab = new QLabel;
  QFont font = lab->font();
  font.setItalic(true);
  lab->setFont(font);
  lab->setText(collections->currentText());
  lab->setObjectName("Label");
  h_box->addWidget(lab, 0, Qt::AlignCenter);

  lab = new QLabel;
  lab->setText(tr("Are you sure?"));
  lab->setObjectName("Label");
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
          [this, base_path, window]
            {
              QString str(threads->text());
              std::vector<std::filesystem::path> files = model->getBase();
              CollectionRefreshingProcWindow *proc
                  = new CollectionRefreshingProcWindow(
                      this->parentWidget(), mlbp, base_path, str.toInt(), true,
                      files);
              this->close();
              window->close();
              (void)(proc);
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
AddBooksWindow::filesAddDialog()
{
  QFileDialog *fd = new QFileDialog(this);
  fd->setAttribute(Qt::WA_DeleteOnClose);
  fd->setWindowModality(Qt::WindowModal);

  fd->setAcceptMode(QFileDialog::AcceptOpen);
  fd->setDirectory(QDir::homePath());
  fd->setFileMode(QFileDialog::ExistingFiles);

  std::vector<std::string> types = mlbp->getSupportedFileTypes();
  QString all;
  QStringList list;
  for(auto it = types.begin(); it != types.end(); it++)
    {
      QString str("*.");
      str += it->c_str();
      if(!all.isEmpty())
        {
          all += " ";
        }
      all += str;
      list.append(str);
    }
  list.insert(list.begin(), all);

  fd->setNameFilters(list);

  connect(fd, &QFileDialog::filesSelected,
          [this](const QStringList &list)
            {
              for(auto it = list.begin(); it != list.end(); it++)
                {
                  std::string str = it->toStdString();
                  std::filesystem::path p
                      = std::u8string(str.begin(), str.end());
                  model->addItem(p);
                }
            });

  fd->show();
}

void
AddBooksWindow::directoriesAddDialog()
{
  QFileDialog *fd = new QFileDialog(this);
  fd->setAttribute(Qt::WA_DeleteOnClose);
  fd->setWindowModality(Qt::WindowModal);

  fd->setAcceptMode(QFileDialog::AcceptOpen);
  fd->setDirectory(QDir::homePath());
  fd->setFileMode(QFileDialog::Directory);

  connect(fd, &QFileDialog::filesSelected,
          [this](const QStringList &list)
            {
              for(auto it = list.begin(); it != list.end(); it++)
                {
                  std::string str = it->toStdString();
                  std::filesystem::path p
                      = std::u8string(str.begin(), str.end());
                  model->addItem(p);
                }
            });

  fd->show();
}

void
AddBooksWindow::paintEvent(QPaintEvent *event)
{
  QStyleOption opt;
  opt.initFrom(this);
  QPainter p(this);
  style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
