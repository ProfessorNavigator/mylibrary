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

#include <NotesManagerWindow.h>
#include <QFileDialog>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMenu>
#include <QPainter>
#include <QPushButton>
#include <QScreen>
#include <QVBoxLayout>
#include <StyledItemDelegate.h>
#include <StyledWindow.h>
#include <TableView.h>
#include <iostream>

NotesManagerWindow::NotesManagerWindow(
    QWidget *parent, const std::shared_ptr<MLBookProc> &mlbp,
    const std::shared_ptr<NotesKeeper> &notes,
    const std::shared_ptr<SettingsManager> &settings)
    : QWidget(parent)
{
  this->mlbp = mlbp;
  this->notes = notes;
  this->settings = settings;

  this->setAttribute(Qt::WA_DeleteOnClose);
  this->setWindowTitle(tr("Notes manager"));
  this->setWindowFlag(Qt::Window, true);
  this->setWindowModality(Qt::WindowModality::WindowModal);

  this->setObjectName("Window");
}

NotesManagerWindow::~NotesManagerWindow()
{
  if(model != nullptr)
    {
      model->deleteLater();
    }
}

void
NotesManagerWindow::createWindow()
{
  QVBoxLayout *v_box = new QVBoxLayout;
  this->setLayout(v_box);

  QLabel *lab = new QLabel;
  lab->setObjectName("Lable");
  QFont font = lab->font();
  font.setBold(true);
  lab->setFont(font);
  lab->setText(tr("Notes manager"));
  v_box->addWidget(lab, 0, Qt::AlignCenter);

  model = new NotesManagerModel(nullptr, notes);

  TableView *table = new TableView;
  table->setObjectName("Table");
  table->viewport()->setObjectName("TableViewport");
  QAbstractItemDelegate *delegate = table->itemDelegate();
  StyledItemDelegate *item_delegate = new StyledItemDelegate(table, settings);
  table->setItemDelegate(item_delegate);
  delete delegate;
  QAbstractItemModel *prev = table->model();
  table->setModel(model);
  delete prev;
  QHeaderView *hh = table->horizontalHeader();
  for(int i = 0; i < hh->count(); i++)
    {
      hh->setSectionResizeMode(i, QHeaderView::ResizeToContents);
    }
  table->setCurrentIndex(QModelIndex());
  table->setContextMenuPolicy(Qt::CustomContextMenu);
  v_box->addWidget(table);

  QAction *remove_act = new QAction(tr("Remove"));
  connect(remove_act, &QAction::triggered, this,
          [table, this]
            {
              QModelIndex index = table->currentIndex();
              if(index.isValid())
                {
                  removeDialog(index);
                }
            });
  table->addAction(remove_act);

  std::vector<QAction *> act_list;
  act_list.push_back(remove_act);

  QAction *save_as = new QAction(tr("Save note file as..."));
  connect(save_as, &QAction::triggered, this,
          [this, table]
            {
              saveDialog(table->currentIndex());
            });
  table->addAction(save_as);
  act_list.push_back(save_as);

  act_list.shrink_to_fit();
  connect(table, &TableView::destroyed,
          [act_list]
            {
              for(size_t i = 0; i < act_list.size(); i++)
                {
                  delete act_list[i];
                }
            });

  connect(table, &TableView::customContextMenuRequested,
          [table](const QPoint &pos)
            {
              table->setCurrentIndex(table->indexAt(pos));
              QList<QAction *> actions = table->actions();
              QMenu *menu = new QMenu(table);
              menu->setAttribute(Qt::WA_DeleteOnClose);
              menu->setObjectName("Menu");

              menu->addActions(actions);

              menu->popup(table->viewport()->mapToGlobal(pos));
            });

  connect(table, &TableView::signalLeftMouseButton,
          [table](const QPoint &glob)
            {
              QPoint pos = table->viewport()->mapFromGlobal(glob);
              table->setCurrentIndex(table->indexAt(pos));
            });

  QHBoxLayout *h_box = new QHBoxLayout;
  v_box->addLayout(h_box);

  QPushButton *remove = new QPushButton;
  remove->setText(tr("Remove selected"));
  QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(remove);
  shadow->setBlurRadius(12);
  shadow->setOffset(0, 2);
  shadow->setColor(QColor(0, 0, 0, 120));
  remove->setGraphicsEffect(shadow);
  remove->setObjectName("ClearButton");
  connect(remove, &QPushButton::clicked, remove_act, &QAction::trigger);
  h_box->addWidget(remove, 0, Qt::AlignCenter);

  QPushButton *save = new QPushButton;
  save->setText(tr("Save selected note file as..."));
  shadow = new QGraphicsDropShadowEffect(save);
  shadow->setBlurRadius(12);
  shadow->setOffset(0, 2);
  shadow->setColor(QColor(0, 0, 0, 120));
  save->setGraphicsEffect(shadow);
  save->setObjectName("ApplyButton");
  connect(save, &QPushButton::clicked, save_as, &QAction::trigger);
  h_box->addWidget(save, 0, Qt::AlignCenter);

  QPushButton *close = new QPushButton;
  close->setText(tr("Close"));
  shadow = new QGraphicsDropShadowEffect(close);
  shadow->setBlurRadius(12);
  shadow->setOffset(0, 2);
  shadow->setColor(QColor(0, 0, 0, 120));
  close->setGraphicsEffect(shadow);
  close->setObjectName("CancelButton");
  connect(close, &QPushButton::clicked, this, &NotesManagerWindow::close);
  h_box->addWidget(close, 0, Qt::AlignCenter);

  QScreen *screen = this->parentWidget()->screen();
  QSize av_sz = screen->availableSize();
  av_sz.setHeight(av_sz.height() * 0.7);
  av_sz.setWidth(av_sz.width() * 0.7);
  this->resize(av_sz);
}

void
NotesManagerWindow::removeDialog(const QModelIndex &index)
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
          [this, index, window]
            {
              model->removeNote(index);
              window->close();
            });
  h_box->addWidget(yes, Qt::AlignCenter);

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
NotesManagerWindow::saveDialog(const QModelIndex &index)
{
  if(!index.isValid())
    {
      return void();
    }

  const UDBElement *note
      = reinterpret_cast<const UDBElement *>(index.constInternalPointer());
  if(note == nullptr)
    {
      return void();
    }

  BaseID bid;
  auto it_nf = std::find_if(note->subelements.begin(), note->subelements.end(),
                            [bid](const UDBElement &el)
                              {
                                return bid.getId(el) == BaseID::BookNoteFile;
                              });
  if(it_nf == note->subelements.end())
    {
      return void();
    }

  QFileDialog *fd = new QFileDialog(this);
  fd->setAttribute(Qt::WA_DeleteOnClose);
  fd->setWindowModality(Qt::WindowModal);

  std::filesystem::path note_p(
      std::u8string(it_nf->content.begin(), it_nf->content.end()));
  std::u8string u8str = note_p.u8string();
  fd->selectFile(u8str.c_str());
  fd->setAcceptMode(QFileDialog::AcceptSave);
  fd->setDefaultSuffix(".txt");
  fd->setDirectory(QDir::homePath());
  fd->setNameFilter("*.txt");

  connect(fd, &QFileDialog::fileSelected, this,
          [note_p](const QString &fpth)
            {
              std::string str = fpth.toStdString();
              std::filesystem::path out(std::u8string(str.begin(), str.end()));
              if(note_p == out)
                {
                  return void();
                }
              std::filesystem::remove_all(out);
              std::error_code ec;
              std::filesystem::copy(note_p, out, ec);
              if(ec)
                {
                  std::cout << "NotesManagerWindow::saveDialog: \""
                            << ec.message() << "\" " << out << std::endl;
                }
            });

  fd->show();
}

void
NotesManagerWindow::paintEvent(QPaintEvent *event)
{
  QStyleOption opt;
  opt.initFrom(this);
  QPainter p(this);
  style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
