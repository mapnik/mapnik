
#include "catch.hpp"

#include <mapnik/sql_utils.hpp>

TEST_CASE("sql parse") {

SECTION("table") {
    std::string subquery("table");
    REQUIRE( subquery == mapnik::sql_utils::table_from_sql(subquery) );
}

SECTION("complex sql 1") {
    std::string subquery("(select * FROM table1, table2) AS data");
    REQUIRE( "table1" == mapnik::sql_utils::table_from_sql(subquery) );
}

SECTION("complex sql 2") {
    std::string subquery("(select * FROM table1 , table2) AS data");
    REQUIRE( "table1" == mapnik::sql_utils::table_from_sql(subquery) );
}

SECTION("complex sql 3") {
    std::string subquery("(select * FROM table1,table2) AS data");
    REQUIRE( "table1" == mapnik::sql_utils::table_from_sql(subquery) );
}

SECTION("complex sql 4") {
    std::string subquery("(select * FROM table1) AS data");
    REQUIRE( "table1" == mapnik::sql_utils::table_from_sql(subquery) );
}

}
