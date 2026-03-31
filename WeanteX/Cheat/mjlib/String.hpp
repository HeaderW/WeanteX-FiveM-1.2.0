class String {
public:
    static std::string Trim(const std::string& str) {
        std::string::const_iterator start = str.begin();
        while (start != str.end() && std::isspace(*start)) {
            ++start;
        }

        std::string::const_iterator end = str.end();
        while (end != start && std::isspace(*(end - 1))) {
            --end;
        }
        return std::string(start, end);
    }

    static std::string Random(int length) {
        const std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
        std::string randomStr;
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, chars.size() - 1);
        for (int i = 0; i < length; ++i) {
            randomStr += chars[dis(gen)];
        }
        return randomStr;
    }
};