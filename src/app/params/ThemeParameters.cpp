
//APP
#include <app/params/ThemeParameters.h>

//SOCLE
#include <ign/Exception.h>


namespace app{
namespace params{


	///
	///
	///
	ThemeParameters::ThemeParameters()
	{
		_initParameter( DB_CONF_FILE, "DB_CONF_FILE");
		_initParameter( THEME_SCHEMA, "THEME_SCHEMA");
		_initParameter( REF_SCHEMA, "REF_SCHEMA");
		_initParameter( UP_SCHEMA, "UP_SCHEMA");
		_initParameter( WK_SCHEMA, "WK_SCHEMA");
		_initParameter( TABLE_REF_SUFFIX, "TABLE_REF_SUFFIX");
		_initParameter( TABLE_UP_SUFFIX, "TABLE_UP_SUFFIX");
		_initParameter( TABLE_CD_SUFFIX, "TABLE_CD_SUFFIX");
		_initParameter( TABLE_WK_SUFFIX, "TABLE_WK_SUFFIX");
		_initParameter( TABLE_WK_IDS_SUFFIX, "TABLE_WK_IDS_SUFFIX");
		_initParameter( TABLE_UP_AREA_SUFFIX, "TABLE_UP_AREA_SUFFIX");
		_initParameter( ID_REF, "ID_REF");
		_initParameter( ID_UP, "ID_UP");
		_initParameter( GEOM_MATCH, "GEOM_MATCH");
		_initParameter( ATTR_MATCH, "ATTR_MATCH");
		_initParameter( MATCHING_ATTR_MATCH, "MATCHING_ATTR_MATCH");
		_initParameter( BORDER_CODE, "BORDER_CODE");

		_initParameter( CR_DIST_THRESHOLD, "CR_DIST_THRESHOLD");
		_initParameter( CR_DIST_BUFFER, "CR_DIST_BUFFER");
		_initParameter( CR_BUFFER_SIMPLIFICATION_THESHOLD, "CR_BUFFER_SIMPLIFICATION_THESHOLD");

		_initParameter( EX_BBOX_SIDE_THRESHOLD, "EX_BBOX_SIDE_THRESHOLD");
	}

	///
	///
	///
	ThemeParameters::~ThemeParameters()
	{
	}

	///
	///
	///
	std::string ThemeParameters::getClassName()const
	{
		return "app::params::ThemeParameters";
	}


}
}