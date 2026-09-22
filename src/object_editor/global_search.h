#pragma once

#include <memory>

#include <QAbstractListModel>
#include <QDialog>
#include <QStyledItemDelegate>

class GlobalSearchWidget : public QDialog {
	Q_OBJECT

public:
	explicit GlobalSearchWidget(QWidget* parent = nullptr);
	~GlobalSearchWidget() override;

	void changeEvent(QEvent* e) override;
	bool eventFilter(QObject* object, QEvent* event) override;

signals:
	void text_changed(QString text);
	void previous();
	void next();

private:
	class Private;
	std::unique_ptr<Private> d;
};
