#pragma once

#include <vector>
#include <string>
#include <functional>
#include <unordered_map>
#include "ClTokenizer.h"

namespace noice
{

class ClParser
{
public:
    using GroupId = unsigned int;
    using ParamId = unsigned int;
    struct ParamData;
    using ParamCallback = std::function<void(const ParamData&, GroupId, void*)>;
    using OnErrorCallback = std::function<void(const std::string&)>;

    struct ParamData
    {
        std::string description;
        std::string shortName;
        std::string longName;
        CliParamType type;
        uint64_t offset;
        std::vector<std::string> enumNames;
        ParamCallback onSet;
    };

    struct Group
    {
        GroupId groupId = -1;
        std::string name;
        std::vector<ParamData> params;
    };

    ClParser() {}

    GroupId createGroup(const char* name);
    bool addParam(GroupId gid, const ParamData& param);
    void bind(GroupId gid, void* object);
    void setOnErrorCallback(OnErrorCallback cb) { m_onError = cb; }
    bool parse(int argc, char* argv[]);
    void printTokens(int argc, char* argv[]);

    const char* appPath() const { return m_appPath.c_str(); }

private:
    struct Schema
    {
        std::vector<Group> groups;
    };

    struct ParamLoc
    {
        GroupId gid;
        int paramIndex;
    };

    void reportError(const char* msg) const;
    void reportErrorStr(const std::string& msg) const;
    bool parseParamName(const ClTokenizer::Name& nm, ParamLoc& outLoc);
    bool parseParamValue(const ParamLoc& loc, const ClTokenizer::Imm& value);

    Schema m_schema;
    OnErrorCallback m_onError = nullptr;
    using ParamMap = std::unordered_map<std::string, ParamLoc>;
    ParamMap m_usedParamNames;
    ParamMap m_usedParamShortNames;
    std::string m_appPath;
    std::vector<void*> m_groupBinds;
};

}
