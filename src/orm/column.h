#ifndef __XTEN_ORM_COLUMN_H__
#define __XTEN_ORM_COLUMN_H__

#include <memory>
#include <string>
#include <tinyxml2.h>

namespace Xten {
namespace orm {

class Table;
//数据库表中的某一列属性
class Column {
friend class Table;
public:
    typedef std::shared_ptr<Column> ptr;
    enum Type {
        TYPE_NULL = 0,
        TYPE_INT8,
        TYPE_UINT8,
        TYPE_INT16,
        TYPE_UINT16,
        TYPE_INT32,
        TYPE_UINT32,
        TYPE_FLOAT,
        TYPE_DOUBLE,
        TYPE_INT64,
        TYPE_UINT64,
        TYPE_STRING,
        TYPE_TEXT,
        TYPE_BLOB,
        TYPE_TIMESTAMP
    };

    const std::string& getName() const { return m_name;}
    const std::string& getType() const { return m_type;}
    const std::string& getDesc() const { return m_desc;}
    const std::string& getDefault() const { return m_default;}

    std::string getDefaultValueString();
    std::string getSQLite3Default();

    bool isAutoIncrement() const { return m_autoIncrement;}
    Type getDType() const { return m_dtype;}
    //根据xml的一个节点进行初始化 例：<column name="id" type="int64" auto_increment="true" desc="用户id"/>
    bool init(const tinyxml2::XMLElement& node);
    //数据信息类生成方法
    //成员定义
    std::string getMemberDefine() const;
    //获取方法定义
    std::string getGetFunDefine() const;
    //设置方法定义
    std::string getSetFunDefine() const;
    //获取方法实现
    std::string getSetFunImpl(const std::string& class_name, int idx) const;
    //获取index
    int getIndex() const { return m_index;}

    static Type ParseType(const std::string& v);
    static std::string TypeToString(Type type);

    std::string getDTypeString() { return TypeToString(m_dtype);}
    //返回具体数据库对应的类型
    std::string getSQLite3TypeString();
    std::string getMySQLTypeString();

    std::string getBindString();
    std::string getGetString();
    const std::string& getUpdate() const { return m_update;}
private:
    std::string m_name; //列名
    std::string m_type; //数据类型 string
    std::string m_default; //默认值
    std::string m_update; //
    std::string m_desc; //描述
    int m_index; //下标

    bool m_autoIncrement; //是否自增
    Type m_dtype; //数据类型 Type
    int m_length; //string类型长度
};

}
}

#endif
