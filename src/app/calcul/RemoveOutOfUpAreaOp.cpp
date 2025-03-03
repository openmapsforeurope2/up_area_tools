// APP
#include <app/calcul/RemoveOutOfUpAreaOp.h>
#include <app/params/ThemeParameters.h>
#include <app/calcul/UpAreaExtractionOp.h>

// BOOST
#include <boost/progress.hpp>

// EPG
#include <epg/Context.h>
#include <epg/params/EpgParameters.h>
#include <epg/sql/tools/numFeatures.h>
#include <epg/sql/DataBaseManager.h>
#include <epg/tools/StringTools.h>
#include <epg/tools/TimeTools.h>
#include <epg/tools/FilterTools.h>
#include <epg/tools/geometry/PolygonSplitter.h>


namespace app
{
    namespace calcul
    {
        ///
        ///
        ///
        RemoveOutOfUpAreaOp::RemoveOutOfUpAreaOp(
            std::string const& featureName,
            std::string const& countryCode,
            bool verbose
        ) : 
            _featureName(featureName),
            _countryCode(countryCode),
            _verbose(verbose)
        {
            _init();
        }

        ///
        ///
        ///
        RemoveOutOfUpAreaOp::~RemoveOutOfUpAreaOp()
        {

            // _shapeLogger->closeShape("ps_cutting_ls");
        }

        ///
        ///
        ///
        void RemoveOutOfUpAreaOp::Compute(
            std::string const& featureName,
            std::string const& countryCode,
			bool verbose
		) {
            RemoveOutOfUpAreaOp op(featureName, countryCode, verbose);
            op._compute();
        }

        ///
        ///
        ///
        void RemoveOutOfUpAreaOp::_init()
        {
            //--
            _logger = epg::log::EpgLoggerS::getInstance();
            _logger->log(epg::log::INFO, "[START] initialization: " + epg::tools::TimeTools::getTime());

            //--
            _shapeLogger = epg::log::ShapeLoggerS::getInstance();
            // _shapeLogger->addShape("ps_cutting_ls", epg::log::ShapeLogger::POLYGON);

            //--
            epg::Context *context = epg::ContextS::getInstance();

            // epg parameters
            epg::params::EpgParameters const& epgParams = context->getEpgParameters();
            std::string const idName = epgParams.getValue(ID).toString();
            std::string const geomName = epgParams.getValue(GEOM).toString();
            std::string const countryCodeName = epgParams.getValue(COUNTRY_CODE).toString();
            std::string const borderTableName = epgParams.getValue(TARGET_BOUNDARY_TABLE).toString();
            
            // app parameters
            params::ThemeParameters *themeParameters = params::ThemeParametersS::getInstance();;
            std::string upAreaTableName = _featureName + themeParameters->getValue(TABLE_UP_AREA_SUFFIX).toString();
            std::string tableName = _featureName + themeParameters->getValue(TABLE_WK_SUFFIX).toString();

            //--
            _fsFeature = context->getDataBaseManager().getFeatureStore(tableName, idName, geomName);
            _fsUpArea = context->getDataBaseManager().getFeatureStore(upAreaTableName, idName, geomName);

            //--
            _logger->log(epg::log::INFO, "[END] initialization: " + epg::tools::TimeTools::getTime());
        };

        ///
        ///
        ///
        void RemoveOutOfUpAreaOp::_compute() const {
            //--
            epg::Context *context = epg::ContextS::getInstance();

            // epg parameters
            epg::params::EpgParameters const& epgParams = context->getEpgParameters();
            std::string const idName = epgParams.getValue(ID).toString();
            std::string const geomName = epgParams.getValue(GEOM).toString();
            std::string const countryCodeName = epgParams.getValue(COUNTRY_CODE).toString();

            // app parameters
            params::ThemeParameters *themeParameters = params::ThemeParametersS::getInstance();
            double const bboxThreshold = themeParameters->getValue(EX_BBOX_SIDE_THRESHOLD).toDouble();

            //--
            ign::feature::FeatureFilter areaFilter(countryCodeName+"='"+_countryCode+"'");

            int nAreas = epg::sql::tools::numFeatures(*_fsUpArea, areaFilter);
            boost::progress_display display1(nAreas, std::cout, "[ recording features in update areas % complete ]\n");

            std::set<std::string> sInUpArea;
            
            ign::feature::FeatureIteratorPtr itArea = _fsUpArea->getFeatures(areaFilter);
            while (itArea->hasNext())
            {
                ++display1;

                ign::feature::Feature const& fArea = itArea->next();
                ign::geometry::MultiPolygon const& mp = fArea.getGeometry().asMultiPolygon();

                for (size_t i = 0 ; i < mp.numGeometries() ; ++i) {
                    ign::geometry::Polygon const& poly = mp.polygonN(i);
                    ign::geometry::Envelope bbox = poly.getEnvelope();

                    std::vector<ign::geometry::Polygon> vPoly;
                    if ( bbox.width() <= bboxThreshold && bbox.height() <= bboxThreshold ) {
                        vPoly.push_back(poly);
                    } else {
                        epg::tools::geometry::PolygonSplitter splitter(poly);

                        double currentX = bbox.xmin()+bboxThreshold;
                        while( currentX < bbox.xmax()) {
                            splitter.addCuttingGeometry(ign::geometry::LineString(ign::geometry::Point(currentX, bbox.ymin()), ign::geometry::Point(currentX, bbox.ymax())));
                            currentX += bboxThreshold;
                        }

                        double currentY = bbox.ymin()+bboxThreshold;
                        while( currentY < bbox.ymax()) {
                            splitter.addCuttingGeometry(ign::geometry::LineString(ign::geometry::Point(bbox.xmin(), currentY), ign::geometry::Point(bbox.xmax(), currentY)));
                            currentY += bboxThreshold;
                        }

                        splitter.split(vPoly);
                    }

                    for (size_t j = 0 ; j < vPoly.size() ; ++j) {
                        ign::feature::FeatureFilter filter("ST_INTERSECTS("+geomName+", ST_SetSRID(ST_GeomFromText('"+ vPoly[j].toString() +"'),3035))");
                        ign::feature::FeatureIteratorPtr itFeat = _fsFeature->getFeatures(filter);
                        while (itFeat->hasNext())
                        {
                            ign::feature::Feature feat = itFeat->next();
                            std::string const featId = feat.getId();

                            sInUpArea.insert(featId);
                        }
                    }
                }
            }

            //--
            ign::feature::FeatureFilter featFilter;

            std::set<std::string> sOutOfUpArea;

            ign::feature::FeatureIteratorPtr itFeat = _fsFeature->getFeatures(featFilter);
            while (itFeat->hasNext())
            {
                ign::feature::Feature feat = itFeat->next();
                std::string const featId = feat.getId();

                if( sInUpArea.find(featId) == sInUpArea.end() )
                    sOutOfUpArea.insert(featId);
            }

            boost::progress_display display2(sOutOfUpArea.size(), std::cout, "[ removing features out of update areas % complete ]\n");
            for ( std::set<std::string>::const_iterator sit = sOutOfUpArea.begin() ; sit != sOutOfUpArea.end() ; sit++ ) {
                ++display2;
                _fsFeature->deleteFeature(*sit);
            }
        }
    }
}
