/* This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2015 Artem Pavlenko
 *
 * Mapnik is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */


#include "styles_model.hpp"
#include <mapnik/config.hpp>
#include <mapnik/util/variant.hpp>
#include <mapnik/expression_string.hpp>
#include <mapnik/util/noncopyable.hpp>
#include <mapnik/rule.hpp>
#include <mapnik/feature_type_style.hpp>
#include <mapnik/symbolizer.hpp>
// boost
#include <boost/concept_check.hpp>

// qt
#include <QList>
#include <QIcon>
#include <QPainter>
#include <QPixmap>

class node : private mapnik::util::noncopyable
{
    struct node_base
    {
        virtual QString name() const=0;
        virtual QIcon   icon() const=0;
        virtual ~node_base() {}
    };

    template <typename T>
    struct wrap : public node_base
    {
    wrap(T const& obj)
        : obj_(obj) {}

    ~wrap() {}

    QString name () const
    {
        return obj_.name();
    }

    QIcon icon() const
    {
        return obj_.icon();
    }

    T obj_;
    };

public:
    template <typename T>
    node ( T const& obj, node * parent=0)
    : impl_(new wrap<T>(obj)),
      parent_(parent)
    {}

    QString name() const
    {
        return impl_->name();
    }

    QIcon icon() const
    {
        return impl_->icon();
    }

    unsigned num_children() const
    {
        return children_.count();
    }

    node * child(unsigned row) const
    {
        return children_.value(row);
    }

    node * parent() const
    {
        return parent_;
    }

    node * add_child(node * child)
    {
        children_.push_back(child);
        return child;
    }
    int row () const
    {
        if (parent_)
            return parent_->children_.indexOf(const_cast<node*>(this));
        else
            return 0;
    }

    ~node()
    {
        qDeleteAll(children_);
    }

private:
    const std::unique_ptr<node_base> impl_;
    QList<node*> children_;
    node * parent_;
};


struct symbolizer_info
{
    QString operator() (mapnik::point_symbolizer const& sym) const
    {
        boost::ignore_unused_variable_warning(sym);
        return QString("PointSymbolizer");
    }

    QString operator() (mapnik::line_symbolizer const& sym) const
    {
        boost::ignore_unused_variable_warning(sym);
        return QString("LineSymbolizer");
    }

    QString operator() (mapnik::line_pattern_symbolizer const& sym) const
    {
        boost::ignore_unused_variable_warning(sym);
        return QString("LinePatternSymbolizer");
    }

    QString operator() (mapnik::polygon_symbolizer const& sym) const
    {
        boost::ignore_unused_variable_warning(sym);
        return QString("PolygonSymbolizer");
    }

    QString operator() (mapnik::polygon_pattern_symbolizer const& sym) const
    {
        boost::ignore_unused_variable_warning(sym);
        return QString("PolygonSymbolizer");
    }

    QString operator() (mapnik::text_symbolizer const& sym) const
    {
        boost::ignore_unused_variable_warning(sym);
        return QString("TextSymbolizer");
    }

    QString operator() (mapnik::shield_symbolizer const& sym) const
    {
        boost::ignore_unused_variable_warning(sym);
        return QString("ShieldSymbolizer");
    }

    QString operator() (mapnik::markers_symbolizer const& sym) const
    {
        boost::ignore_unused_variable_warning(sym);
        return QString("MarkersSymbolizer");
    }

    QString operator() (mapnik::building_symbolizer const& sym) const
    {
        boost::ignore_unused_variable_warning(sym);
        return QString("BuildingSymbolizer");
    }

    template <typename T>
    QString operator() (T const& ) const
    {
        return QString ("FIXME");
    }
};

struct symbolizer_icon
{
    QIcon operator() (mapnik::polygon_symbolizer const& sym) const
    {
        QPixmap pix(16,16);
        QPainter painter(&pix);
        mapnik::color const& fill = mapnik::get<mapnik::color>(sym, mapnik::keys::fill);
        QBrush brush(QColor(fill.red(),fill.green(),fill.blue(),fill.alpha()));
        painter.fillRect(0, 0, 16, 16, brush);
        return QIcon(pix);
    }

    QIcon operator() (mapnik::point_symbolizer const& sym) const
    {
        // FIXME!
        /*
          std::shared_ptr<mapnik::image_rgba8> symbol = sym.get_image();
          if (symbol)
          {
          QImage image(symbol->bytes(),
          symbol->width(),symbol->height(),QImage::Format_ARGB32);
          QPixmap pix = QPixmap::fromImage(image.rgbSwapped());
          return QIcon(pix);
          }
        */
        return QIcon();
    }
    QIcon operator() (mapnik::line_symbolizer const& sym) const
    {
        QPixmap pix(48,16);
        pix.fill();
        QPainter painter(&pix);
        //mapnik::stroke const&  strk = sym.get_stroke();
        mapnik::color const& col = mapnik::get<mapnik::color>(sym, mapnik::keys::stroke);
        QPen pen(QColor(col.red(),col.green(),col.blue(),col.alpha()));
        pen.setWidth(mapnik::get<double>(sym, mapnik::keys::width));
        painter.setPen(pen);
        painter.drawLine(0,7,47,7);
        //painter.drawLine(7,15,12,0);
        //painter.drawLine(12,0,8,15);
        return QIcon(pix);
    }

    template <typename T>
    QIcon operator() (T const& ) const
    {
        return QIcon (":/images/filter.png");
    }
};

class symbolizer_node
{
public:
    symbolizer_node(mapnik::symbolizer const & sym)
        : sym_(sym) {}
    ~symbolizer_node(){}

    QString name() const
    {
        //return QString("Symbolizer:fixme");
        return mapnik::util::apply_visitor(symbolizer_info(),sym_);
    }

    QIcon icon() const
    {
        return mapnik::util::apply_visitor(symbolizer_icon(),sym_);//QIcon(":/images/filter.png");
    }
    mapnik::symbolizer const& sym_;
};

class rule_node
{
public:
    rule_node(QString name,mapnik::rule const & r)
    : name_(name),
      rule_(r) {}
    ~rule_node() {}
    QString name() const
    {
        mapnik::expression_ptr filter = rule_.get_filter();
        return QString(mapnik::to_expression_string(*filter).c_str());
    }

    QIcon icon() const
    {
        return QIcon(":/images/filter.png");
    }

private:
    QString name_;
    mapnik::rule const& rule_;
};

class style_node
{
public:
    style_node(QString name, mapnik::feature_type_style const& style)
    : name_(name),
      style_(style) {}

    ~style_node() {}

    QString name() const
    {
        return name_;
    }

    QIcon icon() const
    {
        return QIcon(":/images/style.png");
    }

private:
    QString name_;
    mapnik::feature_type_style const& style_;
};

class map_node
{
public:
    explicit map_node(std::shared_ptr<mapnik::Map> map)
    : map_(map)  {}
    ~map_node() {}

    QString name() const
    {
        return QString("Map");
    }

    QIcon icon() const
    {
        return QIcon(":/images/map.png");
    }

private:
    std::shared_ptr<mapnik::Map> map_;
};

StyleModel::StyleModel(std::shared_ptr<mapnik::Map> map, QObject * parent)
    : QAbstractItemModel(parent),
      root_(new node(map_node(map)))
{
    using style_type = std::map<std::string,mapnik::feature_type_style>;
    style_type const & styles = map->styles();
    style_type::const_iterator itr = styles.begin();
    style_type::const_iterator end = styles.end();
    for (; itr != end; ++itr)
    {
        node * style_n = root_->add_child(new node(style_node(QString(itr->first.c_str()),itr->second),root_.get()));
        mapnik::rules const& rules = itr->second.get_rules();
        mapnik::rules::const_iterator itr2 = rules.begin();
        for ( ; itr2 != rules.end();++itr2)
        {
            node* rule_n = style_n->add_child(new node(rule_node(QString("Rule"),*itr2),style_n));
            mapnik::rule::symbolizers::const_iterator itr3 = (*itr2).begin();
            for ( ; itr3 !=itr2->end();++itr3)
            {
            rule_n->add_child(new node(symbolizer_node(*itr3),rule_n));
            }
        }
    }
}

StyleModel::~StyleModel() {}

// interface
QModelIndex StyleModel::index (int row, int col, QModelIndex const& parent) const
{
//   qDebug("index() row=%d col=%d parent::internalId() = %lld", row,col,parent.internalId());
    node * parent_node;

    if (!parent.isValid())
        parent_node = root_.get();
    else
        parent_node = static_cast<node*>(parent.internalPointer());

    node * child_node = parent_node->child(row);
    if (child_node)
        return createIndex(row,col,child_node);
    else
        return QModelIndex();
}

QModelIndex StyleModel::parent (QModelIndex const& index) const
{
    node * child_node = static_cast<node*>(index.internalPointer());
    node * parent_node = child_node->parent();
    if (parent_node == root_.get())
        return QModelIndex();

    return createIndex(parent_node->row(),0,parent_node);
}

int StyleModel::rowCount(QModelIndex const& parent) const
{
    //qDebug("rowCount");
    node * parent_node;
    if (parent.column() > 0) return 0;
    if (!parent.isValid())
        parent_node = root_.get();
    else
        parent_node = static_cast<node*>(parent.internalPointer());
    return parent_node->num_children();
}

int StyleModel::columnCount( QModelIndex const&) const
{
    return 1;
}

QVariant StyleModel::data(const QModelIndex & index, int role) const
{
    //qDebug("data index::internalId() = %lld", index.internalId());
    if (!index.isValid())
        return QVariant();
    node * cur_node = static_cast<node*>(index.internalPointer());
    if (cur_node)
    {
        if (role == Qt::DisplayRole)
        {

            return QVariant(cur_node->name());
        }
        else if ( role == Qt::DecorationRole)
        {
            return cur_node->icon();
        }
    }
    return QVariant();
}
