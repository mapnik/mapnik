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
#include <QSettings>
#include <QDebug>

#include <chrono>
#include "ThreadPool.h"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include <iostream>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <unicode/unistr.h> // ICU的头文件

//BOOST_GEOMETRY_REGISTER_MULTI_LINESTRING(mapnik::geometry::multi_line_string<double>)
//BOOST_GEOMETRY_REGISTER_MULTI_POLYGON(mapnik::geometry::multi_polygon<double>);

using namespace mapnik;

typedef std::shared_ptr<geometry::multi_polygon<double>> MultiPolygonPtr;


#ifndef  MERGE_RESULT
#define MERGE_RESULT "MERGE_RESULT"
#endif


#ifndef NeedAddCehui_RESULT
#define NeedAddCehui_RESULT "NeedAddCehui_RESULT"
#endif


#ifndef  IDKEY
#define  IDKEY "id"
#endif

#ifndef  NAMEKEY
#define  NAMEKEY "name"
#endif

#ifndef  DIRECTIONKEY
#define  DIRECTIONKEY "direction"
#endif


//// 合并相交或相互包含的多边形
//static void mergePolygons(std::vector<geometry::multi_polygon<double>>& polygons) {
//    bool merged = true;
//    while (merged) {
//        merged = false;
//        for (size_t i = 0; i < polygons.size(); ++i) {
//            for (size_t j = i + 1; j < polygons.size(); ++j) {
//                if (!boost::geometry::disjoint(polygons[i], polygons[j])) {
//                    // 合并多边形
//                    boost::geometry::union_(polygons[i], polygons[j], polygons[i]);
//                    polygons.erase(polygons.begin() + j);
//                    merged = true;
//                    break;
//                }
//            }
//            if (merged) {
//                break;
//            }
//        }
//    }
//}

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
RoadMerger::RoadMerger(MapWidget* mapWidget_):QThread(),
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
                context_ptr contextPtr = std::make_shared<mapnik::context_type>();
                feature_ptr feature(feature_factory::create(contextPtr, count));
                feature->put_new("OSMID",cloneFeat->get("OSMID"));
                feature->put_new(MERGE_RESULT,1);
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
    unsigned width = mapWidget->width();
    unsigned height = mapWidget->height();
    std::shared_ptr<mapnik::Map> map(new mapnik::Map(width, height));
    mapWidget->setMap(map);
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

    mapWidget->zoomAll();

    m_mergedSourceIndex = 2;
    m_clipedCehuiSourceIndex =-1;
}

void RoadMerger::clearLayers()
{
    mapWidget->getMap()->remove_all();
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

    mapWidget->zoomAll();
    m_clipedCehuiSourceIndex =3;
    m_mergedSourceIndex = 4;
}

std::string RoadMerger::convertToWKT(const mapnik::geometry::line_string<double>& lineString)
{
    // 将geometry::line_string<double>转换为WKT格式
    std::stringstream wktStream;
    wktStream << "LINESTRING (";
    for (const auto& point : lineString) {
        wktStream << point.x << " " << point.y << ",";
    }
    wktStream.seekp(-1, std::ios_base::end); // 移除最后一个逗号
    wktStream << ")";
    return wktStream.str();
}

std::string RoadMerger::convertToCustomText(const mapnik::geometry::line_string<double>& lineString)
{
    // 将geometry::line_string<double>转换为WKT格式
    std::stringstream wktStream;
    wktStream << "";
    for (const auto& point : lineString) {
        wktStream << point.x << "," << point.y << ";";
    }
    wktStream.seekp(-1, std::ios_base::end); // 移除最后一个逗号
    wktStream << "";
    return wktStream.str();
}

std::string RoadMerger::convertToWKT(const mapnik::geometry::multi_line_string<double>& multiLineString)
{
    std::stringstream wktStream;
    wktStream << "MULTILINESTRING (";
    for (const auto& segment : multiLineString) {
        wktStream << "(";
        for (const auto& point : segment) {
            wktStream << point.x << " " << point.y << ",";
        }
        wktStream.seekp(-1, std::ios_base::end); // 移除最后一个逗号
        wktStream << "),";
    }
    wktStream.seekp(-1, std::ios_base::end); // 移除最后一个逗号
    wktStream << ")";

    return wktStream.str();
}



bool RoadMerger::SerializeCompleteRoadInfos(const std::vector<cehuidataInfo>& result, const QString& groupId, const QString& completeRoadsFile)
{
    using namespace rapidjson;
    // 创建一个JSON对象
    Document document;
    document.SetObject();
    Document::AllocatorType& allocator = document.GetAllocator();

    Value jsonValue(kStringType);

    std::string strValue = groupId.toStdString();
    jsonValue.SetString(strValue.c_str(), strValue.size(),allocator);

    // 添加groupId
    document.AddMember("groupId", jsonValue, allocator);

    // 添加updateInfos
    Value updateInfos(kArrayType);
    Value updateInfo(kObjectType);
    updateInfo.AddMember("type", "add", allocator);

    // 添加fileCompletes
    Value fileCompletes(kArrayType);

    for(int i=0;i<result.size();++i)
    {

        const cehuidataInfo& info = result[i];
        Value fileComplete(kObjectType);

        strValue = info.ID;

        std::cout<<"info.ID: "<<strValue<<std::endl;

        strValue = info.NAME;
        jsonValue.SetString(strValue.c_str(), strValue.size(),allocator);
        fileComplete.AddMember("name", jsonValue, allocator);

        strValue = info.geometryWkt;
        jsonValue.SetString(strValue.c_str(), strValue.size(),allocator);
        fileComplete.AddMember("info", jsonValue, allocator);

        strValue = info.LEVEL;
        jsonValue.SetString(strValue.c_str(), strValue.size(),allocator);
        fileComplete.AddMember("level", jsonValue, allocator);

        strValue = info.WIDTH;
        jsonValue.SetString(strValue.c_str(), strValue.size(),allocator);
        fileComplete.AddMember("width", jsonValue, allocator);

        strValue = info.DIRECTION;
        jsonValue.SetString(strValue.c_str(), strValue.size(),allocator);
        fileComplete.AddMember("bidirectional", jsonValue, allocator);

        fileCompletes.PushBack(fileComplete, allocator);
    }

    updateInfo.AddMember("fileCompletes", fileCompletes, allocator);
    updateInfos.PushBack(updateInfo, allocator);

    document.AddMember("updateInfos", updateInfos, allocator);

    // 将JSON写入文件
    std::ofstream file(completeRoadsFile.toStdString());
    if (file.is_open())
    {
        // 将json字符串写入文件中
        StringBuffer buffer;
        Writer<StringBuffer> writer(buffer);
        document.Accept(writer);
        file << buffer.GetString();
        file.close();
        return true;
    }
    else
    {
      return false;
    }
}

void RoadMerger::getCompleteRoadsResult(std::vector<cehuidataInfo>& result)
{
    std::string idFieldName = m_cehuiKey2fieldName[IDKEY];
    std::string nameFieldName = m_cehuiKey2fieldName[NAMEKEY];
    std::string dirFieldName = m_cehuiKey2fieldName[DIRECTIONKEY];

    query q(clipedCehuiSource->envelope());
    q.add_property_name(NeedAddCehui_RESULT);
    q.add_property_name(idFieldName);
    q.add_property_name(nameFieldName);
    q.add_property_name(dirFieldName);
    auto fs = clipedCehuiSource->features(q);
    feature_ptr feat = fs->next();
    while(feat){
        if( feat->get(NeedAddCehui_RESULT) == 1 ){
            cehuidataInfo info;
            info.ID = feat->get(idFieldName).to_string();
            info.NAME = feat->get(nameFieldName).to_string();
            info.DIRECTION = feat->get(dirFieldName).to_string();
            geometry::geometry<double>& geo = feat->get_geometry();
            if(geo.is<geometry::line_string<double> >())
            {
                mapnik::geometry::line_string<double>& lineString = geo.get<geometry::line_string<double>>();
                info.geometryWkt = convertToCustomText(lineString);
            }
            else if(geo.is<geometry::multi_line_string<double>>())
            {
                mapnik::geometry::multi_line_string<double>& multilineString = geo.get<geometry::multi_line_string<double>>();
                info.geometryWkt = convertToWKT(multilineString);
            }

            result.push_back(info);
        }
        feat = fs->next();
    }
}

bool RoadMerger::exportCompleteRoads(const QString& completeRoadsFile, const QString& groupid)
{
    std::vector<cehuidataInfo> result;
    getCompleteRoadsResult(result);
    return SerializeCompleteRoadInfos(result, groupid, completeRoadsFile);
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
    mapWidget->getMap()->insert_style(name.toStdString(), std::move(line_style));

    layer lyr(name.toStdString());
    lyr.set_datasource(ds);
    lyr.add_style(name.toStdString());
    mapWidget->getMap()->add_layer(lyr);
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
    mapWidget->getMap()->insert_style(name.toStdString(), std::move(poly_style));

    layer lyr(name.toStdString());
    lyr.set_datasource(ds);
    lyr.add_style(name.toStdString());
    mapWidget->getMap()->add_layer(lyr);
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
    mapWidget->getMap()->insert_style("merged_layer", std::move(poly_style));

    layer lyr("merged_layer");
    lyr.set_datasource(mergedSource);
    lyr.add_style("merged_layer");
    mapWidget->getMap()->add_layer(lyr);
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
    mapWidget->getMap()->insert_style("needAddCehui_layer", std::move(poly_style));

    layer lyr("needAddCehui_layer");
    lyr.set_datasource(clipedCehuiSource);
    lyr.add_style("needAddCehui_layer");
    mapWidget->getMap()->add_layer(lyr);
}

void RoadMerger::addSelectedResultBufferLayer()
{
    addPolygonLayer("selectedResultBufferSource",selectedResultBufferSource, color(234,63,247,100), color(115,251,253,100));
}

void RoadMerger::toggleMergedRoad(double x, double y)
{
    std::cout<<"begin toggleMergedRoad"<<std::endl;
    if(m_mergedSourceIndex==-1)
    {
        std::cout<<"end toggleMergedRoad"<<std::endl;
        return;
    }

    auto map = mapWidget->getMap();
    mapnik::featureset_ptr fs = map->query_map_point(m_mergedSourceIndex, x, y);
    if (fs)
    {
        feature_ptr feat = fs->next();
        while (feat)
        {
            auto val = feat->get(MERGE_RESULT) == 1 ? 0 : 1;
            feat->put(MERGE_RESULT,val);

            feat = fs->next();
            std::cout<<"toggle road "<<std::endl;
        }
    }
    mapWidget->updateMap();
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

    auto map = mapWidget->getMap();
    mapnik::featureset_ptr fs = map->query_map_point(m_clipedCehuiSourceIndex, x, y);
    if (fs)
    {
        feature_ptr feat = fs->next();
        while (feat)
        {
            auto val = feat->get(NeedAddCehui_RESULT) == 1 ? 0 : 1;
            feat->put(NeedAddCehui_RESULT,val);

            feat = fs->next();
            std::cout<<"toggle complete road "<<std::endl;
        }
    }
    mapWidget->updateMap();
    std::cout<<"end toggleNeedCompleteRoad"<<std::endl;
}

void RoadMerger::getMergeResult(std::vector<long>& result)
{
    query q(mergedSource->envelope());
    q.add_property_name(MERGE_RESULT);
    q.add_property_name("OSMID");
    auto fs = mergedSource->features(q);
    feature_ptr feat = fs->next();
    while(feat){
        if( feat->get(MERGE_RESULT) == 1 ){
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
        q.add_property_name(MERGE_RESULT);
        auto fs = mergedSource->features(q);
        feature_ptr feat = fs->next();
        while(feat){
            if( feat->get(MERGE_RESULT) == 1 )
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
                              mapnik::geometry::multi_line_string<double>& out)
{
    mapnik::geometry::multi_point<double> allIntersection_points;
    if(in.is<geometry::line_string<double>>())
    {
        auto line = in.get<geometry::line_string<double>>();
        bool isDisjoin = true;
        mapnik::geometry::multi_line_string<double> result;
        for(auto it = buffers.begin(); it != buffers.end(); it++)
        {
            mapnik::geometry::multi_polygon<double>& multiPoly = *it;
            for(auto polyItor = multiPoly.begin(); polyItor != multiPoly.end(); polyItor++)
            {
                mapnik::geometry::polygon<double>& poly = *polyItor;
                if(!boost::geometry::disjoint(line, poly))
                {
                    isDisjoin = false;
                    if(boost::geometry::intersects(line, poly))
                    {
                        mapnik::geometry::multi_point<double> intersection_points;
                        boost::geometry::intersection(line, poly, intersection_points);
                        std::cout<<"intersection_points' count:"<< intersection_points.size() <<std::endl;
                        if(intersection_points.size())
                        {
                            for(int i=0;i<intersection_points.size();i++)
                            {
                                std::cout<<"intersection_points' count:"<< intersection_points.size() <<std::endl;
                                std::cout<<"intersection_points["<<i<<"] x:"<< intersection_points[i].x <<std::endl;
                                std::cout<<"intersection_points["<<i<<"] y:"<< intersection_points[i].y <<std::endl;
                                allIntersection_points.push_back(intersection_points[i]);
                            }
                        }

                    }
                }
            }


        }

        if(isDisjoin)
        {
            result.push_back(line);
            out = result;
            std::cout<<"in.is geometry::line_string. It is disjoint."<<std::endl;
            return;
        }


        if(allIntersection_points.size())
        {
            struct nodeInfo
            {
               mapnik::geometry::point<double> pos;
               bool isIntersectionPoint;
            };

            std::vector<nodeInfo> nodeList;
            for(auto& p:line)
            {
                nodeInfo node;
                node.pos = p;
                node.isIntersectionPoint = false;
                nodeList.push_back(node);
            }

            for(int j=0;j<allIntersection_points.size();++j)
            {
                mapnik::geometry::point<double>& cossPoint = allIntersection_points[j];
                for(int i=0;i<nodeList.size()-1;++i)
                {
                   geometry::line_string<double> curSeg;
                   curSeg.push_back(nodeList[i].pos);
                   curSeg.push_back(nodeList[i+1].pos);

                   if(boost::geometry::intersects(cossPoint, curSeg))
                   {
                       nodeInfo node = {cossPoint,true};
                       nodeList.insert(nodeList.begin()+i, node);
                       break;
                   }
                }
            }

            geometry::line_string<double> cutline;
            for(auto& node:nodeList)
            {
                if(node.isIntersectionPoint)
                {
                    cutline.push_back(node.pos);
                    if(cutline.size()>1)
                    {
                        bool isOut = true;
                        for(auto itor = buffers.begin(); itor != buffers.end(); itor++)
                        {
                            mapnik::geometry::multi_polygon<double>& polyCheck = *itor;
                            bool isStartIntersects = boost::geometry::intersects(cutline[0], polyCheck);
                            bool isEndIntersects = boost::geometry::intersects(cutline[cutline.size()-1], polyCheck);
                            bool isStartWithin = boost::geometry::within(cutline[0], polyCheck);
                            bool isEndWithin = boost::geometry::within(cutline[cutline.size()-1], polyCheck);

                            if(boost::geometry::within(cutline, polyCheck)
                               ||(isStartIntersects && isEndIntersects)
                               || (isStartWithin && isEndIntersects)
                               || (isStartIntersects && isEndWithin)
                               || (isStartWithin && isEndWithin))
                            {
                                isOut = false;
                                break;
                            }

//                            if(boost::geometry::within(cutline[0], polyCheck)
//                               || boost::geometry::intersects(cutline[0], polyCheck))
//                            {
//                                isOut = false;
//                                break;
//                            }

                        }

                        if(isOut)
                            result.push_back(cutline);
                    }

                    cutline.clear();
                    cutline.push_back(node.pos);
                }
                else
                {
                    cutline.push_back(node.pos);
                }

            }

            out = result;
            std::cout<<"result count:"<<result.size()<<std::endl;
        }

    }
    else
    {
        std::cout<<"has other geometry type"<<std::endl;
    }
}

void RoadMerger::OnItemCheckBoxChanged(const QString& id, int status)
{
    std::string idFieldName = m_cehuiKey2fieldName[IDKEY];
    std::cout<<"begin OnItemCheckBoxChanged"<<std::endl;
    query q(clipedCehuiSource->envelope());
    q.add_property_name(idFieldName);
    q.add_property_name(NeedAddCehui_RESULT);
    auto fs = clipedCehuiSource->features(q);
    feature_ptr feat = fs->next();
    while(feat){
        std::string strId = feat->get(idFieldName).to_string();
        if(strId==id.toStdString())
        {
            feat->put(NeedAddCehui_RESULT, status);
        }
        feat = fs->next();
    }

    mapWidget->updateMap();
    std::cout<<"end OnItemCheckBoxChanged"<<std::endl;
}

void RoadMerger::clipedCehuiData()
{
    std::string idFieldName = m_cehuiKey2fieldName[IDKEY];
    std::string nameFieldName = m_cehuiKey2fieldName[NAMEKEY];
    std::string dirFieldName = m_cehuiKey2fieldName[DIRECTIONKEY];

    qDebug() << "idFieldName:"<< QString::fromStdString(idFieldName);
    qDebug() << "nameFieldName:"<< QString::fromStdString(nameFieldName);
    qDebug() << "dirFieldName:"<< QString::fromStdString(dirFieldName);

    clipedCehuiSource->clear();
    selectedResultBufferSource->clear();

    std::vector<mapnik::geometry::multi_polygon<double>> mergedResultBuffer;
    generateResultBuffer(mergedResultBuffer);
    if(mergedResultBuffer.size())
    {
        std::shared_ptr<datasource> cehuiDS = readShp(cehuiShp.toStdString());

        //
        query q(cehuiDS->envelope());
        q.add_property_name(idFieldName);
        q.add_property_name(nameFieldName);
        q.add_property_name(dirFieldName);
        auto fs = cehuiDS->features(q);
        feature_ptr feat = fs->next();
        {
            ThreadPool pool(16);
            std::vector< std::future<std::vector<feature_ptr> > > results;
            int count = 1;
            while(feat){
                feature_ptr cloneFeat = feat;
                results.emplace_back(pool.enqueue([&,cloneFeat] {
                    mapnik::geometry::multi_line_string<double> out;
                    clipedLineEx(cloneFeat->get_geometry(),mergedResultBuffer,out);
                    std::vector<feature_ptr> result;
//                    if(out.is<geometry::multi_line_string<double>>())
                    {
//                       mapnik::geometry::multi_line_string<double> lines = out.get<geometry::multi_line_string<double>>();
                       mapnik::geometry::multi_line_string<double>& lines = out;
                       for(int i=0; i<lines.size(); i++)
                       {
                           context_ptr contextPtr = std::make_shared<mapnik::context_type>();
                           feature_ptr feature(feature_factory::create(contextPtr, count));
                           std::stringstream ss;
                           ss << cloneFeat->get(idFieldName).to_string() << "_"<<i;
                           std::string sID = ss.str();
                           icu::UnicodeString unicodeString = icu::UnicodeString::fromUTF8(icu::StringPiece(sID.c_str()));
                           mapnik::value val = unicodeString;
                           feature->put_new(idFieldName, val);
                           feature->put_new(nameFieldName,cloneFeat->get(nameFieldName));
                           feature->put_new(dirFieldName,cloneFeat->get(dirFieldName));
                           feature->put_new(NeedAddCehui_RESULT, 1);
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
            context_ptr contextPtr = std::make_shared<mapnik::context_type>();
            feature_ptr feature(feature_factory::create(contextPtr, 1));
            feature->put_new("mergedResultBuffer",1);
            feature->set_geometry(mapnik::geometry::geometry<double>(buffer));
            selectedResultBufferSource->push(feature);
        }

    }
}


void RoadMerger::loadCehuiTableFields(const QString& cehuiTableIniFilePath)
{
    m_cehuiKey2fieldName.clear();
    QSettings settings(cehuiTableIniFilePath, QSettings::IniFormat);
    // 读取数据表字段名配置
    settings.beginGroup("cehuidata");
    QString idField = settings.value(IDKEY, "id").toString();
    QString nameField = settings.value(NAMEKEY, "name").toString();
    QString directionField = settings.value(DIRECTIONKEY, "direction").toString();
    settings.endGroup();

    m_cehuiKey2fieldName.insert(std::make_pair<std::string, std::string>(IDKEY, idField.toStdString()));
    m_cehuiKey2fieldName.insert(std::make_pair<std::string, std::string>(NAMEKEY, nameField.toStdString()));
    m_cehuiKey2fieldName.insert(std::make_pair<std::string, std::string>(DIRECTIONKEY, directionField.toStdString()));

    // 输出读取的配置
    qDebug() << "idField:" << idField;
    qDebug() << "nameField:" << nameField;
    qDebug() << "directionField:" << directionField;
}
