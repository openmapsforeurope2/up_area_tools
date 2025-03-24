//BOOST
#include <boost/program_options.hpp>

//EPG
#include <epg/Context.h>
#include <epg/log/EpgLogger.h>
#include <epg/log/ShapeLogger.h>
#include <epg/tools/TimeTools.h>
#include <epg/params/tools/loadParameters.h>

//OME2
#include <ome2/utils/setTableName.h>

//APP
#include <app/detail/schema.h>
#include <app/params/ThemeParameters.h>
#include <app/calcul/RemoveOutOfUpAreaOp.h>
#include <app/calcul/UpAreaCreationOp.h>
#include <app/calcul/UpAreaExtractionOp.h>


namespace po = boost::program_options;
using namespace app;

int main(int argc, char *argv[])
{
    const std::string creationOp = "create";
    const std::string extractionOp = "extract";
    const std::string themeIb = "ib";
    const std::string themeAu = "au";
    const std::string themeHy = "hy";
    const std::string themeTn = "tn";
    const std::string schemaProd = "prod";
    const std::string schemaRef = "ref";
    const std::string schemaUp = "up";
    const std::string schemaWork = "work";

    // ign::geometry::PrecisionModel::SetDefaultPrecisionModel(ign::geometry::PrecisionModel(ign::geometry::PrecisionModel::FIXED, 1.0e5, 1.0e7) );
    epg::Context* context = epg::ContextS::getInstance();
	std::string     logDirectory = "";
	std::string     epgParametersFile = "";
	std::string     themeParametersFile = "";
	std::string     countryCode = "";
    std::string     neighborCountryCode = "";
	std::string     operation = "";
	std::string     theme = "";
	std::string     feature = "";
	std::string     from = schemaProd;
	std::string     to = schemaWork;
	bool            withIds = true;
	bool            reset = true;
	bool            verbose = true;

    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("c" , po::value< std::string >(&epgParametersFile)     , "conf file" )
        ("T" , po::value< std::string >(&theme)                 , "theme" )
        ("f" , po::value< std::string >(&feature)               , "feature" )
        ("cc" , po::value< std::string >(&countryCode)          , "country code" )
        ("ncc" , po::value< std::string >(&neighborCountryCode) , "neighbor country code" )
        ("op" , po::value< std::string >(&operation)            , "operation" )
        ("from", po::value< std::string >(&from)                , "origin" )
        ("to", po::value< std::string >(&to)                    , "destination" )
        ("no_reset", "do not empty target table before to fill it" )
        ("without_ids", "do not fill de _ids table" )
    ;

    //main log
    std::string     logFileName = "log.txt";
    std::ofstream   logFile( logFileName.c_str() ) ;

    logFile << "[START] " << epg::tools::TimeTools::getTime() << std::endl;

    int returnValue = 0;
    try{

        po::variables_map vm;
        int style = po::command_line_style::default_style & ~po::command_line_style::allow_guessing;
        po::store( po::parse_command_line( argc, argv, desc, style ), vm );
        po::notify( vm );    

        if ( vm.count( "help" ) ) {
            std::cout << desc << std::endl;
            return 1;
        }

        if( theme != themeIb && theme != themeAu && theme != themeHy && theme != themeTn )
            IGN_THROW_EXCEPTION("unknown theme "+theme);
        if( feature == "" )
            IGN_THROW_EXCEPTION("feature name not defined");
        if( operation != creationOp && operation != extractionOp )
            IGN_THROW_EXCEPTION("unknown operation "+operation);
        if( countryCode == "" )
            IGN_THROW_EXCEPTION("missing country code");
        if( from != schemaProd && from != schemaRef && from != schemaUp && from != schemaWork )
            IGN_THROW_EXCEPTION("unknown 'from' schema "+from);
        if( to != schemaProd && to != schemaRef && to != schemaUp && to != schemaWork )
            IGN_THROW_EXCEPTION("unknown 'to' schema "+to);

        reset = !vm.count( "no_reset" );
        withIds = !vm.count( "without_ids" );

        //parametres EPG
		context->loadEpgParameters( epgParametersFile );

        //Initialisation du log de prod
        logDirectory = context->getConfigParameters().getValue( LOG_DIRECTORY ).toString();

        //test si le dossier de log existe sinon le creer
        boost::filesystem::path logDir(logDirectory);
        if (!boost::filesystem::is_directory(logDir))
        {
            if (!boost::filesystem::create_directory(logDir))
            {
                std::string mError = "le dossier " + logDirectory + " ne peut être cree";
                IGN_THROW_EXCEPTION(mError);
            }
        }

        //repertoire de travail
        context->setLogDirectory( logDirectory );

		//theme parameters
		themeParametersFile = context->getConfigParameters().getValue(THEME_PARAMETER_FILE).toString();
		app::params::ThemeParameters* themeParameters = app::params::ThemeParametersS::getInstance();
        epg::params::tools::loadParams(*themeParameters, themeParametersFile, theme);

        //info de connection db
        context->loadEpgParameters( themeParameters->getValue(DB_CONF_FILE).toString() );

        //epg logger
        epg::log::EpgLogger* logger = epg::log::EpgLoggerS::getInstance();
        // logger->setProdOfstream( logDirectory+"/au_merging.log" );
        logger->setDevOfstream( context->getLogDirectory()+"/up_area_tools.log" );

        //shape logger
        epg::log::ShapeLogger* shapeLogger = epg::log::ShapeLoggerS::getInstance();
	    // shapeLogger->setDataDirectory( context->getLogDirectory()+"/shape" );

        //set BDD search path
        context->getDataBaseManager().setSearchPath(themeParameters->getValue(WK_SCHEMA).toString());
        context->getDataBaseManager().addSchemaToSearchPath(themeParameters->getValue(UP_SCHEMA).toString());
        context->getDataBaseManager().addSchemaToSearchPath(themeParameters->getValue(REF_SCHEMA).toString());
        ome2::utils::setTableName<epg::params::EpgParametersS>(TARGET_BOUNDARY_TABLE);
        context->getDataBaseManager().addSchemaToSearchPath(themeParameters->getValue(THEME_SCHEMA).toString());

        detail::SCHEMA sourceSchema = from == schemaProd ? detail::PROD : from == schemaRef ? detail::REF : from == schemaUp ? detail::UP : from == schemaWork ? detail::WORK : detail::PROD;
        detail::SCHEMA targetSchema = to == schemaProd ? detail::PROD : to == schemaRef ? detail::REF : to == schemaUp ? detail::UP : to == schemaWork ? detail::WORK : detail::PROD;

		logger->log(epg::log::INFO, "[START UP AREA TOOLS PROCESS ] " + epg::tools::TimeTools::getTime());
        
        //lancement du traitement
        if(operation == creationOp) {
            app::calcul::UpAreaCreationOp::Compute(feature, countryCode, verbose);
        }
        else if(operation == extractionOp) {
            app::calcul::UpAreaExtractionOp::Compute(
                feature,
                countryCode,
                neighborCountryCode,
                sourceSchema,
                targetSchema,
                withIds,
                reset,
                verbose
            );
        }

		logger->log(epg::log::INFO, "[END UP AREA TOOLS PROCESS ] " + epg::tools::TimeTools::getTime());
	
    }
    catch( ign::Exception &e )
    {
        std::cerr<< e.diagnostic() << std::endl;
        epg::log::EpgLoggerS::getInstance()->log( epg::log::ERROR, std::string(e.diagnostic()));
        logFile << e.diagnostic() << std::endl;
        returnValue = 1;
    }
    catch( std::exception &e )
    {
        std::cerr << e.what() << std::endl;
        epg::log::EpgLoggerS::getInstance()->log( epg::log::ERROR, std::string(e.what()));
        logFile << e.what() << std::endl;
        returnValue = 1;
    }

    logFile << "[END] " << epg::tools::TimeTools::getTime() << std::endl;

    epg::ContextS::kill();
    epg::log::EpgLoggerS::kill();
    epg::log::ShapeLoggerS::kill();
    app::params::ThemeParametersS::kill();
    
    logFile.close();

    return returnValue ;
}