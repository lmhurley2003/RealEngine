cmake_minimum_required(VERSION 3.5)
project(buildShaders)

function(compile_shaders)
    if(WIN32)
        execute_process(
            COMMAND cmd /c compile.bat
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/shaders
            RESULT_VARIABLE COMPILE_RESULT
        )

        if(NOT COMPILE_RESULT EQUAL 0)
            message(FATAL_ERROR "Shader compilation failed with code ${COMPILE_RESULT}")
        else()
            message(STATUS "Shader compilation succeeded.")
        endif()
    endif()
endfunction()

function(create_resources dir)
    # Collect input files
    file(GLOB bins ${dir}/*)
    foreach(bin ${bins})
        # Get short filename
        string(REGEX MATCH "([^/]+)$" filename ${bin})
        # Replace filename spaces & extension separator for C compatibility
        string(REGEX REPLACE "\\.| |-" "_" filename ${filename})

        # get rid of _spv extension
        string(LENGTH ${filename} STRING_LENGTH)
        math(EXPR NEW_LENGTH "${STRING_LENGTH} - 4")
        string(SUBSTRING ${filename} 0 "${NEW_LENGTH}" trimmedFileName)

        file(WRITE "shaders/embedded/${trimmedFileName}.cpp" "[[maybe_unused]]\n")
        # Read hex data from file
        file(READ ${bin} filedata HEX)
        # Convert hex data for C compatibility
        string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," filedata ${filedata})
        # Append data to output file
        file(APPEND "shaders/embedded/${trimmedFileName}.cpp" "const unsigned char ${trimmedFileName}[] = {${filedata}};\n[[maybe_unused]]\nconst unsigned ${trimmedFileName}Size = sizeof(${trimmedFileName});\n")
    endforeach()

endfunction()
