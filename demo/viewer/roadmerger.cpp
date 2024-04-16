#include "roadmerger.h"
#include <mapnik/map.hpp>
#include <mapnik/layer.hpp>
#include <mapnik/rule.hpp>
#include <mapnik/feature_type_style.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/text/placements/dummy.hpp>
#include <mapnik/text/text_properties.hpp>
#include <mapnik/text/formatting/text.hpp>
#include <mapnik/datasource_cache.hpp>
#include <mapnik/font_engine_freetype.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/expression.hpp>
#include <mapnik/color_factory.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/save_map.hpp>
#include <mapnik/cairo_io.hpp>
#include <mapnik/memory_datasource.hpp>
#include <mapnik/safe_cast.hpp>
#include <mapnik/geometry/boost_adapters.hpp>
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/register/multi_linestring.hpp>
#include <boost/geometry/multi/geometries/register/multi_polygon.hpp>
#include <mapnik/feature_factory.hpp>
#include <mapnik/util/timer.hpp>

#include <chrono>
#include "ThreadPool.h"

//BOOST_GEOMETRY_REGISTER_MULTI_LINESTRING(mapnik::geometry::multi_line_string<double>)
//BOOST_GEOMETRY_REGISTER_MULTI_POLYGON(mapnik::geometry::multi_polygon<double>);

using namespace mapnik;

typedef std::shared_ptr<geometry::multi_polygon<double>> MultiPolygonPtr;

#define MERGE_RESULT "MERGE_RESULT"

#define NeedAddCehui_RESULT "NeedAddCehui_RESULT"

// 计算缓冲区
static MultiPolygonPtr bufferGeom(geometry::geometry<double>& geom,double buffer_distance = 0.001){
    const int points_per_circle = buffer_distance;
    boost::geometry::strategy::buffer::distance_symmetric<double> distance_strategy(buffer_distance);
    boost::geometry::strategy::buffer::join_round join_strategy(points_per_circle);
    boost::geometry::strategy::buffer::end_round end_strategy(points_per_circle);
    boost::geometry::strategy::buffer::point_circle circle_strategy(points_per_circle);
    boost::geometry::strategy::buffer::side_straight side_strategy;

    auto mp = std::make_shared<geometry::multi_polygon<double>>();
    if( geom.is<geometry::line_string<double>>() ){
        auto line = geom.get<geometry::line_string<double>>();
        boost::geometry::buffer(line,*mp.get(),
                                distance_strategy, side_strategy,
                                join_strategy, end_strategy, circle_strategy);
    }
    else if( geom.is<geometry::multi_line_string<double>>() ){
        auto line = geom.get<geometry::multi_line_string<double>>();
        boost::geometry::buffer(line,*mp.get(),
                                distance_strategy, side_strategy,
                                join_strategy, end_strategy, circle_strategy);
    }
    return mp;
}

// 路段剪裁
static void clipLine(geometry::geometry<double>& in,std::vector<mapnik::geometry::multi_polygon<double>>& buffers,geometry::geometry<double>& out){
    if( in.is<geometry::multi_line_string<double>>()){
//        auto line = in.get<geometry::multi_line_string<double>>();
//        for(auto it = buffers.begin(); it != buffers.end(); it++){
//            mapnik::geometry::multi_polygon<double>& poly = *it;
//            std::deque<mapnik::geometry::polygon<double>> result;
//            mapnik::geometry::line_string<double> outline;
//            boost::geometry::intersection(line,poly,outline);
//            if( !outline.empty() ){
//                std::cout << "*************" << std::endl;
//            }
//        }
    }
    else{
        auto line = in.get<geometry::line_string<double>>();
        mapnik::geometry::multi_line_string<double> result;
        for(auto it = buffers.begin(); it != buffers.end(); it++){
            mapnik::geometry::multi_polygon<double>& poly = *it;
            mapnik::geometry::multi_line_string<double> outline;
            if( boost::geometry::within(line,poly) ){
                result.push_back(line);
            }
        }
        if( !result.empty() ){
            out.set<mapnik::geometry::multi_line_string<double>>(result);
        }
    }
}

// 读取shp数据源
static std::shared_ptr<datasource> readShp(const std::string& path){
    parameters p;
    p["type"] = "shape";
    p["file"] = path;
    return datasource_cache::instance().create(p);
}

//***************************************
// RoadMerger
//***************************************
RoadMerger::RoadMerger(MapWidget& mapWidget_):QThread(),
   mapWidget(mapWidget_)
{
    parameters p;
    clipedBaseSource = std::make_shared<memory_datasource>(p);
    cehuiBufferSource = std::make_shared<memory_datasource>(p);
    mergedSource = std::make_shared<memory_datasource>(p);
    clipedCehuiSource = std::make_shared<memory_datasource>(p);
    selectedResultBufferSource = std::make_shared<memory_datasource>(p);

    m_mergedSourceIndex = -1;
    m_clipedCehuiSourceIndex =-1;
}


void RoadMerger::run()
{
    emit signalMergeStart();

    std::shared_ptr<datasource> baseDS = readShp(baseShp.toStdString());
    std::shared_ptr<datasource> cehuiDS = readShp(cehuiShp.toStdString());

    // 生成测绘数据buffer
    std::vector<geometry::multi_polygon<double>> cehuiBuffer;
    {
        ThreadPool pool(16);
        std::vector< std::future<MultiPolygonPtr> > results;
        mapnik::auto_cpu_timer t(std::clog, "生成测绘数据buffer took: ");

        query q(cehuiDS->envelope());
        auto fs = cehuiDS->features(q);
        feature_ptr feat = fs->next();
        while(feat){
            results.emplace_back(pool.enqueue([&,feat] {

                auto res = bufferGeom(feat->get_geometry());
                return res;

            }));
            feat = fs->next();
        }
        for(auto && result: results){
            cehuiBuffer.push_back(*result.get());
        }
    }
    for(int i = 0; i < cehuiBuffer.size(); i++){
        feature_ptr feature(feature_factory::create(std::make_shared<mapnik::context_type>(), 1));
        feature->set_geometry(mapnik::geometry::geometry<double>(cehuiBuffer[i]));
        cehuiBufferSource->push(feature);
    }

    //
    query q(cehuiDS->envelope());
    q.add_property_name("OSMID");
    auto fs = baseDS->features(q);
    feature_ptr feat = fs->next();

    {
        mapnik::auto_cpu_timer t(std::clog, "剪裁测绘数据buffer took: ");
        ThreadPool pool(16);
        std::vector< std::future<feature_ptr> > results;
        int count = 1;
        while(feat){
            feature_ptr cloneFeat = feat;
            results.emplace_back(pool.enqueue([&,cloneFeat] {
                mapnik::geometry::geometry<double> out;
                clipLine(cloneFeat->get_geometry(),cehuiBuffer,out);
                feature_ptr feature(feature_factory::create(std::make_shared<mapnik::context_type>(), count));
                feature->put_new("OSMID",cloneFeat->get("OSMID"));
                feature->put_new("MERGE_RESULT",1);
                feature->set_geometry(mapnik::geometry::geometry<double>(out));
                return feature;
            }));
            clipedBaseSource->push(feat);
            feat = fs->next();
        }
        for(auto && result: results){
            mergedSource->push(result.get());
        }
    }
    emit signalMergeEnd();
}

// 启动融合过程
void RoadMerger::merge(QString const& base,QString const& cehui){
    baseShp = base;
    cehuiShp = cehui;
    unsigned width = mapWidget.width();
    unsigned height = mapWidget.height();
    std::shared_ptr<mapnik::Map> map(new mapnik::Map(width, height));
    mapWidget.setMap(map);
    start();
}

/**
 * @brief 显示路网图层
 */
void RoadMerger::showRoadLayers()
{
    // 显示基础路网数据
    addLineLayer("base",clipedBaseSource,"#bbbbbb");

    // 显示测绘路网数据
    addLineLayer("cehui",cehuiShp,"green",2.0);

    // 显示融合图层
    addMergedLayer();

    mapWidget.zoomAll();

    m_mergedSourceIndex = 2;
    m_clipedCehuiSourceIndex =-1;
}

void RoadMerger::clearLayers()
{
    mapWidget.getMap()->remove_all();
    m_mergedSourceIndex = -1;
    m_clipedCehuiSourceIndex =-1;
}

void RoadMerger::showClipedCehuiOnMap()
{
    // 显示基础路网数据
    addLineLayer("base",clipedBaseSource,"#bbbbbb");

    addSelectedResultBufferLayer();

    // 显示测绘路网数据
    addLineLayer("cehui",cehuiShp,"green",2.0);

    addClipedCehuiLayer();

    // 显示融合图层
    addMergedLayer();

    mapWidget.zoomAll();
    m_clipedCehuiSourceIndex =3;
    m_mergedSourceIndex = 4;
}

void RoadMerger::exportCompleteRoads(const QString& completeRoadsFile)
{

}

void RoadMerger::addLineLayer(QString const& name,QString const& shpPath,std::string lineColor, double lineWidth)
{
    parameters p;
    p["type"] = "shape";
    p["file"] = shpPath.toStdString();
    auto ds = datasource_cache::instance().create(p);
    addLineLayer(name,ds,lineColor,lineWidth);
}

void RoadMerger::addLineLayer(QString const& name,std::shared_ptr<mapnik::datasource> ds,std::string lineColor, double lineWidth)
{
    feature_type_style line_style;
    {
        rule r;
        //r.set_filter(parse_expression("[KIND_NUM] = '1'"));
        {
            line_symbolizer line_sym;
            put(line_sym, keys::stroke, color(lineColor));
            put(line_sym, keys::stroke_width, lineWidth);
            put(line_sym, keys::stroke_linecap, mapnik::line_cap_enum::ROUND_CAP);
            put(line_sym, keys::stroke_linejoin, mapnik::line_join_enum::ROUND_JOIN);
            r.append(std::move(line_sym));
        }
        line_style.add_rule(std::move(r));
    }
    mapWidget.getMap()->insert_style(name.toStdString(), std::move(line_style));

    layer lyr(name.toStdString());
    lyr.set_datasource(ds);
    lyr.add_style(name.toStdString());
    mapWidget.getMap()->add_layer(lyr);
}

void RoadMerger::addPolygonLayer(QString const& name,std::shared_ptr<mapnik::datasource> ds,mapnik::color stroke,mapnik::color fill)
{
    feature_type_style poly_style;
    {
        rule r;
        //r.set_filter(parse_expression("[KIND_NUM] = '1'"));
        {
            line_symbolizer line_sym;
            put(line_sym, keys::stroke, color(stroke));
            put(line_sym, keys::stroke_width, 1.0);
            put(line_sym, keys::stroke_linecap, mapnik::line_cap_enum::ROUND_CAP);
            put(line_sym, keys::stroke_linejoin, mapnik::line_join_enum::ROUND_JOIN);
            r.append(std::move(line_sym));
        }

        {
            polygon_symbolizer poly_sym;
            put(poly_sym,keys::fill,color(fill));
            r.append(std::move(poly_sym));
        }
        poly_style.add_rule(std::move(r));
    }
    mapWidget.getMap()->insert_style(name.toStdString(), std::move(poly_style));

    layer lyr(name.toStdString());
    lyr.set_datasource(ds);
    lyr.add_style(name.toStdString());
    mapWidget.getMap()->add_layer(lyr);
}

void RoadMerger::addMergedLayer()
{
    feature_type_style poly_style;
    {
        rule r;
        r.set_filter(parse_expression("[MERGE_RESULT] = 1"));
        {
            line_symbolizer line_sym;
            put(line_sym, keys::stroke, color("red"));
            put(line_sym, keys::stroke_width, 1.0);
            put(line_sym, keys::stroke_linecap, mapnik::line_cap_enum::ROUND_CAP);
            put(line_sym, keys::stroke_linejoin, mapnik::line_join_enum::ROUND_JOIN);
            r.append(std::move(line_sym));
        }
        poly_style.add_rule(std::move(r));
    }
    {
        rule r;
        r.set_filter(parse_expression("[MERGE_RESULT] = 0"));
        {
            line_symbolizer line_sym;
            put(line_sym, keys::stroke, color("black"));
            put(line_sym, keys::stroke_width, 1.0);
            put(line_sym, keys::stroke_linecap, mapnik::line_cap_enum::ROUND_CAP);
            put(line_sym, keys::stroke_linejoin, mapnik::line_join_enum::ROUND_JOIN);
            r.append(std::move(line_sym));
        }
        poly_style.add_rule(std::move(r));
    }
    mapWidget.getMap()->insert_style("merged_layer", std::move(poly_style));

    layer lyr("merged_layer");
    lyr.set_datasource(mergedSource);
    lyr.add_style("merged_layer");
    mapWidget.getMap()->add_layer(lyr);
}

void RoadMerger::addClipedCehuiLayer()
{
    feature_type_style poly_style;
    {
        rule r;
        r.set_filter(parse_expression("[NeedAddCehui_RESULT] = 1"));
        {
            line_symbolizer line_sym;
            put(line_sym, keys::stroke, color("yellow"));
            put(line_sym, keys::stroke_width, 2.0);
            put(line_sym, keys::stroke_linecap, mapnik::line_cap_enum::ROUND_CAP);
            put(line_sym, keys::stroke_linejoin, mapnik::line_join_enum::ROUND_JOIN);
            r.append(std::move(line_sym));
        }
        poly_style.add_rule(std::move(r));
    }
    {
        rule r;
        r.set_filter(parse_expression("[NeedAddCehui_RESULT] = 0"));
        {
            line_symbolizer line_sym;
            put(line_sym, keys::stroke, color("blue"));
            put(line_sym, keys::stroke_width, 2.0);
            put(line_sym, keys::stroke_linecap, mapnik::line_cap_enum::ROUND_CAP);
            put(line_sym, keys::stroke_linejoin, mapnik::line_join_enum::ROUND_JOIN);
            r.append(std::move(line_sym));
        }
        poly_style.add_rule(std::move(r));
    }
    mapWidget.getMap()->insert_style("needAddCehui_layer", std::move(poly_style));

    layer lyr("needAddCehui_layer");
    lyr.set_datasource(clipedCehuiSource);
    lyr.add_style("needAddCehui_layer");
    mapWidget.getMap()->add_layer(lyr);
}

void RoadMerger::addSelectedResultBufferLayer()
{
    addPolygonLayer("selectedResultBufferSource",selectedResultBufferSource, color(0,0,200), color(120,120,120,100));
}

void RoadMerger::toggleMergedRoad(double x, double y)
{
    std::cout<<"begin toggleMergedRoad"<<std::endl;
    if(m_mergedSourceIndex==-1)
    {
        std::cout<<"end toggleMergedRoad"<<std::endl;
        return;
    }

    auto map = mapWidget.getMap();
    mapnik::featureset_ptr fs = map->query_map_point(m_mergedSourceIndex, x, y);
    if (fs)
    {
        feature_ptr feat = fs->next();
        while (feat)
        {
            auto val = feat->get("MERGE_RESULT") == 1 ? 0 : 1;
            feat->put("MERGE_RESULT",val);

            feat = fs->next();
            std::cout<<"toggle road "<<std::endl;
        }
    }
    mapWidget.updateMap();
    std::cout<<"end toggleMergedRoad"<<std::endl;
}

void RoadMerger::toggleNeedCompleteRoad(double x, double y)
{
    std::cout<<"begin toggleNeedCompleteRoad"<<std::endl;
    if(m_clipedCehuiSourceIndex==-1)
    {
        std::cout<<"end toggleNeedCompleteRoad"<<std::endl;
        return;
    }

    auto map = mapWidget.getMap();
    mapnik::featureset_ptr fs = map->query_map_point(m_clipedCehuiSourceIndex, x, y);
    if (fs)
    {
        feature_ptr feat = fs->next();
        while (feat)
        {
            auto val = feat->get("NeedAddCehui_RESULT") == 1 ? 0 : 1;
            feat->put("NeedAddCehui_RESULT",val);

            feat = fs->next();
            std::cout<<"toggle complete road "<<std::endl;
        }
    }
    mapWidget.updateMap();
    std::cout<<"end toggleNeedCompleteRoad"<<std::endl;
}

void RoadMerger::getMergeResult(std::vector<long>& result)
{
    query q(mergedSource->envelope());
    q.add_property_name("MERGE_RESULT");
    q.add_property_name("OSMID");
    auto fs = mergedSource->features(q);
    feature_ptr feat = fs->next();
    while(feat){
        if( feat->get("MERGE_RESULT") == 1 ){
            result.push_back(feat->get("OSMID").to_int());
        }
        feat = fs->next();
    }
}

void RoadMerger::generateResultBuffer(std::vector<mapnik::geometry::multi_polygon<double>>& out){
    // 生成results数据buffer
    {
        ThreadPool pool(16);
        std::vector< std::future<MultiPolygonPtr> > results;
        mapnik::auto_cpu_timer t(std::clog, "生成result数据buffer took: ");
        query q(mergedSource->envelope());
        q.add_property_name("MERGE_RESULT");
        auto fs = mergedSource->features(q);
        feature_ptr feat = fs->next();
        while(feat){
            if( feat->get("MERGE_RESULT") == 1 )
            {
                results.emplace_back(pool.enqueue([&,feat] {

                    auto res = bufferGeom(feat->get_geometry());
                    return res;

                }));
            }

            feat = fs->next();
        }
        for(auto && result: results){
            out.push_back(*result.get());
        }
    }
}


void RoadMerger::clipedLineEx(mapnik::geometry::geometry<double>& in,
                              std::vector<mapnik::geometry::multi_polygon<double>>& buffers,
                              mapnik::geometry::geometry<double>& out)
{
    if( in.is<geometry::multi_line_string<double>>()){
        auto line = in.get<geometry::multi_line_string<double>>();
        mapnik::geometry::multi_line_string<double> result;
        for(auto it = buffers.begin(); it != buffers.end(); it++){
            mapnik::geometry::multi_polygon<double>& poly = *it;
            mapnik::geometry::line_string<double> outline;
            boost::geometry::intersection(line,poly,outline);
//            boost::geometry::difference(line,poly,outline);
            if( !outline.empty() ){
                std::cout << "*************" << std::endl;
                result.push_back(outline);
            }
        }

        if( !result.empty() ){
            out.set<mapnik::geometry::multi_line_string<double>>(result);
        }
    }
    else if(in.is<geometry::line_string<double>>()){
        auto line = in.get<geometry::line_string<double>>();
        mapnik::geometry::multi_line_string<double> result;
        for(auto it = buffers.begin(); it != buffers.end(); it++){
            mapnik::geometry::multi_polygon<double>& poly = *it;
            mapnik::geometry::line_string<double> outline;
            boost::geometry::intersection(line,poly,outline);
//            boost::geometry::difference(line,poly,outline);
            if( !outline.empty() ){
                std::cout << "*************" << std::endl;
                result.push_back(outline);
            }
        }
        if( !result.empty() ){
            out.set<mapnik::geometry::multi_line_string<double>>(result);
        }
    }
}

void RoadMerger::clipedCehuiData()
{
    std::vector<mapnik::geometry::multi_polygon<double>> mergedResultBuffer;
    generateResultBuffer(mergedResultBuffer);
    if(mergedResultBuffer.size())
    {
        std::shared_ptr<datasource> cehuiDS = readShp(cehuiShp.toStdString());

        //
        query q(cehuiDS->envelope());
        q.add_property_name("OSMID");
        auto fs = cehuiDS->features(q);
        feature_ptr feat = fs->next();

        {
            ThreadPool pool(16);
            std::vector< std::future<std::vector<feature_ptr> > > results;
            int count = 1;
            while(feat){
                feature_ptr cloneFeat = feat;
                results.emplace_back(pool.enqueue([&,cloneFeat] {
                    mapnik::geometry::geometry<double> out;
                    clipedLineEx(cloneFeat->get_geometry(),mergedResultBuffer,out);
                    std::vector<feature_ptr> result;
                    if(out.is<geometry::multi_line_string<double>>())
                    {
                       mapnik::geometry::multi_line_string<double> lines = out.get<geometry::multi_line_string<double>>();
                       for(int i=0; i<lines.size(); i++)
                       {
                           feature_ptr feature(feature_factory::create(std::make_shared<mapnik::context_type>(), count));
                           feature->put_new("OSMID",cloneFeat->get("OSMID"));
                           feature->put_new("NeedAddCehui_RESULT",1);
                           feature->set_geometry(mapnik::geometry::geometry<double>(lines.at(i)));
                           result.emplace_back(feature);
                       }
                    }

                    return result;
                }));
                feat = fs->next();
            }

            for(auto && result: results)
            {
                std::vector<feature_ptr> featureVector = result.get();
                for(auto& feature:featureVector)
                {
                    clipedCehuiSource->push(feature);
                }

            }
        }


        //test code:
        for(auto& buffer:mergedResultBuffer)
        {
            feature_ptr feature(feature_factory::create(std::make_shared<mapnik::context_type>(), 1));
            feature->put_new("mergedResultBuffer",1);
            feature->set_geometry(mapnik::geometry::geometry<double>(buffer));
            selectedResultBufferSource->push(feature);
        }

    }
}
