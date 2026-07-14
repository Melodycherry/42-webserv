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

#ifndef UTILSPARSING_HPP
#define UTILSPARSING_HPP

#include <string>
#include <vector>

long		stringToLong(const std::string &str);
long		hexToLong(const std::string& hex);
std::string longToString(long n);
int			stringToInt(const std::string &str);
size_t		parseSize(const std::string &str);
std::string	toLower(const std::string& str);
bool        isHex(char c);
int         hexValue(char c);
char        hexToChar(char a, char b);
std::string trim(const std::string& s);
void        safeClose(int *fd);
void	    *freePtr(void **ptr);
void	    freeTab(char ***tab);

std::vector<std::string> splitLines(std::string str);

#endif