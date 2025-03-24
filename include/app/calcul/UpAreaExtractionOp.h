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

	/// @brief Classe consacrée à l'extraction des objets en intersection avec les zones mise à jour
	class UpAreaExtractionOp {

	public:

        /// @brief Constructeur
        /// @param featureName Nom de la classe d'objet à traiter
        /// @param countryCode Code pays simple
        /// @param neighborCountryCode Code pays voisin (optionnel: renseigner une chaîne vide si on ne souhaite pas extraire le pays voisin)
        /// @param sourceSchema Schéma source (les valeurs possibles sont : PROD, UP, REF, WORK)
        /// @param targetSchema Schéma cible (les valeurs possibles sont : PROD, UP, REF, WORK)
        /// @param reset Suppression des enregistrements contenus dans la table cible avant extraction
        /// @param verbose Mode verbeux
        UpAreaExtractionOp(
			std::string const& featureName,
            std::string const& countryCode,
			std::string const& neighborCountryCode,
            detail::SCHEMA sourceSchema,
			detail::SCHEMA targetSchema,
            bool reset,
            bool verbose = false
        );

        /// @brief Destructeur
        ~UpAreaExtractionOp();

		/// @brief 
		/// @param featureName Nom de la classe d'objet à traiter
		/// @param countryCode Code pays simple
		/// @param neighborCountryCode Code pays voisin (optionnel: renseigner une chaîne vide si on ne souhaite pas extraire le pays voisin)
		/// @param sourceSchema Schéma source (les valeurs possibles sont : PROD, UP, REF, WORK)
        /// @param targetSchema Schéma cible (les valeurs possibles sont : PROD, UP, REF, WORK)
		/// @param withIds Indique si l'on souhaite enregistrer les indentifiants des objects extraits dans la table des identifiants
		/// (ne pas enregistrer les identifiants dans la table des identifiants permet que les objets extraits soit considérés comme 
		/// des créations par l'outil d'intégration - cf. projet data-tools)
        /// @param reset Suppression des enregistrements contenus dans la table cible avant extraction
        /// @param verbose Mode verbeux
		static void Compute(
			std::string const& featureName,
			std::string const& countryCode,
			std::string const& neighborCountryCode,
            detail::SCHEMA sourceSchema,
			detail::SCHEMA targetSchema,
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
			detail::SCHEMA sourceSchema,
			detail::SCHEMA targetSchema,
			bool reset
		);

		//--
		void _compute(bool withIds);

    };

}
}

#endif