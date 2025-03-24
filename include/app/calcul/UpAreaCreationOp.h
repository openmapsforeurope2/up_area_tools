#ifndef _APP_CALCUL_UPAREACREATIONOP_H_
#define _APP_CALCUL_UPAREACREATIONOP_H_


//SOCLE
#include <ign/feature/sql/FeatureStorePostgis.h>

//EPG
#include <epg/log/EpgLogger.h>
#include <epg/log/ShapeLogger.h>


namespace app{
namespace calcul{

	/// @brief Classe dédiée à la création des zones de mise à jour
	class UpAreaCreationOp {

	public:

        /// @brief 
        /// @param featureName Nom de la classe d'objet à traiter
        /// @param countryCode Code pays simple
        /// @param verbose Mode verbeux
        UpAreaCreationOp(
			std::string const& featureName,
            std::string const& countryCode,
            bool verbose
        );

        /// @brief 
        ~UpAreaCreationOp();


		/// @brief Lance la création des zones de mise à jour. Ces zones
		/// sont obtenues par la fusion des buffers réalisés autour des 
		/// créés, des objet supprimés aisni que des objets faisant l'objet 
		/// d'une mise à jour géométrique ou d'une mise à jour de certains de leurs
		/// attributs (ceux définis comme impactant le processus de raccordement 
		/// aux frontières)
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
		ign::feature::sql::FeatureStorePostgis*                  _fsProd;
		//--
		ign::feature::sql::FeatureStorePostgis*                  _fsRef;
		//--
		ign::feature::sql::FeatureStorePostgis*                  _fsUp;
		//--
		ign::feature::sql::FeatureStorePostgis*                  _fsCd;
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
		bool                                                     _verbose;

	private:

		//--
		void _init();

		//--
		void _compute() const;

		//--
		void _createAreas(
            ign::feature::FeatureFilter const& filter,
            ign::feature::sql::FeatureStorePostgis* fs,
            std::set<std::string> const& sCd,
            std::set<std::string> & sTreated,
            ign::geometry::GeometryPtr & resultGeomPtr
        ) const;

    };

}
}

#endif