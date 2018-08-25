#ifndef COASTLINE_HANDLERS_HPP
#define COASTLINE_HANDLERS_HPP

/*

  Copyright 2012-2018 Jochen Topf <jochen@topf.org>.

  This file is part of OSMCoastline.

  OSMCoastline is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  OSMCoastline is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OSMCoastline.  If not, see <https://www.gnu.org/licenses/>.

*/

#include "coastline_ring_collection.hpp"
#include "output_database.hpp"

#include <osmium/handler.hpp>
#include <osmium/geom/ogr.hpp>

#include <iostream>
#include <utility>

/**
 * Osmium handler for the first pass over the input file in which
 * all ways tagged with 'natural=coastline' are read and CoastlineRings
 * are created.
 */
class CoastlineHandlerPass1 : public osmium::handler::Handler {

    CoastlineRingCollection& m_coastline_rings;

public:

    explicit CoastlineHandlerPass1(CoastlineRingCollection& coastline_rings) :
        m_coastline_rings(coastline_rings) {
    }

    void way(const osmium::Way& way) {
        // We are only interested in ways tagged with natural=coastline.
        if (way.tags().has_tag("natural", "coastline")) {
            if (way.tags().has_tag("coastline", "bogus")) {
                return; // ignore bogus coastline in Antarctica
            }
            m_coastline_rings.add_way(way);
        }
    }

}; // class CoastlineHandlerPass1

/**
 * Osmium handler for the second pass over the input file in which
 * node locations are added to the CoastlineRings.
 */
class CoastlineHandlerPass2 : public osmium::handler::Handler {

    CoastlineRingCollection& m_coastline_rings;

    /**
     * Multimap for a mapping from node ID to all places where the
     * position of this node should be written to. Those places
     * are in the CoastlineRings created from the ways. This map
     * is set up first thing when the handler is instantiated and
     * thereafter used for each node coming in.
     */
    posmap_type m_posmap{};
    OutputDatabase& m_output;
    osmium::geom::OGRFactory<> m_factory;

public:

    CoastlineHandlerPass2(CoastlineRingCollection& coastline_rings, OutputDatabase& output) :
        m_coastline_rings(coastline_rings),
        m_output(output) {
        m_coastline_rings.setup_positions(m_posmap);
    }

    void node(const osmium::Node& node) {
        if (node.tags().has_tag("natural", "coastline")) {
            try {
                m_output.add_error_point(m_factory.create_point(node), "tagged_node", node.id());
            } catch (const osmium::geometry_error&) {
                std::cerr << "Ignoring illegal geometry for node " << node.id() << ".\n";
            }
        }

        const auto ret = m_posmap.equal_range(node.id());
        for (auto it = ret.first; it != ret.second; ++it) {
            *(it->second) = node.location();
        }
    }

}; // class CoastlineHandlerPass2

#endif // COASTLINE_HANDLERS_HPP
