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

#include <AuthorEditWindow.h>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QPainter>
#include <QPushButton>
#include <QScreen>
#include <QVBoxLayout>
#include <StyledWindow.h>
#include <TableView.h>

AuthorEditWindow::AuthorEditWindow(QWidget *parent) : QWidget(parent)
{
  this->setAttribute(Qt::WA_DeleteOnClose);
  this->setWindowTitle(tr("Authors editor"));
  this->setWindowFlag(Qt::Window, true);
  this->setWindowModality(Qt::WindowModality::WindowModal);

  this->setObjectName("Window");
}

AuthorEditWindow::~AuthorEditWindow()
{
  delete model;
}

void
AuthorEditWindow::createWindow(const QModelIndex &index)
{
  this->index = index;

  QVBoxLayout *v_box = new QVBoxLayout;
  this->setLayout(v_box);

  QLabel *lab = new QLabel;
  lab->setObjectName("Label");
  QFont font = lab->font();
  font.setBold(true);
  lab->setFont(font);
  lab->setText(tr("Authors editor"));
  v_box->addWidget(lab, 0, Qt::AlignCenter);

  QVariant var = index.data(Qt::EditRole);
  std::vector<UDBElement> authors = var.value<std::vector<UDBElement>>();
  model = new AuthorsModel(nullptr, authors);
  model->setEditable(true);

  TableView *table = new TableView;
  table->setObjectName("Table");
  table->viewport()->setObjectName("TableViewport");
  QAbstractItemModel *prev = table->model();
  table->setModel(model);
  delete prev;
  for(int i = 4; i < model->columnCount(); i++)
    {
      table->setColumnHidden(i, true);
    }
  connect(table, &TableView::signalResized, table,
          [table](const QSize &sz)
            {
              int total = sz.width();
              int col_w = total / 4;

              QHeaderView *hh = table->horizontalHeader();
              for(int i = 0; i < 3; i++)
                {
                  hh->resizeSection(i, col_w);
                  total -= col_w;
                }
              hh->resizeSection(3, total);
            });
  table->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(table, &TableView::customContextMenuRequested,
          [table](const QPoint &pos)
            {
              QModelIndex index = table->indexAt(pos);
              table->setCurrentIndex(index);

              QMenu *menu = new QMenu(table);
              menu->setAttribute(Qt::WA_DeleteOnClose);
              menu->setObjectName("Menu");
              menu->addActions(table->actions());
              menu->popup(table->viewport()->mapToGlobal(pos));
            });
  v_box->addWidget(table);

  std::vector<QAction *> act_list;

  QAction *edit = new QAction(tr("Edit"));
  connect(edit, &QAction::triggered, table,
          [table]
            {
              QModelIndex index = table->currentIndex();
              if(index.isValid())
                {
                  table->edit(index);
                }
            });
  table->addAction(edit);
  act_list.push_back(edit);

  QAction *add_act = new QAction(tr("Add author"));
  connect(add_act, &QAction::triggered, this,
          &AuthorEditWindow::addAuthorDialog);
  table->addAction(add_act);
  act_list.push_back(add_act);

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

  QPushButton *add = new QPushButton;
  add->setText(tr("Add author"));
  QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(add);
  shadow->setBlurRadius(12);
  shadow->setOffset(0, 2);
  shadow->setColor(QColor(0, 0, 0, 120));
  add->setGraphicsEffect(shadow);
  add->setObjectName("ApplyButton");
  connect(add, &QPushButton::clicked, add_act, &QAction::trigger);
  h_box->addWidget(add, 0, Qt::AlignCenter);

  QPushButton *remove = new QPushButton;
  remove->setText(tr("Remove selected author"));
  shadow = new QGraphicsDropShadowEffect(remove);
  shadow->setBlurRadius(12);
  shadow->setOffset(0, 2);
  shadow->setColor(QColor(0, 0, 0, 120));
  remove->setGraphicsEffect(shadow);
  remove->setObjectName("ClearButton");
  connect(remove, &QPushButton::clicked, table,
          [table, this]
            {
              QModelIndex index = table->currentIndex();
              if(index.isValid())
                {
                  model->removeAuthor(index);
                }
            });
  h_box->addWidget(remove, 0, Qt::AlignCenter);

  h_box = new QHBoxLayout;
  v_box->addLayout(h_box);

  QPushButton *apply = new QPushButton;
  apply->setText(tr("Apply"));
  shadow = new QGraphicsDropShadowEffect(apply);
  shadow->setBlurRadius(12);
  shadow->setOffset(0, 2);
  shadow->setColor(QColor(0, 0, 0, 120));
  apply->setGraphicsEffect(shadow);
  apply->setObjectName("ApplyButton");
  connect(apply, &QPushButton::clicked, this,
          [this]
            {
              applied = true;
              this->close();
            });
  h_box->addWidget(apply, 0, Qt::AlignCenter);

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

  QScreen *screen = this->parentWidget()->screen();
  QSize av_sz = screen->availableSize();

  av_sz.setHeight(av_sz.height() * 0.5);
  av_sz.setWidth(av_sz.width() * 0.5);
  this->resize(av_sz);
}

std::vector<UDBElement>
AuthorEditWindow::getAuthors()
{
  return model->getAuthors();
}

void
AuthorEditWindow::addAuthorDialog()
{
  StyledWindow *window = new StyledWindow(this);
  window->setObjectName("Window");
  window->setWindowTitle(tr("Author editor"));
  window->setWindowModality(Qt::WindowModality::WindowModal);

  QVBoxLayout *v_box = new QVBoxLayout;
  window->setLayout(v_box);

  QLabel *lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Surname:"));
  v_box->addWidget(lab, 0, Qt::AlignLeft);

  QLineEdit *surname = new QLineEdit;
  surname->setObjectName("LineEdit");
  v_box->addWidget(surname);

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Name:"));
  v_box->addWidget(lab, 0, Qt::AlignLeft);

  QLineEdit *name = new QLineEdit;
  name->setObjectName("LineEdit");
  v_box->addWidget(name);

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Second name:"));
  v_box->addWidget(lab, 0, Qt::AlignLeft);

  QLineEdit *sec_name = new QLineEdit;
  sec_name->setObjectName("LineEdit");
  v_box->addWidget(sec_name);

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Nickname:"));
  v_box->addWidget(lab, 0, Qt::AlignLeft);

  QLineEdit *nickname = new QLineEdit;
  nickname->setObjectName("LineEdit");
  v_box->addWidget(nickname);

  QHBoxLayout *h_box = new QHBoxLayout;
  v_box->addLayout(h_box);

  QPushButton *add = new QPushButton;
  add->setText(tr("Add"));
  QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(add);
  shadow->setBlurRadius(12);
  shadow->setOffset(0, 2);
  shadow->setColor(QColor(0, 0, 0, 120));
  add->setGraphicsEffect(shadow);
  add->setObjectName("ApplyButton");
  connect(add, &QPushButton::clicked, this,
          [this, window, surname, name, sec_name, nickname]
            {
              UDBElement author;
              bid.setId(author, BaseID::Author);

              UDBElement el;
              bid.setId(el, BaseID::LastName);
              el.content = surname->text().toStdString();
              author.subelements.emplace_back(el);

              el = UDBElement();
              bid.setId(el, BaseID::FirstName);
              el.content = name->text().toStdString();
              author.subelements.emplace_back(el);

              el = UDBElement();
              bid.setId(el, BaseID::MiddleName);
              el.content = sec_name->text().toStdString();
              author.subelements.emplace_back(el);

              el = UDBElement();
              bid.setId(el, BaseID::Nickname);
              el.content = nickname->text().toStdString();
              author.subelements.emplace_back(el);

              model->addAuthor(author);

              window->close();
            });
  h_box->addWidget(add, 0, Qt::AlignCenter);

  QPushButton *cancel = new QPushButton;
  cancel->setText(tr("Cancel"));
  shadow = new QGraphicsDropShadowEffect(cancel);
  shadow->setBlurRadius(12);
  shadow->setOffset(0, 2);
  shadow->setColor(QColor(0, 0, 0, 120));
  cancel->setGraphicsEffect(shadow);
  cancel->setObjectName("CancelButton");
  connect(cancel, &QPushButton::clicked, window, &QWidget::close);
  h_box->addWidget(cancel, 0, Qt::AlignCenter);

  window->show();
}

void
AuthorEditWindow::keyPressEvent(QKeyEvent *event)
{
  if(event->key() == Qt::Key_Return)
    {
      applied = true;
    }
  QWidget::keyPressEvent(event);
}

void
AuthorEditWindow::paintEvent(QPaintEvent *event)
{
  QStyleOption opt;
  opt.initFrom(this);
  QPainter p(this);
  style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
