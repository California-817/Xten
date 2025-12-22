#ifndef __XTEN_ORM_TABLE_H__
#define __XTEN_ORM_TABLE_H__

#include "column.h"
#include "index.h"
#include <fstream>

namespace Xten
{
    namespace orm
    {
        // 数据库表
        class Table
        {
        public:
            typedef std::shared_ptr<Table> ptr;
            // 获取表名
            const std::string &getName() const { return m_name; }
            // 获取表命名空间
            const std::string &getNamespace() const { return m_namespace; }
            // 获取表描述
            const std::string &getDesc() const { return m_desc; }
            // 获取列属性
            const std::vector<Column::ptr> &getCols() const { return m_cols; }
            // 获取键属性
            const std::vector<Index::ptr> &getIdxs() const { return m_idxs; }
            // 从xml文件根节点初始化
            bool init(const tinyxml2::XMLElement &node);
            // 产生源文件和头文件
            void gen(const std::string &path);
            // 获取表名
            std::string getFilename() const;

        private:
            // 生成头文件
            void gen_inc(const std::string &path);
            // 生成源文件
            void gen_src(const std::string &path);
            // json序列化
            std::string genToStringInc();
            std::string genToStringSrc(const std::string &class_name);

            std::string genToInsertSQL(const std::string &class_name);
            std::string genToUpdateSQL(const std::string &class_name);
            std::string genToDeleteSQL(const std::string &class_name);
            // 获取主键列
            std::vector<Column::ptr> getPKs() const;
            Column::ptr getCol(const std::string &name) const;

            std::string genWhere() const;

            void gen_dao_inc(std::ofstream &ofs);
            void gen_dao_src(std::ofstream &ofs);
            // 数据库驱动类型
            enum DBType
            {
                TYPE_SQLITE3 = 1,
                TYPE_MYSQL = 2
            };

        private:
            std::string m_name; //表名
            std::string m_namespace; //命名空间
            std::string m_desc; //表描述
            std::string m_subfix = "_info"; //后缀
            DBType m_type = TYPE_SQLITE3; //数据库驱动类型
            std::string m_dbclass = "Xten::IDB";
            std::string m_queryclass = "Xten::IDB";
            std::string m_updateclass = "Xten::IDB";
            std::vector<Column::ptr> m_cols; //所有列
            std::vector<Index::ptr> m_idxs; //所有键值信息
        };

    }
}

#endif
