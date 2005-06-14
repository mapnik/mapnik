/* This file is part of Mapnik (c++ mapping toolkit)
 * Copyright (C) 2005 Artem Pavlenko
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

//$Id: map.hpp 39 2005-04-10 20:39:53Z pavlenko $

#ifndef MAP_HPP
#define MAP_HPP

namespace mapnik
{
    class Layer;

    class Map
    {
    private:
	static const int MIN_MAPSIZE=16;
	static const int MAX_MAPSIZE=1024;
	int width_;
	int height_;
	int srid_;
	Color background_;
	std::vector<Layer> layers_;
	Envelope<double> currentExtent_;
    public:
	Map(int width,int height,int srid=-1);
	Map(const Map& rhs);
	Map& operator=(const Map& rhs);
	size_t layerCount() const;
	void addLayer(const Layer& l);
	const Layer& getLayer(size_t index) const;
	void removeLayer(size_t index);
	void removeLayer(const char* lName);
	int getWidth() const;
	int getHeight() const;
	int srid() const;
	void setBackground(const Color& c);
	const Color& getBackground() const;
	void zoom(double zoom);
	void zoomToBox(const Envelope<double>& box);
	void pan(int x,int y);
	void pan_and_zoom(int x,int y,double zoom);
	const Envelope<double>& getCurrentExtent() const;
	double scale() const;
	virtual ~Map();
    private:
	void fixAspectRatio();
    };
}
#endif //MAP_HPP
