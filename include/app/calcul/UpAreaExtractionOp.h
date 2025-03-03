#ifndef _APP_CALCUL_UPAREAEXTRACTIONOP_H_
#define _APP_CALCUL_UPAREAEXTRACTIONOP_H_

//APP
#include <app/detail/schema.h>

//SOCLE
#include <ign/feature/sql/FeatureStorePostgis.h>

//EPG
#include <epg/log/EpgLogger.h>
#include <epg/log/ShapeLogger.h>


namespace app{
namespace calcul{

	class UpAreaExtractionOp {

	public:

	
        /// @brief 
        /// @param countryCode 
        /// @param verbose 
        UpAreaExtractionOp(
			std::string const& featureName,
            std::string const& countryCode,
			std::string const& neighborCountryCode,
            detail::SCHEMA sourceTheme,
			detail::SCHEMA targetTheme,
            bool reset,
            bool verbose = false
        );

        /// @brief 
        ~UpAreaExtractionOp();


		/// \brief
		static void Compute(
			std::string const& featureName,
			std::string const& countryCode,
			std::string const& neighborCountryCode,
            detail::SCHEMA sourceTheme,
			detail::SCHEMA targetTheme,
            bool withIds,
            bool reset,
			bool verbose = false
		);


	private:
		//--
		ign::feature::sql::FeatureStorePostgis*                  _fsSource;
		//--
		ign::feature::sql::FeatureStorePostgis*                  _fsTarget;
		//--
		ign::feature::sql::FeatureStorePostgis*                  _fsBorder;
		//--
		ign::feature::sql::FeatureStorePostgis*                  _fsUpArea;
		//--
		epg::log::EpgLogger*                                     _logger;
		//--
		epg::log::ShapeLogger*                                   _shapeLogger;
		//--
		std::string                                              _featureName;
		//--
		std::string                                              _countryCode;
		//--
		std::string                                              _neighborCountryCode;
		//--
		std::set<std::string>                                    _sExtracted;
		//--
		bool                                                     _verbose;

	private:

		//--
		void _init(
			detail::SCHEMA sourceTheme,
			detail::SCHEMA targetTheme,
			 bool reset
		);

		//--
		void _compute(bool withIds);

    };

}
}

#endif