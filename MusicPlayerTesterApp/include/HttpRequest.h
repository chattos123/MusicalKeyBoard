#pragma once

#include <string>
#include <sstream>

struct HttpRequest
{
    std::string method;
    std::string path;
    std::string query;
    std::string version;
    bool isValid{ false };

    static HttpRequest Parse(const std::string& rawRequest)
    {
        HttpRequest req;
        std::istringstream stream(rawRequest);
        std::string requestLine;

        if (!std::getline(stream, requestLine) || requestLine.empty())
        {
            return req;
        }

        // Strip trailing \r if present
        if (!requestLine.empty() && requestLine.back() == '\r')
        {
            requestLine.pop_back();
        }

        std::istringstream lineStream(requestLine);
        std::string fullTarget;
        if (!(lineStream >> req.method >> fullTarget >> req.version))
        {
            return req;
        }

        size_t queryPos = fullTarget.find('?');
        if (queryPos != std::string::npos)
        {
            req.path = fullTarget.substr(0, queryPos);
            req.query = fullTarget.substr(queryPos + 1);
        }
        else
        {
            req.path = fullTarget;
        }

        req.isValid = true;
        return req;
    }
};