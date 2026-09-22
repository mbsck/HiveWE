#include "global_search.h"

#include "model_editor.h"
#include "object_editor.h"
#include "trigger_editor.h"

#include <memory>
#include <string>

#include <QCoreApplication>
#include <QDialog>
#include <QFrame>
#include <QIcon>
#include <QKeyEvent>
#include <QLayout>
#include <QLineEdit>
#include <QListView>
#include <QPainter>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QVBoxLayout>
#include <QConcatenateTablesProxyModel>
#include <QStyledItemDelegate>
#include <QSize>

#include <ui_object_editor.h>

import std;
import WindowHandler;
import Globals;
import TableModel;

import DestructibleListModel;
import UnitListModel;
import BaseListModel;
import DoodadListModel;
import ItemListModel;
import AbilityListModel;
import UpgradeListModel;
import BuffListModel;


class ActionListModel : public QAbstractListModel {
public:
	struct Item {
		QString text;
		QIcon icon;
	};

	QList<Item> items;

	explicit ActionListModel(QObject* parent = nullptr)
		: QAbstractListModel(parent)
	{
		items = {
			{"Open Object Editor", QIcon("data/icons/ribbon/objecteditor.png")},
			{"Open Model Editor", QIcon("data/icons/ribbon/model_editor.png")},
			{"Open Trigger Editor", QIcon("data/icons/ribbon/triggereditor.png")}
		};
	}

	int rowCount(const QModelIndex& parent = QModelIndex()) const override {
		return parent.isValid() ? 0 : items.size();
	}

	QVariant data(const QModelIndex& index, const int role) const override {
		if (!index.isValid() || index.row() >= items.size()) {
			return {};
		}

		const auto& it = items[index.row()];

		if (role == Qt::DisplayRole) {
			return it.text;
		}

		if (role == Qt::DecorationRole) {
			return it.icon;
		}

		return {};
	}
};


class ExtraTextDelegate : public QStyledItemDelegate {
public:
	using QStyledItemDelegate::QStyledItemDelegate;

	QFont font = QFont("Consolas");

	void paint(
		QPainter* painter,
		const QStyleOptionViewItem& option,
		const QModelIndex& index
	) const override {
		QStyledItemDelegate::paint(painter, option, index);

		const QString rightText = index.data(Qt::UserRole).toString();

		if (rightText.isEmpty()) {
			return;
		}

		painter->save();

		painter->setFont(font);
		painter->setPen(Qt::gray);

		const QRect rect = option.rect;
		const QRect rightRect = rect.adjusted(6, 0, -6, 0);
		const QFontMetrics fm = painter->fontMetrics();

		const int x =
			rightRect.right() - fm.horizontalAdvance(rightText);

		const int y =
			rightRect.top() +
			(rightRect.height() - fm.height()) / 2;

		painter->drawText(x, y + fm.ascent(), rightText);

		painter->restore();
	}
};


class GlobalSearchWidget::Private {
public:
	QLineEdit* edit = new QLineEdit;
	QPushButton* case_sensitive = new QPushButton;
	QPushButton* match_whole_word = new QPushButton;
	QPushButton* regular_expression = new QPushButton;

	DoodadListModel* doodad_list_model = nullptr;
	DoodadListFilter* doodad_filter_model = nullptr;

	DestructableListModel* destructable_list_model = nullptr;
	DestructableListFilter* destructable_filter_model = nullptr;

	UnitListModel* unit_list_model = nullptr;
	UnitListFilter* units_filter_model = nullptr;

	AbilityListModel* ability_list_model = nullptr;
	AbilityListFilter* ability_filter_model = nullptr;

	ItemListModel* items_list_model = nullptr;
	ItemListFilter* item_filter_model = nullptr;

	UpgradeListModel* upgrade_list_model = nullptr;
	UpgradeListFilter* upgrade_filter_model = nullptr;

	BuffListModel* buff_list_model = nullptr;
	BuffListFilter* buff_filter_model = nullptr;

	ActionListModel* action_model = nullptr;
	QSortFilterProxyModel* action_filter_model = nullptr;

	QConcatenateTablesProxyModel* concat_table = nullptr;

	QListView* list = nullptr;
};


GlobalSearchWidget::GlobalSearchWidget(QWidget* parent)
	: QDialog(parent)
	, d(std::make_unique<Private>())
{
	setWindowFlag(Qt::FramelessWindowHint, true);
	setAttribute(Qt::WA_DeleteOnClose);

	resize(500, 700);

	d->ability_list_model = new AbilityListModel(this);
	d->ability_list_model->setSourceModel(abilities_table);

	d->ability_filter_model = new AbilityListFilter(this);
	d->ability_filter_model->setFilterCaseSensitivity(Qt::CaseInsensitive);
	d->ability_filter_model->setSourceModel(d->ability_list_model);
	d->ability_filter_model->sort(0, Qt::AscendingOrder);


	d->destructable_list_model = new DestructableListModel(this);
	d->destructable_list_model->setSourceModel(destructibles_table);

	d->destructable_filter_model = new DestructableListFilter(this);
	d->destructable_filter_model->setFilterCaseSensitivity(Qt::CaseInsensitive);
	d->destructable_filter_model->setSourceModel(d->destructable_list_model);
	d->destructable_filter_model->sort(0, Qt::AscendingOrder);


	d->doodad_list_model = new DoodadListModel(this);
	d->doodad_list_model->setSourceModel(doodads_table);

	d->doodad_filter_model = new DoodadListFilter(this);
	d->doodad_filter_model->setFilterCaseSensitivity(Qt::CaseInsensitive);
	d->doodad_filter_model->setSourceModel(d->doodad_list_model);
	d->doodad_filter_model->sort(0, Qt::AscendingOrder);


	d->items_list_model = new ItemListModel(this);
	d->items_list_model->setSourceModel(items_table);

	d->item_filter_model = new ItemListFilter(this);
	d->item_filter_model->setFilterCaseSensitivity(Qt::CaseInsensitive);
	d->item_filter_model->setSourceModel(d->items_list_model);
	d->item_filter_model->sort(0, Qt::AscendingOrder);


	d->unit_list_model = new UnitListModel(this);
	d->unit_list_model->setSourceModel(units_table);

	d->units_filter_model = new UnitListFilter(this);
	d->units_filter_model->setFilterCaseSensitivity(Qt::CaseInsensitive);
	d->units_filter_model->setSourceModel(d->unit_list_model);
	d->units_filter_model->sort(0, Qt::AscendingOrder);


	d->upgrade_list_model = new UpgradeListModel(this);
	d->upgrade_list_model->setSourceModel(upgrade_table);

	d->upgrade_filter_model = new UpgradeListFilter(this);
	d->upgrade_filter_model->setFilterCaseSensitivity(Qt::CaseInsensitive);
	d->upgrade_filter_model->setSourceModel(d->upgrade_list_model);
	d->upgrade_filter_model->sort(0, Qt::AscendingOrder);


	d->buff_list_model = new BuffListModel(this);
	d->buff_list_model->setSourceModel(buff_table);

	d->buff_filter_model = new BuffListFilter(this);
	d->buff_filter_model->setFilterCaseSensitivity(Qt::CaseInsensitive);
	d->buff_filter_model->setSourceModel(d->buff_list_model);
	d->buff_filter_model->sort(0, Qt::AscendingOrder);


	d->action_model = new ActionListModel(this);

	d->action_filter_model = new QSortFilterProxyModel(this);
	d->action_filter_model->setFilterCaseSensitivity(Qt::CaseInsensitive);
	d->action_filter_model->setSourceModel(d->action_model);
	d->action_filter_model->sort(0, Qt::AscendingOrder);


	d->concat_table = new QConcatenateTablesProxyModel(this);

	d->concat_table->addSourceModel(d->units_filter_model);
	d->concat_table->addSourceModel(d->ability_filter_model);
	d->concat_table->addSourceModel(d->destructable_filter_model);
	d->concat_table->addSourceModel(d->doodad_filter_model);
	d->concat_table->addSourceModel(d->item_filter_model);
	d->concat_table->addSourceModel(d->upgrade_filter_model);
	d->concat_table->addSourceModel(d->buff_filter_model);
	d->concat_table->addSourceModel(d->action_filter_model);


	d->list = new QListView;
	d->list->setModel(d->concat_table);
	d->list->setUniformItemSizes(true);
	d->list->setGridSize(QSize(d->list->gridSize().width(), 32));
	d->list->setIconSize(QSize(32, 32));
	d->list->setCurrentIndex(d->concat_table->index(0, 0));
	d->list->setItemDelegate(new ExtraTextDelegate(d->list));


	d->edit->setClearButtonEnabled(true);
	d->edit->setPlaceholderText("Find anything");
	d->edit->addAction(
		QIcon("data/icons/object_editor/search.png"),
		QLineEdit::LeadingPosition
	);
	d->edit->installEventFilter(this);


	QVBoxLayout* layout = new QVBoxLayout;
	layout->addWidget(d->edit);
	layout->addWidget(d->list);
	layout->setSpacing(3);

	setLayout(layout);


	connect(
		d->edit,
		&QLineEdit::textEdited,
		d->ability_filter_model,
		&QSortFilterProxyModel::setFilterFixedString
	);

	connect(
		d->edit,
		&QLineEdit::textEdited,
		d->item_filter_model,
		&QSortFilterProxyModel::setFilterFixedString
	);

	connect(
		d->edit,
		&QLineEdit::textEdited,
		d->units_filter_model,
		&QSortFilterProxyModel::setFilterFixedString
	);

	connect(
		d->edit,
		&QLineEdit::textEdited,
		d->doodad_filter_model,
		&QSortFilterProxyModel::setFilterFixedString
	);

	connect(
		d->edit,
		&QLineEdit::textEdited,
		d->destructable_filter_model,
		&QSortFilterProxyModel::setFilterFixedString
	);

	connect(
		d->edit,
		&QLineEdit::textEdited,
		d->upgrade_filter_model,
		&QSortFilterProxyModel::setFilterFixedString
	);

	connect(
		d->edit,
		&QLineEdit::textEdited,
		d->buff_filter_model,
		&QSortFilterProxyModel::setFilterFixedString
	);

	connect(
		d->edit,
		&QLineEdit::textEdited,
		d->action_filter_model,
		&QSortFilterProxyModel::setFilterFixedString
	);

	connect(d->edit, &QLineEdit::textEdited, [&] {
		d->list->setCurrentIndex(d->concat_table->index(0, 0));
	});


	connect(d->list, &QListView::activated, [this](const QModelIndex& index) {
		const auto model = d->concat_table->mapToSource(index).model();

		if (model == d->action_filter_model) {
			if (index.data() == "Open Model Editor") {
				bool created;
				window_handler.create_or_raise<ModelEditor>(nullptr, created);
			}
			else if (index.data() == "Open Object Editor") {
				bool created;
				window_handler.create_or_raise<ObjectEditor>(nullptr, created);
			}
			else if (index.data() == "Open Trigger Editor") {
				bool created;
				window_handler.create_or_raise<TriggerEditor>(nullptr, created);
			}

			return;
		}


		std::string id;
		TableModel* table_model = nullptr;


		if (model == d->units_filter_model) {
			const int row =
				d->units_filter_model
					->mapToSource(d->concat_table->mapToSource(index))
					.row();

			id = units_slk.index_to_row.at(row);
			table_model = units_table;
		}
		else if (model == d->destructable_filter_model) {
			const int row =
				d->destructable_filter_model
					->mapToSource(d->concat_table->mapToSource(index))
					.row();

			id = destructibles_slk.index_to_row.at(row);
			table_model = destructibles_table;
		}
		else if (model == d->doodad_filter_model) {
			const int row =
				d->doodad_filter_model
					->mapToSource(d->concat_table->mapToSource(index))
					.row();

			id = doodads_slk.index_to_row.at(row);
			table_model = doodads_table;
		}
		else if (model == d->item_filter_model) {
			const int row =
				d->item_filter_model
					->mapToSource(d->concat_table->mapToSource(index))
					.row();

			id = items_slk.index_to_row.at(row);
			table_model = items_table;
		}
		else if (model == d->ability_filter_model) {
			const int row =
				d->ability_filter_model
					->mapToSource(d->concat_table->mapToSource(index))
					.row();

			id = abilities_slk.index_to_row.at(row);
			table_model = abilities_table;
		}
		else if (model == d->buff_filter_model) {
			const int row =
				d->buff_filter_model
					->mapToSource(d->concat_table->mapToSource(index))
					.row();

			id = buffs_slk.index_to_row.at(row);
			table_model = buff_table;
		}
		else if (model == d->upgrade_filter_model) {
			const int row =
				d->upgrade_filter_model
					->mapToSource(d->concat_table->mapToSource(index))
					.row();

			id = upgrades_slk.index_to_row.at(row);
			table_model = upgrade_table;
		}


		bool created;

		auto* object_editor =
			window_handler.create_or_raise<ObjectEditor>(
				nullptr,
				created
			);

		object_editor->open_by_id(
			table_model,
			id,
			index.data(Qt::DisplayRole).toString(),
			index.data(Qt::DecorationRole).value<QIcon>()
		);
	});
}


GlobalSearchWidget::~GlobalSearchWidget() = default;


void GlobalSearchWidget::changeEvent(QEvent* e)
{
	if (e->type() == QEvent::ActivationChange && !isActiveWindow()) {
		close();
	}

	QDialog::changeEvent(e);
}


bool GlobalSearchWidget::eventFilter(QObject* object, QEvent* event)
{
	if (object == d->edit && event->type() == QEvent::KeyPress) {
		const QKeyEvent* keyEvent =
			static_cast<const QKeyEvent*>(event);

		if (
			keyEvent->key() == Qt::Key_Down ||
			keyEvent->key() == Qt::Key_Up ||
			keyEvent->key() == Qt::Key_Return ||
			keyEvent->key() == Qt::Key_Enter
		) {
			QCoreApplication::sendEvent(d->list, event);
			return true;
		}
	}

	return QDialog::eventFilter(object, event);
}
