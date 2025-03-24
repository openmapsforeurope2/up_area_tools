#ifndef _APP_CALCUL_REMOVEOUTOFUPAREAOP_H_
#define _APP_CALCUL_REMOVEOUTOFUPAREAOP_H_


//SOCLE
#include <ign/feature/sql/FeatureStorePostgis.h>

//EPG
#include <epg/log/EpgLogger.h>
#include <epg/log/ShapeLogger.h>


namespace app{
namespace calcul{

	/// @brief Classe detinée à la suppression des objets qui ne sont pas
	/// en intersection avec les zones de mise à jour.
	class RemoveOutOfUpAreaOp {

	public:

	
        /// @brief Constructeur
        /// @param featureName Nom de la classe d'objet à traiter
        /// @param countryCode Code pays simple
        /// @param verbose Mode verbeux
        RemoveOutOfUpAreaOp(
			std::string const& featureName,
			std::string const& countryCode,
            bool verbose
        );

        /// @brief Destructeur
        ~RemoveOutOfUpAreaOp();


		/// @brief Lance la suppression des objets qui ne sont pas en
		/// intersection avec les zones de mise à jour
		/// @param featureName Nom de la classe d'objet à traiter
        /// @param countryCode Code pays simple
        /// @param verbose Mode verbeux
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