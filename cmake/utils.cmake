function(ragelmaker src_rl outputlist outputdir)
     #Create a custom build step that will call ragel on the provided src_rl file.
     #The output .cpp file will be appended to the variable name passed in outputlist.
    get_filename_component(src_file ${src_rl} NAME_WE) #获取文件名（无后缀）
    set(rl_out ${outputdir}/${src_file}.rl.cc) #生成的目标文件的name
     #adding to the list inside a function takes special care, we cannot use list(APPEND...)
     #because the results are local scope only
    set(${outputlist} ${${outputlist}} ${rl_out} PARENT_SCOPE) #将目标文件追加到源文件列表
     #Warning: The " -S -M -l -C -T0  --error-format=msvc" are added to match existing window invocation
     #we might want something different for mac and linux
    add_custom_command(  #自定义生成源文件命令
        OUTPUT ${rl_out}
        COMMAND cd ${outputdir}
        COMMAND ragel ${CMAKE_CURRENT_SOURCE_DIR}/${src_rl} -o ${rl_out} -l -G2  --error-format=msvc
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${src_rl} #保证.rl文件变更时自动生成
        )
    set_source_files_properties(${rl_out} PROPERTIES GENERATED TRUE)
endfunction(ragelmaker)

