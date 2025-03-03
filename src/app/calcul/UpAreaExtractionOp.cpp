// APP
#include <app/calcul/UpAreaExtractionOp.h>
#include <app/params/ThemeParameters.h>

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
        UpAreaExtractionOp::UpAreaExtractionOp(
            std::string const& featureName,
            std::string const& countryCode,
			std::string const& neighborCountryCode,
            detail::SCHEMA sourceTheme,
			detail::SCHEMA targetTheme,
            bool reset,
            bool verbose
        ) : 
            _featureName(featureName),
            _countryCode(countryCode),
            _neighborCountryCode(neighborCountryCode),
            _verbose(verbose)
        {
            _init(sourceTheme, targetTheme, reset);
        }

        ///
        ///
        ///
        UpAreaExtractionOp::~UpAreaExtractionOp()
        {

            // _shapeLogger->closeShape("ps_cutting_ls");
        }

        ///
        ///
        ///
        void UpAreaExtractionOp::Compute(
            std::string const& featureName,
			std::string const& countryCode,
			std::string const& neighborCountryCode,
            detail::SCHEMA sourceTheme,
			detail::SCHEMA targetTheme,
            bool withIds,
            bool reset,
			bool verbose
		) {
            UpAreaExtractionOp op(
                featureName,
                countryCode,
                neighborCountryCode,
                sourceTheme,
                targetTheme,
                reset,
                verbose
            );
            op._compute(withIds);
        }

        ///
        ///
        ///
        void UpAreaExtractionOp::_init(
            detail::SCHEMA sourceTheme,
			detail::SCHEMA targetTheme,
            bool reset
        ) {
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
            params::ThemeParameters *themeParameters = params::ThemeParametersS::getInstance();
            std::string sourceTableName = _featureName + detail::getSuffix(sourceTheme);
            std::string targetTableName = _featureName + detail::getSuffix(targetTheme);
            std::string upAreaTableName = _featureName + themeParameters->getValue(TABLE_UP_AREA_SUFFIX).toString();
            std::string targetIdsTableName = _featureName + themeParameters->getValue(TABLE_WK_IDS_SUFFIX).toString();

            if (reset) {
                std::ostringstream ss;
                ss << "DELETE FROM " << targetTableName << " ;";
                context->getDataBaseManager().getConnection()->update(ss.str());
            }

            if (reset) {
                std::ostringstream ss;
                ss << "DELETE FROM " << targetIdsTableName << " ;";
                context->getDataBaseManager().getConnection()->update(ss.str());
            }

            //--
            _fsSource = context->getDataBaseManager().getFeatureStore(sourceTableName, idName, geomName);
            _fsTarget = context->getDataBaseManager().getFeatureStore(targetTableName, idName, geomName);
            _fsUpArea = context->getDataBaseManager().getFeatureStore(upAreaTableName, idName, geomName);
            _fsTargetIds = context->getDataBaseManager().getFeatureStore(targetIdsTableName, idName, geomName);

            //--
            if (!reset) {
                std::ostringstream ss;
                ss << "SELECT " << idName << " FROM " << targetIdsTableName << " ;";
                ign::sql::SqlResultSetPtr result = context->getDataBaseManager().getConnection()->query( ss.str() );

                for ( size_t i = 0; i < result->size(); i++ ){
                    ign::sql::SqlValue value = result->getFieldValue(i,0);
                    _sExtracted.insert(value.toString());
                }
            }

            //--
            _logger->log(epg::log::INFO, "[END] initialization: " + epg::tools::TimeTools::getTime());
        };

        ///
        ///
        ///
        void UpAreaExtractionOp::_compute(
            bool withIds
        ) {
            //--
            epg::Context *context = epg::ContextS::getInstance();

            // epg parameters
            epg::params::EpgParameters const& epgParams = context->getEpgParameters();
            std::string const idName = epgParams.getValue(ID).toString();
            std::string const geomName = epgParams.getValue(GEOM).toString();
            std::string const countryCodeName = epgParams.getValue(COUNTRY_CODE).toString();

            // app parameters
            params::ThemeParameters *themeParameters = params::ThemeParametersS::getInstance();
            std::string const borderCodeName = themeParameters->getValue(BORDER_CODE).toString();
            double const bboxThreshold = themeParameters->getValue(EX_BBOX_SIDE_THRESHOLD).toDouble();
            std::string targetTableName = _featureName + themeParameters->getValue(TABLE_WK_SUFFIX).toString();
            std::string targetIdsTableName = _featureName + themeParameters->getValue(TABLE_WK_IDS_SUFFIX).toString();

            //--
            ign::feature::FeatureFilter areaFilter(countryCodeName+"='"+_countryCode+"'");
            if (_neighborCountryCode != "")
                epg::tools::FilterTools::addAndConditions(areaFilter, borderCodeName+"='"+_countryCode+"' OR "+borderCodeName+" LIKE '%"+_neighborCountryCode+"%'");

            int nAreas = epg::sql::tools::numFeatures(*_fsUpArea, areaFilter);
            boost::progress_display display(nAreas, std::cout, "[ extracting % complete ]\n");

            //--
            std::ostringstream ssCondition;
            if( _neighborCountryCode == "" ) 
                ssCondition  << countryCodeName << " NOT LIKE '%" << _countryCode << "%'";
            else
                ssCondition  << countryCodeName << "!='" << _countryCode << "'"
                    << " AND " << countryCodeName << "!='" << _neighborCountryCode << "'"
                    << " AND " << countryCodeName << "!='" << std::min(_countryCode, _neighborCountryCode) << "#" << std::max(_countryCode, _neighborCountryCode) << "';";
            std::string const countryCodeContition = ssCondition.str();

            //--
            ign::feature::FeatureIteratorPtr itArea = _fsUpArea->getFeatures(areaFilter);
            while (itArea->hasNext())
            {
                ++display;

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
                        epg::tools::FilterTools::addAndConditions(filter, countryCodeContition);
                        ign::feature::FeatureIteratorPtr itFeat = _fsSource->getFeatures(filter);
                        while (itFeat->hasNext())
                        {
                            ign::feature::Feature feat = itFeat->next();
                            std::string const featId = feat.getId();

                            if (_sExtracted.find(featId) != _sExtracted.end()) continue;
                            _sExtracted.insert(featId);

                            _fsTarget->createFeature(feat, featId);
                            if( withIds ) {
                                std::ostringstream ss;
                                ss << "INSERT INTO " << targetIdsTableName << " (" << idName << ") VALUES (" << featId << ");";
                                context->getDataBaseManager().getConnection()->update(ss.str());
                            }
                        }
                    }
                }
            }
        }
    }
}
