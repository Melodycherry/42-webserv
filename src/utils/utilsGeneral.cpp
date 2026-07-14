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

#include "utilsGeneral.hpp"
#include "WebServer.hpp"
#include "Exceptions.hpp"
#include <climits>

/* ************************************************** */
/* Conversion					                      */
/* ************************************************** */

long	stringToLong(const std::string &str){
	long result = 0;
	for (size_t i = 0; i < str.size(); i++)
	{
		if (!isdigit(static_cast<unsigned char>(str[i])))
			throw ParseException(CONF, "Invalid number: " + str);
		result = result * 10 + (str[i] - '0');
	}
	return result;
}

std::string longToString(long n) {
    std::stringstream ss;
    ss << n;
    return ss.str();
}

long	hexToLong(const std::string& hex)
{
    long result = 0;
    
    for (size_t i = 0; i < hex.size(); i++)
    {
        char c = hex[i];
        result *= 16;
        
        if (c >= '0' && c <= '9')
            result += c - '0';
        else if (c >= 'a' && c <= 'f')
            result += c - 'a' + 10;
        else if (c >= 'A' && c <= 'F')
            result += c - 'A' + 10;
        else
            return -1;
    }
    return result;
}

char hexToChar(char a, char b) {
	char i = hexValue(a) * 16 + hexValue(b);
	LOG("le char du demon est " << i);
    return (hexValue(a) * 16 + hexValue(b));
}

int	stringToInt(const std::string &str){
	int result = 0;
	for (size_t i = 0; i < str.size(); i++)
	{
		if (!isdigit(static_cast<unsigned char>(str[i])))
			throw ParseException(CONF, "Invalid number: " + str);
		if (result > (INT_MAX - (str[i] - '0')) / 10)
			throw ParseException(CONF, "Number too large: " + str);
		result = result * 10 + (str[i] - '0');
	}
	return result;
}

/* ************************************************** */
/* Autre						                      */
/* ************************************************** */

std::string toLower(const std::string& str){
    std::string res;
    for (size_t i = 0; i < str.size(); ++i) {
        res += std::tolower(str[i]);
    }
	return res;
}

bool isHex(char c) {
    return (
        (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f')
    );
}

int hexValue(char c){
	if (c >= '0' && c <= '9')
		return (c - '0');
	
	else if (c >= 'A' && c <= 'F')
		return (c - 'A' + 10);

	else if (c >= 'a' && c <= 'f')
		return (c - 'a' + 10);

	return -1 ;
} 

size_t	parseSize(const std::string &str){
	if (str.empty()) return 0;
	char unit = str[str.size() - 1];
	std::string valueStr = str;
	size_t multiplier = 1;
	if (!isdigit(unit)) {
		valueStr = str.substr(0, str.size() - 1);
		if (unit == 'K' || unit == 'k') multiplier = 1024;
		else if (unit == 'M' || unit == 'm') multiplier = 1024 * 1024;
		else if (unit == 'G' || unit == 'g') multiplier = 1024 * 1024 * 1024;
		else throw ParseException(CONF, "Invalid size unit: " + str);
	}
	long val = stringToLong(valueStr);
	if (val < 0) 
		throw ParseException(CONF, "Size cannot be negative: " + str);
	return static_cast<size_t>(val * multiplier);
}

std::string trim(const std::string& s)
{
    size_t start = 0;
    while (start < s.size() && std::isspace(s[start]))
        start++;

    size_t end = s.size();
    while (end > start && std::isspace(s[end - 1]))
        end--;

    return s.substr(start, end - start);
}

/* ************************************************** */
/* Close and free				                      */
/* ************************************************** */

void	safeClose(int *fd){
	if (!*fd || !fd)
		return;
	if (*fd != -1) {
		close(*fd);
		*fd = -1;
	}
}

void	*freePtr(void **ptr)
{
	if (*ptr && ptr) {
		free(*ptr);
		*ptr = NULL;
	}
	return (NULL);
}

void freeTab(char*** envp)
{
    for (int i = 0; (*envp)[i] != NULL; i++)
        free((*envp)[i]);

    delete[] *envp;
    *envp = NULL;
}

std::vector<std::string> splitLines(std::string str){
	std::vector<std::string> ret;
	
	size_t posStart = 0;
    size_t pos;
	
	while((pos = str.find('\n', posStart)) != std::string::npos){
	
		std::string line = str.substr(posStart, pos - posStart);
		if (!line.empty() && line[line.size() - 1] == '\r')
            line.erase(line.size() - 1);

        ret.push_back(line);
        posStart = pos + 1;
    }

    if (posStart < str.size()) {
        std::string line = str.substr(posStart);
        if (!line.empty() && line[line.size() - 1] == '\r')
            line.erase(line.size() - 1);

        ret.push_back(line);
    }

	return ret;
}
