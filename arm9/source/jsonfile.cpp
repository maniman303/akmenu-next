#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "jsonfile.h"
#include "jsmn/jsmn.h"

cJsonFile::cJsonFile(const std::string& fileName)
    : _fileName(fileName) {
    load();
}

// ---------------- GET ----------------

const cJsonFile::Node* cJsonFile::find(const std::vector<std::string>& path) const {
    const Node* cur = &_root;

    for (const auto& p : path) {
        auto it = cur->children.find(p);
        if (it == cur->children.end())
            return nullptr;

        cur = &it->second;
    }

    return cur;
}

bool cJsonFile::getBool(const std::vector<std::string>& path, bool def) const {
    const Node* n = find(path);
    if (!n || n->type != Node::BOOL)
        return def;

    return n->value == "true";
}

int cJsonFile::getInt(const std::vector<std::string>& path, int def) const {
    const Node* n = find(path);
    if (!n || n->type != Node::NUMBER)
        return def;

    return atoi(n->value.c_str());
}

std::string cJsonFile::getString(const std::vector<std::string>& path, const std::string& def) const {
    const Node* n = find(path);
    if (!n || n->type != Node::STRING)
        return def;

    return n->value;
}

// ---------------- SET ----------------

cJsonFile::Node* cJsonFile::findOrCreate(const std::vector<std::string>& path) {
    Node* cur = &_root;

    for (const auto& p : path) {
        cur = &cur->children[p];
        cur->type = Node::OBJECT;
    }

    return cur;
}

void cJsonFile::set(const std::vector<std::string>& path, const std::string& value, Node::Type type) {
    Node* n = findOrCreate(path);
    n->type = type;
    n->value = value;
}

void cJsonFile::setBool(const std::vector<std::string>& path, bool value) {
    set(path, value ? "true" : "false", Node::BOOL);
}

void cJsonFile::setInt(const std::vector<std::string>& path, int value) {
    set(path, std::to_string(value), Node::NUMBER);
}

void cJsonFile::setString(const std::vector<std::string>& path, const std::string& value) {
    set(path, value, Node::STRING);
}

// ---------------- SAVE ----------------

std::string cJsonFile::indent(int n) const {
    return std::string(n * 4, ' ');
}

std::string cJsonFile::dump(const Node& node, int level) const {
    if (node.type != Node::OBJECT) {
        if (node.type == Node::STRING)
            return "\"" + node.value + "\"";
        return node.value;
    }

    std::string out = "{\n";
    bool first = true;

    for (const auto& kv : node.children) {
        if (!first)
            out += ",\n";
        first = false;

        out += indent(level + 1);
        out += "\"" + kv.first + "\": ";
        out += dump(kv.second, level + 1);
    }

    out += "\n" + indent(level) + "}";
    return out;
}

bool cJsonFile::save() {
    FILE* f = fopen(_fileName.c_str(), "wb");
    if (!f)
        return false;

    std::string out = dump(_root, 0);

    bool ok = fwrite(out.c_str(), 1, out.size(), f) == out.size();
    fclose(f);

    return ok;
}

// ---------------- LOAD ----------------

void cJsonFile::load() {
    FILE* f = fopen(_fileName.c_str(), "rb");
    if (!f)
        return;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);

    if (size <= 0) {
        fclose(f);
        return;
    }

    rewind(f);

    std::string json(size, '\0');
    fread(&json[0], 1, size, f);
    fclose(f);

    jsmn_parser parser;
    jsmn_init(&parser);

    jsmntok_t tokens[256];

    int r = jsmn_parse(
        &parser,
        json.c_str(),
        json.size(),
        tokens,
        256
    );

    if (r < 1 || tokens[0].type != JSMN_OBJECT) {
        return;
    }

    int index = 0;
    parseObject(json.c_str(), tokens, index, _root);
}

void cJsonFile::parseObject(
    const char* js,
    void* tokens,
    int& i,
    Node& node
) {
    jsmntok_t* t = (jsmntok_t*)tokens;

    node.type = Node::OBJECT;

    // current token MUST be object
    int count = t[i].size;
    i++; // move to first key

    for (int k = 0; k < count; k++) {

        std::string key(
            js + t[i].start,
            t[i].end - t[i].start
        );

        i++; // move to value

        Node& child = node.children[key];

        if (t[i].type == JSMN_OBJECT) {
            parseObject(js, t, i, child);
        }
        else if (t[i].type == JSMN_STRING) {
            child.type = Node::STRING;
            child.value = std::string(js + t[i].start, t[i].end - t[i].start);
            i++;
        }
        else if (t[i].type == JSMN_PRIMITIVE) {
            std::string v(js + t[i].start, t[i].end - t[i].start);

            if (v == "true" || v == "false") {
                child.type = Node::BOOL;
            } else {
                child.type = Node::NUMBER;
            }

            child.value = v;
            i++;
        }
    }
}