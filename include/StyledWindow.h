#ifndef STYLEDWINDOW_H
#define STYLEDWINDOW_H

#include <QPaintEvent>
#include <QWidget>

class StyledWindow : public QWidget
{
  Q_OBJECT
public:
  StyledWindow(QWidget *parent = nullptr);

protected:
  virtual void
  paintEvent(QPaintEvent *event) override;
};

#endif // STYLEDWINDOW_H
