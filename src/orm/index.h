#ifndef __XTEN_ORM_INDEX_H__
#define __XTEN_ORM_INDEX_H__

#include <memory>
#include <string>
#include <vector>
#include <tinyxml2.h>

namespace Xten {
namespace orm {
//数据库表中列的键信息: 主键，唯一建....
class Index {
public:
    enum Type {
        TYPE_NULL = 0, //非法键
        TYPE_PK, //主键
        TYPE_UNIQ, //唯一建
        TYPE_INDEX
    };
    typedef std::shared_ptr<Index> ptr;
    const std::string& getName() const { return m_name;}
    const std::string& getType() const { return m_type;}
    const std::string& getDesc() const { return m_desc;}
    const std::vector<std::string>& getCols() const { return m_cols;}
    Type getDType() const { return m_dtype;}
    //根据xml文件中一个节点初始化 例：<index name="pk" cols="id" type="pk"/>
    bool init(const tinyxml2::XMLElement& node);

    bool isPK() const { return m_type == "pk";}

    static Type ParseType(const std::string& v);
    static std::string TypeToString(Type v);
private:
    std::string m_name;  //名字
    std::string m_type; //类型 string
    std::string m_desc; //描述
    std::vector<std::string> m_cols; //作用的所有表列

    Type m_dtype; //类型 Type
};

}
}

#endif
