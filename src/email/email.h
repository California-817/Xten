#ifndef __XTEN_EMAIL_EMAIL_H__
#define __XTEN_EMAIL_EMAIL_H__

#include <memory>
#include <string>
#include <vector>
#include <map>

namespace Xten {

//附件实体
class EMailEntity {
public:
    typedef std::shared_ptr<EMailEntity> ptr;
    //从文件中读取附件正文
    static EMailEntity::ptr CreateAttach(const std::string& filename);

    void addHeader(const std::string& key, const std::string& val);
    std::string getHeader(const std::string& key, const std::string& def = "");

    const std::string& getContent() const { return m_content;}
    void setContent(const std::string& v) { m_content = v;}

    std::string toString() const;
private:
    std::map<std::string, std::string> m_headers; //附件头部字段
    std::string m_content; //附件正文
};


class EMail {
public:
    typedef std::shared_ptr<EMail> ptr;
    //静态工厂方法创建邮件对象
    static EMail::ptr Create(const std::string& from_address, const std::string& from_passwd
                            ,const std::string& title, const std::string& body
                            ,const std::vector<std::string>& to_address
                            ,const std::vector<std::string>& cc_address = {}
                            ,const std::vector<std::string>& bcc_address = {});

    const std::string& getFromEMailAddress() const { return m_fromEMailAddress;}
    const std::string& getFromEMailPasswd() const { return m_fromEMailPasswd;}
    const std::string& getTitle() const { return m_title;}
    const std::string& getBody() const { return m_body;}

    void setFromEMailAddress(const std::string& v) { m_fromEMailAddress = v;}
    void setFromEMailPasswd(const std::string& v) { m_fromEMailPasswd = v;}
    void setTitle(const std::string& v) { m_title = v;}
    void setBody(const std::string& v) { m_body = v;}

    const std::vector<std::string>& getToEMailAddress() const { return m_toEMailAddress;}
    const std::vector<std::string>& getCcEMailAddress() const { return m_ccEMailAddress;}
    const std::vector<std::string>& getBccEMailAddress() const { return m_bccEMailAddress;}

    void setToEMailAddress(const std::vector<std::string>& v) { m_toEMailAddress = v;}
    void setCcEMailAddress(const std::vector<std::string>& v) { m_ccEMailAddress = v;}
    void setBccEMailAddress(const std::vector<std::string>& v) { m_bccEMailAddress = v;}

    void addEntity(EMailEntity::ptr entity);
    const std::vector<EMailEntity::ptr>& getEntitys() const { return m_entitys;}
private:
    std::string m_fromEMailAddress;  //发送方ip
    std::string m_fromEMailPasswd; //发送方密码
    std::string m_title; //标题
    std::string m_body; //正文
    std::vector<std::string> m_toEMailAddress; //接收方地址
    std::vector<std::string> m_ccEMailAddress;//
    std::vector<std::string> m_bccEMailAddress; //
    std::vector<EMailEntity::ptr> m_entitys; //附件
};

}

#endif
