#include "catch.hpp"

#include <mapnik/debug.hpp>
#include <mapnik/value.hpp>
#include <mapnik/xml_tree.hpp>
#include <mapnik/xml_loader.hpp>
#include <mapnik/attribute.hpp>

TEST_CASE("xml parser") {

  SECTION("trims whitespace") {

    // simple and non-valid mapnik XML reduced from the empty_parameter2.xml
    // test case. this is to check that the xml parsing routine is trimming
    // whitespace from text nodes as part of the parsing operation.
    const std::string xml("<Map>"
                          "  <Layer>"
                          "    <Datasource>"
                          "      <Parameter name=\"empty\"><![CDATA[ ]]></Parameter>"
                          "    </Datasource>"
                          "  </Layer>"
                          "</Map>");

    mapnik::xml_tree tree;
    tree.set_filename("xml_datasource_parameter_trim.cpp");
    REQUIRE_NOTHROW(read_xml_string(xml, tree.root(), ""));

    REQUIRE(tree.root().has_child("Map"));
    mapnik::xml_node const &map = tree.root().get_child("Map");

    REQUIRE(map.has_child("Layer"));
    mapnik::xml_node const &layer = map.get_child("Layer");

    REQUIRE(layer.has_child("Datasource"));
    mapnik::xml_node const &datasource = layer.get_child("Datasource");

    REQUIRE(datasource.has_child("Parameter"));
    mapnik::xml_node const &parameter = datasource.get_child("Parameter");

    // parser should call mapnik::util::trim on the text content and
    // this should result in an empty text string in the parameter.
    REQUIRE(parameter.get_text() == "");
  }
}

