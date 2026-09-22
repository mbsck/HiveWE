module;

#include <QLabel>
#include <QPainter>
#include <QPixmap>

#include "aspect_ratio_pixmap_label.h"

export module AspectRatioPixmapLabel;

export class AspectRatioPixmapLabel : public QLabel {
Q_OBJECT

QPixmap pixmap;

QPixmap get_scaled_pixmap(bool grid_lines) {
	QPixmap new_pixmap(width(), height());

	QPainter painter(&new_pixmap);
	painter.fillRect(0, 0, width(), height(), Qt::black);

	QPixmap scaled_pixmap = pixmap.scaled(width(), height(), Qt::KeepAspectRatio);
	horizontal_border = (width() - scaled_pixmap.width()) / 2.f;
	vertical_border = (height() - scaled_pixmap.height()) / 2.f;

	painter.drawPixmap(horizontal_border, vertical_border, scaled_pixmap);

	return new_pixmap;
}

public:
AspectRatioPixmapLabel() {
setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

using QLabel::QLabel;

int horizontal_border;
int vertical_border;

public slots:
void setPixmap(const QPixmap& p) {
pixmap = p;

	if (pixmap.isNull()) {
		return;
	}

	QLabel::setPixmap(get_scaled_pixmap(false));
}

void resizeEvent(QResizeEvent* e) override {
	Q_UNUSED(e)

	if (pixmap.isNull()) {
		return;
	}

	QLabel::setPixmap(get_scaled_pixmap(false));
}

};

#include "aspect_ratio_pixmap_label.moc"
