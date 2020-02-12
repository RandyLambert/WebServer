// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/sserver/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is a public header file, it must only include public header files.

#ifndef SSERVER_HTTP_HTTPRESPONSE_H
#define SSERVER_HTTP_HTTPRESPONSE_H

/* #include "../../sserver_base/Types.h" */

#include <map>

namespace sserver
{
namespace net
{

class Buffer;
using std::string;
class HttpResponse
{ //http响应类的封装
public:
    enum HttpStatusCode
    {
        kUnknown,                   //还有很多没实现
        k200Ok = 200,               //成功
        k301MovedPermanently = 301, //错误重定向，请求的页面永久转移到另一个地址
        k400BadRequest = 400,       //错误的请求，语法格式有问题，服务器无法处理此请求
        k404NotFound = 404,         //请求的网页不存在
    };

    explicit HttpResponse(bool close)
        : statusCode_(kUnknown),
          closeConnection_(close)
    {
    }

    void setStatusCode(HttpStatusCode code)
    {
        statusCode_ = code;
    }

    void setStatusMessage(const string &message)
    {
        statusMessage_ = message;
    }

    void setCloseConnection(bool on)
    {
        closeConnection_ = on;
    }

    bool closeConnection() const
    {
        return closeConnection_;
    }
    //设置文档媒体类型
    void setContentType(const string &contentType)
    {
        addHeader("Content-Type", contentType);
    }

    // FIXME: replace string with StringPiece
    void addHeader(const string &key, const string &value)
    {
        headers_[key] = value;
    }

    void setBody(const string &body)
    {
        body_ = body;
    }

    void appendToBuffer(Buffer *output) const;//将httpresponse添加到buffer

private:
    std::map<string, string> headers_; //header列表
    HttpStatusCode statusCode_;        //状态相应吗
    // FIXME: add http version
    string statusMessage_; //状态响应码对应的文本信息
    bool closeConnection_; //是否关闭连接
    string body_;          //实体
};

} // namespace net
} // namespace sserver

#endif // SSERVER_HTTP_HTTPRESPONSE_H
