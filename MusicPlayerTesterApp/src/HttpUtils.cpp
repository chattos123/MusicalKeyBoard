#include "HttpUtils.h"
#include <fstream>
#include <sstream>
#include <vector>

namespace HttpUtils
{
    namespace
    {
        const std::vector<std::string> AssetSearchPrefixes = {
            "",
            "web/",
            "../",
            "../web/",
            "../../",
            "../../web/",
            "MusicPlayer/",
            "MusicPlayer/web/"
        };

        int HexCharToInt(char c)
        {
            if (c >= '0' && c <= '9') return c - '0';
            if (c >= 'a' && c <= 'f') return c - 'a' + 10;
            if (c >= 'A' && c <= 'F') return c - 'A' + 10;
            return -1;
        }
    }

    std::string LoadFile(const std::string& path)
    {
        std::ifstream file(path, std::ios::binary);
        if (!file.is_open())
        {
            return "";
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    std::string UrlDecode(const std::string& str)
    {
        std::string result;
        result.reserve(str.length());

        for (size_t i = 0; i < str.length(); ++i)
        {
            if (str[i] == '%' && i + 2 < str.length())
            {
                int high = HexCharToInt(str[i + 1]);
                int low = HexCharToInt(str[i + 2]);

                if (high != -1 && low != -1)
                {
                    result += static_cast<char>((high << 4) | low);
                    i += 2;
                    continue;
                }
            }

            if (str[i] == '+')
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

    std::string GetQueryParam(const std::string& query, const std::string& key)
    {
        const std::string delimiter = key + "=";
        size_t pos = query.find(delimiter);
        if (pos == std::string::npos)
        {
            return "";
        }

        pos += delimiter.length();
        size_t end = query.find('&', pos);
        if (end == std::string::npos)
        {
            end = query.length();
        }

        return UrlDecode(query.substr(pos, end - pos));
    }

    std::string GetMimeType(const std::string& path)
    {
        size_t dotPos = path.rfind('.');
        if (dotPos == std::string::npos)
        {
            return Constants::MimePlain;
        }

        std::string extension = path.substr(dotPos);

        if (extension == ".html" || extension == ".htm")
        {
            return Constants::MimeHtml;
        }
        if (extension == ".css")
        {
            return Constants::MimeCss;
        }
        if (extension == ".js")
        {
            return Constants::MimeJs;
        }
        if (extension == ".txt")
        {
            return Constants::MimePlain;
        }

        return Constants::MimeOctetStream;
    }

    std::string ResolveAssetPath(const std::string& relativePath)
    {
        for (size_t i = 0; i < AssetSearchPrefixes.size(); ++i)
        {
            std::string candidate = AssetSearchPrefixes[i] + relativePath;
            std::ifstream fileCheck(candidate, std::ios::binary);
            if (fileCheck.is_open())
            {
                return candidate;
            }
        }
        return "";
    }

    bool TryLoadAsset(const std::string& relativePath, std::string& outContent)
    {
        std::string resolvedPath = ResolveAssetPath(relativePath);
        if (resolvedPath.empty())
        {
            outContent.clear();
            return false;
        }

        outContent = LoadFile(resolvedPath);
        return !outContent.empty();
    }

    std::string MakeHttpResponse(const std::string& contentType, 
                                 const std::string& body, 
                                 const std::string& statusLine)
    {
        return statusLine +
               "Content-Type: " + contentType + "\r\n" +
               Constants::HeaderCors +
               "Content-Length: " + std::to_string(body.length()) + "\r\n" +
               Constants::HeaderClose +
               body;
    }

} // namespace HttpUtils