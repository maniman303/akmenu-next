#pragma once

#include <string>
#include <vector>
#include <unordered_map>

class cJsonFile {
public:
    explicit cJsonFile(const std::string& fileName);

    bool getBool(const std::vector<std::string>& path, bool def = false) const;
    int getInt(const std::vector<std::string>& path, int def = 0) const;
    std::string getString(const std::vector<std::string>& path, const std::string& def = "") const;

    void setBool(const std::vector<std::string>& path, bool value);
    void setInt(const std::vector<std::string>& path, int value);
    void setString(const std::vector<std::string>& path, const std::string& value);

    bool save();

private:
    struct Node {
        enum Type { OBJECT, STRING, NUMBER, BOOL } type = OBJECT;

        std::string value;
        std::unordered_map<std::string, Node> children;
    };

    std::string _fileName;
    Node _root;

    const Node* find(const std::vector<std::string>& path) const;
    Node* findOrCreate(const std::vector<std::string>& path);

    void set(const std::vector<std::string>& path, const std::string& value, Node::Type type);

    std::string dump(const Node& node, int indent) const;
    std::string indent(int n) const;

    void load();
    void parseObject(const char* js, void* tokens, int& index, Node& node);
};