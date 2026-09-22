#pragma once

#include <QLabel>
#include <QPixmap>

class AspectRatioPixmapLabel : public QLabel {
Q_OBJECT

QPixmap pixmap;

QPixmap get_scaled_pixmap(bool grid_lines);

public:
AspectRatioPixmapLabel();
using QLabel::QLabel;

int horizontal_border;
int vertical_border;

public slots:
void setPixmap(const QPixmap& p);
void resizeEvent(QResizeEvent* e) override;
};
