cmake_minimum_required(VERSION 3.7)
option(ENABLE_PCH_HEADERS_FOR_UE4Library "Enable precompiled headers" OFF)
if (ENABLE_PCH_HEADERS_FOR_UE4Library)
    set(PCH_CPP_OPT pch_UE4Library.cpp)
else ()
    set(PCH_CPP_OPT "")
endif ()
add_library(UE4Library STATIC UE4Library/StringRange.cpp
UE4Library/LogMessageInfo.cpp
UE4Library/UnrealLogEvent.cpp
UE4Library/UClass.cpp
UE4Library/BlueprintFunction.cpp
UE4Library/ScriptCallStackFrame.cpp
UE4Library/IScriptCallStack.cpp
UE4Library/EmptyScriptCallStack.cpp
UE4Library/ScriptCallStack.cpp
UE4Library/UnableToDisplayScriptCallStack.cpp
UE4Library/IScriptMsg.cpp
UE4Library/ScriptMsgException.cpp
UE4Library/ScriptMsgCallStack.cpp
UE4Library/BlueprintHighlighter.cpp
UE4Library/BlueprintReference.cpp
UE4Library/UE4Library.cpp
UE4Library/IScriptCallStack_Unknown.cpp
UE4Library/IScriptMsg_Unknown.cpp
UE4Library/StringRange.h
UE4Library/LogMessageInfo.h
UE4Library/UnrealLogEvent.h
UE4Library/UClass.h
UE4Library/BlueprintFunction.h
UE4Library/ScriptCallStackFrame.h
UE4Library/IScriptCallStack.h
UE4Library/EmptyScriptCallStack.h
UE4Library/ScriptCallStack.h
UE4Library/UnableToDisplayScriptCallStack.h
UE4Library/IScriptMsg.h
UE4Library/ScriptMsgException.h
UE4Library/ScriptMsgCallStack.h
UE4Library/BlueprintHighlighter.h
UE4Library/BlueprintReference.h
UE4Library/UE4Library.h
UE4Library/IScriptCallStack_Unknown.h
UE4Library/IScriptMsg_Unknown.h
instantiations.h
instantiations.cpp
${PCH_CPP_OPT})

target_include_directories(UE4Library PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} UE4Library)
target_link_libraries(UE4Library PUBLIC rd_framework_cpp)
if (ENABLE_PCH_HEADERS_FOR_UE4Library)
    include(PrecompiledHeader.cmake)
    add_precompiled_header(UE4Library pch_UE4Library.h SOURCE_CXX pch_UE4Library.cpp FORCEINCLUDE)
endif ()
