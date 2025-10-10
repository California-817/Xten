#!/bin/sh

command_error_exit() {
    $*
    if [ $? -ne 0 ]
    then
        exit 1
    fi
}

if [ $# -lt 2 ]
then
    echo "use [$0 project_name namespace]"
    exit 0
fi

project_name=$1
namespace=$2

command_error_exit mkdir $project_name
command_error_exit cd $project_name
command_error_exit git clone git@github.com:California-817/Xten.git
command_error_exit cd Xten
command_error_exit git checkout main
command_error_exit cd ..
command_error_exit cp Xten/Makefile .
command_error_exit cp -rf Xten/template/* .
command_error_exit mv mymodule ${namespace}
command_error_exit sed -i "s/project_name/${project_name}/g" CMakeLists.txt
command_error_exit sed -i "s/mymodule/${namespace}/g" CMakeLists.txt
command_error_exit sed -i "s/project_name/${project_name}/g" move.sh
command_error_exit cd ${namespace}
command_error_exit sed -i "s/name_space/${namespace}/g" `ls .`
command_error_exit sed -i "s/project_name/${project_name}/g" `ls .`
command_error_exit cd ../bin/conf
command_error_exit sed -i "s/name_space/${namespace}/g" `ls .`
command_error_exit sed -i "s/project_name/${project_name}/g" `ls .`

echo "create module ${project_name} -- ${namespace} ok" 
