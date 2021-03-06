/*
 * @Date: 2020-08-06 17:33:41
 * @LastEditors: OBKoro1
 * @LastEditTime: 2021-02-04 19:13:55
 * @FilePath: /ssxrver/http/HttpResponse.h
 * @Auther: SShouxun
 * @GitHub: https://github.com/RandyLambert
 */
#ifndef SSXRVER_HTTP_HTTPRESPONSE_H
#define SSXRVER_HTTP_HTTPRESPONSE_H
#include <map>
#include <string_view>
#include <memory>
#include "../base/File.h"
namespace ssxrver::net
{
class Buffer;
using std::string;
class HttpResponse //http相应封装的类
{
public:
    enum HttpStatus
    {
        kUnknown,                  //还有很多没实现
        k200Ok = 200,              //成功
        k301MovePermanently = 301, //错误重定向，请求的页面永久转移到另一个地址
        k404BadRequest = 400,      //错误的请求，语法格式有问题，服务器无法处理此请求
        k404NotFound = 404         //请求的网页不存在
    };

    explicit HttpResponse()
        : file_(),
          statusCode_(kUnknown),
          closeConnection_(false)
    {
    }

    void swap(HttpResponse &that);
    void setClose(std::string_view connection) { closeConnection_ = (connection == "close" || connection.empty()); }
    void setStatusCode(HttpStatus code) { statusCode_ = code; }
    void setStatusMessage(std::string_view message) { statusMessage_ = message; }
    void setCloseConnection(bool on) { closeConnection_ = on; }
    [[nodiscard]] bool closeConnection() const { return closeConnection_; }
    //设置文档的媒体类型
    void setContentType(std::string_view contentType) { addHeader("Content-Type", contentType); }
    void addHeader(std::string_view key, std::string_view value) { headers_.insert({key.data(),value.data()}); }
    void setBody(std::string_view body) { body_ = body; }
    void setFile(std::string_view fileName) { file_ = std::make_unique<base::file::SendFileUtil>(fileName); }
    std::unique_ptr<base::file::SendFileUtil> getFile() { return std::move(file_); }
    [[nodiscard]] bool hasFile() const { return file_ != nullptr; }
    void appendToBuffer(Buffer *output) const; //将httpResponse添加到buffer
private:
    std::unique_ptr<base::file::SendFileUtil> file_;
    std::map<string, string> headers_; //header列表
    HttpStatus statusCode_;            //状态响应码
    string statusMessage_;             //状态响应码对应的文本信息
    bool closeConnection_;             //是否关闭连接
    string body_;                      //实体
};

} // namespace ssxrver::net
#endif
