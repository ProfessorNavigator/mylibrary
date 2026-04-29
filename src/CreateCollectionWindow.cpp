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

#include <CollectionCreationProcWindow.h>
#include <CreateCollectionWindow.h>
#include <QDir>
#include <QFileDialog>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMenu>
#include <QPainter>
#include <QPushButton>
#include <QVBoxLayout>
#include <TableView.h>
#include <StyledWindow.h>

CreateCollectionWindow::CreateCollectionWindow(
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

CreateCollectionWindow::~CreateCollectionWindow()
{
  delete model;
}

void
CreateCollectionWindow::createWindow()
{
  QVBoxLayout *v_box = new QVBoxLayout;
  this->setLayout(v_box);

  QLabel *lab = new QLabel;
  lab->setObjectName("Label");
  QFont font = lab->font();
  font.setBold(true);
  lab->setFont(font);
  lab->setText(tr("Collection creation"));
  v_box->addWidget(lab, 0, Qt::AlignCenter);

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Collection name:"));
  v_box->addWidget(lab, 0, Qt::AlignLeft);

  collection_name = new QLineEdit;
  collection_name->setObjectName("LineEdit");
  v_box->addWidget(collection_name);

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Collection files and directories"));
  v_box->addWidget(lab, 0, Qt::AlignCenter);

  model = new FilesModel;

  TableView *collection_files = new TableView;
  collection_files->setObjectName("Table");
  collection_files->viewport()->setObjectName("TableViewport");
  QAbstractItemModel *m = collection_files->model();
  collection_files->setModel(model);
  if(m != nullptr)
    {
      m->deleteLater();
    }
  collection_files->setTextElideMode(Qt::ElideNone);
  collection_files->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(collection_files, &TableView::customContextMenuRequested,
          [collection_files](const QPoint &pos)
            {
              QModelIndex index = collection_files->indexAt(pos);
              collection_files->setCurrentIndex(index);
              if(index.isValid())
                {
                  QMenu *menu = new QMenu(collection_files);
                  menu->setAttribute(Qt::WA_DeleteOnClose);
                  menu->setObjectName("Menu");
                  menu->addActions(collection_files->actions());
                  menu->popup(collection_files->viewport()->mapToGlobal(pos));
                }
            });
  connect(collection_files, &TableView::signalResized, this,
          [collection_files](const QSize &sz)
            {
              QHeaderView *hh = collection_files->horizontalHeader();
              hh->resizeSection(0, sz.width());
            });
  v_box->addWidget(collection_files);

  QAction *remove_item_act = new QAction(tr("Remove item"));
  connect(remove_item_act, &QAction::triggered, this,
          [this, collection_files]
            {
              QModelIndex index = collection_files->currentIndex();
              model->removeItem(index);
            });
  collection_files->addAction(remove_item_act);

  std::vector<QAction *> act_list;
  act_list.push_back(remove_item_act);

  act_list.shrink_to_fit();
  connect(collection_files, &TableView::destroyed,
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
          &CreateCollectionWindow::filesAddDialog);
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
          &CreateCollectionWindow::directoriesAddDialog);
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
  QString str;
  str.setNum(std::thread::hardware_concurrency());
  threads->setText(str);
  QFontMetrics mtr = threads->fontMetrics();
  threads->setMaximumWidth(
      mtr.horizontalAdvance(str) + mtr.averageCharWidth() * 2
      + threads->textMargins().left() + threads->textMargins().right());
  threads->setAlignment(Qt::AlignCenter);
  h_box->addWidget(threads);

  h_box->addStretch();

  h_box = new QHBoxLayout;
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
          &CreateCollectionWindow::checkInput);
  h_box->addWidget(create, 0, Qt::AlignCenter);

  QPushButton *cancel = new QPushButton;
  cancel->setText(tr("Cancel"));
  shadow = new QGraphicsDropShadowEffect(cancel);
  shadow->setBlurRadius(12);
  shadow->setOffset(0, 2);
  shadow->setColor(QColor(0, 0, 0, 120));
  cancel->setGraphicsEffect(shadow);
  cancel->setObjectName("CancelButton");
  connect(cancel, &QPushButton::clicked, this, &CreateCollectionWindow::close);
  h_box->addWidget(cancel, 0, Qt::AlignCenter);
}

void
CreateCollectionWindow::filesAddDialog()
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
CreateCollectionWindow::directoriesAddDialog()
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
CreateCollectionWindow::checkInput()
{
  if(model->rowCount() <= 0)
    {
      return void();
    }
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
  base_path /= std::u8string(l_str.begin(), l_str.end());
  base_path /= std::filesystem::path(u8"base");
  if(std::filesystem::exists(base_path.parent_path()))
    {
      errorDialog(Error::CollectionExists);
      return void();
    }

  std::vector<std::filesystem::path> files = model->getBase();

  str = threads->text();
  int thr;
  thr = str.toInt();

  CollectionCreationProcWindow *proc = new CollectionCreationProcWindow(
      this->parentWidget(), mlbp, base_path, files, thr);
  proc->createWindow();
  proc->show();
  proc->startCreationProc();

  this->close();
}

void
CreateCollectionWindow::errorDialog(const Error &er)
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
CreateCollectionWindow::paintEvent(QPaintEvent *event)
{
  QStyleOption opt;
  opt.initFrom(this);
  QPainter p(this);
  style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
