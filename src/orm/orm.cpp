#include "table.h"
#include "../util.h"
#include "../log.h"

static Xten::Logger::ptr g_logger = XTEN_LOG_NAME("orm");

/* example: [userinfo.xml]
<table name="user" namespace="blog.data" desc="用户表">
    <columns>
        <column name="id" type="int64" auto_increment="true" desc="用户id"/>
        <column name="account" type="string" length="30" desc="账号"/>
        <column name="email" type="string" length="50" desc="邮件地址"/>
        <column name="passwd" type="string" length="50" desc="密码"/>
        <column name="name" type="string" length="30" desc="名称"/>
        <column name="code" type="string" length="20" desc="验证码"/>
        <column name="role" type="int32" desc="角色"/>
        <column name="state" type="int32" desc="状态1未激活2激活"/>
        <column name="login_time" type="datetime" desc="登录时间"/>
        <column name="is_deleted" type="int32" desc="是否删除"/>
        <column name="create_time" type="datetime" desc="创建时间"/>
        <column name="update_time" type="datetime" update="current_timestamp" desc="修改时间"/>
    </columns>
    <indexs>
        <index name="pk" cols="id" type="pk"/>
        <index name="account" cols="account" type="uniq"/>
        <index name="email" cols="email" type="uniq"/>
        <index name="name" cols="name" type="uniq"/>
    </indexs>
</table>
*/

// 生成camkelists文件
/*
cmake_minimum_required(VERSION 3.0)
project(orm_data)

set(LIB_SRC
    blog/data/article_info.cc
    blog/data/article_category_rel_info.cc
    blog/data/article_label_rel_info.cc
    blog/data/category_info.cc
    blog/data/channel_info.cc
    blog/data/comment_info.cc
    blog/data/label_info.cc
    blog/data/user_info.cc
)
add_library(orm_data ${LIB_SRC})
force_redefine_file_macro_for_sources(orm_data)
*/
void gen_cmake(const std::string &path, const std::map<std::string, Xten::orm::Table::ptr> &tbs)
{
    std::ofstream ofs(path + "/CMakeLists.txt");
    ofs << "cmake_minimum_required(VERSION 3.0)" << std::endl;
    ofs << "project(orm_data)" << std::endl;
    ofs << std::endl;
    ofs << "set(LIB_SRC" << std::endl;
    for (auto &i : tbs)
    {
        ofs << "    " << Xten::replace(i.second->getNamespace(), ".", "/")
            << "/" << Xten::ToLower(i.second->getFilename()) << ".cpp" << std::endl;
    }
    ofs << ")" << std::endl;
    ofs << "add_library(orm_data ${LIB_SRC})" << std::endl;
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        std::cout << "Usage: " << argv[0] << " [xorm_config_path] [xorm_output_path]" << std::endl;
        std::cout << "example: " << argv[0] << " ./xorm_config ./xorm_out" << std::endl;
        exit(1);
    }
    // default path
    std::string input_path = "./xorm_config";
    std::string out_path = "./xorm_out";

    input_path = argv[1];
    out_path = argv[2];
    std::vector<std::string> files;
    Xten::FileUtil::ListAllFile(files, input_path, ".xml");
    std::vector<Xten::orm::Table::ptr> tbs;
    bool has_error = false;
    for (auto &i : files)
    {
        XTEN_LOG_INFO(g_logger) << "init xml=" << i << " begin";
        tinyxml2::XMLDocument doc;
        if (doc.LoadFile(i.c_str()))
        { // 加载xml文件
            XTEN_LOG_ERROR(g_logger) << "error: " << doc.ErrorStr();
            has_error = true;
        }
        else
        {
            Xten::orm::Table::ptr table(new Xten::orm::Table);
            if (!table->init(*doc.RootElement()))
            { // 解析xml文件生成table
                XTEN_LOG_ERROR(g_logger) << "table init error";
                has_error = true;
            }
            else
            {
                tbs.push_back(table);
            }
        }
        XTEN_LOG_INFO(g_logger) << "init xml=" << i << " end";
    }
    if (has_error)
    {
        return 0;
    }

    std::map<std::string, Xten::orm::Table::ptr> orm_tbs;
    for (auto &i : tbs)
    {
        i->gen(out_path); // 生成源文件和头文件
        orm_tbs[i->getName()] = i;
    }
    // 生成cmakelists文件
    gen_cmake(out_path, orm_tbs);
    return 0;
}
