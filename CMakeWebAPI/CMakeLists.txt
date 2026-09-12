# CMakeList.txt : CMakeWebAPI の CMake プロジェクト。ソースを含めて、次を定義します:
# プロジェクト専用ロジックはこちらです。
#
cmake_minimum_required (VERSION 3.10)

# サポートされている場合は、MSVC コンパイラのホット リロードを有効にします。
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("CMakeWebAPI")


# cpp-httplib を GitHub から自動取得
include(FetchContent)
FetchContent_Declare(
    httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG v0.56.0
)
FetchContent_MakeAvailable(httplib)


# ソースをこのプロジェクトの実行可能ファイルに追加します。
add_executable (CMWebAPI "CMakeWebAPI.cpp" "CMakeWebAPI.h")


# cpp-httplib をリンク (マルチスレッド対応用 Threads ライブラリもリンク)
find_package(Threads REQUIRED)
target_link_libraries(CMWebAPI PRIVATE httplib::httplib Threads::Threads)


if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET CMWebAPI PROPERTY CXX_STANDARD 20)
endif()

# TODO: テストを追加し、必要な場合は、ターゲットをインストールします。
