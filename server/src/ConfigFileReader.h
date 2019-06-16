
#ifndef CONFIGFILEREADER_H_
#define CONFIGFILEREADER_H_

#include <map>
#include <string>

using namespace std;

class ConfigFileReader
{
public:
    ConfigFileReader(const char *filename);

    ~ConfigFileReader();

    char *GetConfigName(const char *name);

    int SetConfigValue(const char *name, const char *value);

private:
    void _LoadFile(const char *filename);

    int _WriteFIle(const char *filename = NULL);

    void _ParseLine(char *line);

    char *_TrimSpace(char *name);

    bool m_load_ok;
    std::map<string, string> m_config_map;
    std::string m_config_file;
};


#endif /* CONFIGFILEREADER_H_ */
