#ifndef _APP_CALCUL_REMOVEOUTOFUPAREAOP_H_
#define _APP_CALCUL_REMOVEOUTOFUPAREAOP_H_


//SOCLE
#include <ign/feature/sql/FeatureStorePostgis.h>

//EPG
#include <epg/log/EpgLogger.h>
#include <epg/log/ShapeLogger.h>


namespace app{
namespace calcul{

	class RemoveOutOfUpAreaOp {

	public:

	
        /// @brief 
        RemoveOutOfUpAreaOp(
			std::string const& featureName,
			std::string const& countryCode,
            bool verbose
        );

        /// @brief 
        ~RemoveOutOfUpAreaOp();


		/// \brief
		static void Compute(
			std::string const& featureName,
			std::string const& countryCode,
			bool verbose
		);


	private:
		//--
		ign::feature::sql::FeatureStorePostgis*                  _fsFeature;
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
		bool                                                     _verbose;

	private:

		//--
		void _init();

		//--
		void _compute() const;

    };

}
}

#endif