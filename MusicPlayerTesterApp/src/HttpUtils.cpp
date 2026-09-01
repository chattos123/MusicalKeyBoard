#include "HttpUtils.h"
#include <fstream>
#include <sstream>
#include <vector>

namespace HttpUtils
{

    std::string LoadHtmlFile(const std::string &path)
    {
        std::ifstream file(path, std::ios::binary);
        if (!file.is_open())
            return "";
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    std::string UrlDecode(const std::string &str)
    {
        std::string result;
        result.reserve(str.length());

        for (size_t i = 0; i < str.length(); ++i)
        {
            if (str[i] == '%' && i + 2 < str.length())
            {
                int value = 0;
                std::istringstream is(str.substr(i + 1, 2));

                if (is >> std::hex >> value)
                {
                    result += static_cast<char>(value);
                    i += 2;
                }
                else
                {
                    result += str[i];
                }
            }
            else if (str[i] == '+')
            {
                result += ' ';
            }
            else
            {
                result += str[i];
            }
        }
        return result;
    }

    std::string GetQueryParam(const std::string &query, const std::string &key)
    {
        size_t pos = query.find(key + "=");
        if (pos == std::string::npos)
            return "";
        pos += key.length() + 1;
        size_t end = query.find('&', pos);
        if (end == std::string::npos)
            end = query.length();
        return UrlDecode(query.substr(pos, end - pos));
    }

    std::string ResolveHtmlContent()
    {
        const std::vector<std::string> searchPaths = {
            "index.html",
            "../../index.html",
            "../index.html",
            "MusicPlayer/index.html"};

        for (const auto &path : searchPaths)
        {
            std::string content = LoadHtmlFile(path);
            if (!content.empty())
                return content;
        }

        return "<h1>index.html not found! Place it in the project directory.</h1>";
    }

    void SendHttpResponse(SOCKET clientSocket, const std::string &contentType, const std::string &body)
    {
        std::string response = "HTTP/1.1 200 OK\r\n"
                               "Content-Type: " +
                               contentType + "\r\n"
                                             "Access-Control-Allow-Origin: *\r\n"
                                             "Content-Length: " +
                               std::to_string(body.length()) + "\r\n"
                                                               "Connection: close\r\n\r\n" +
                               body;
        send(clientSocket, response.c_str(), static_cast<int>(response.length()), 0);
    }

} // namespace HttpUtils