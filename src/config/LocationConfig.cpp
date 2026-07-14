/*****************************************************************************/
/*                                                                           */
/*                                                                           */
/*                        LES CODEUSES DU DIMANCHE                           */
/*                                FONT UN                                    */
/*                                                                           */
/*                   _ _ _ ____ ___  ____ ____ ____ _  _                     */
/*                   | | | |___ |__] [__  |___ |__/ |  |                     */
/*                   |_|_| |___ |__] ___] |___ |  \  \/                      */
/*                                                                           */
/*****************************************************************************/

#include "LocationConfig.hpp"

/* *************************************************** */
/*  Constructors, destructor, and assignment operator  */
/* *************************************************** */ 
LocationConfig::LocationConfig(){
	path = "/";
	root = "";
	autoindex = false;
	index.push_back("index.html");
	returnCode = 0;
	returnUrl = "";
	uploadStore = "";
	maxBodySize = 100000;
	cgiEnabled = false;

	methods.push_back("GET");

	hasGet = false;
	hasPost = false;
	hasDelete = false;

}
LocationConfig::LocationConfig(const LocationConfig &other) {
	path = other.path;
	root = other.root;
	methods = other.methods;
	autoindex = other.autoindex;
	index = other.index;
	returnCode = other.returnCode;
	returnUrl = other.returnUrl;
	uploadStore = other.uploadStore;
	cgiInfo = other.cgiInfo;
	maxBodySize = other.maxBodySize;
	cgiEnabled = other.cgiEnabled;
	hasGet = other.hasGet;
	hasDelete = other.hasDelete;
	hasPost = other.hasPost;
}
LocationConfig& LocationConfig::operator=(const LocationConfig &other) {
	if (this != &other) {
		path = other.path;
		root = other.root;
		methods = other.methods;
		autoindex = other.autoindex;
		index = other.index;
		returnCode = other.returnCode;
		returnUrl = other.returnUrl;
		uploadStore = other.uploadStore;
		cgiInfo = other.cgiInfo;
		maxBodySize = other.maxBodySize;
		cgiEnabled = other.cgiEnabled;
		hasGet = other.hasGet;
		hasDelete = other.hasDelete;
		hasPost = other.hasPost;
	}
	return *this;
}
LocationConfig::~LocationConfig() {}

