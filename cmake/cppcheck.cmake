set(CMAKE_CXX_CPPCHECK "cppcheck")
list(APPEND CMAKE_CXX_CPPCHECK "--suppressions-list=${CMAKE_SOURCE_DIR}/CppCheckSuppressions.txt")
list(APPEND CMAKE_CXX_CPPCHECK "--enable=all" "--quiet" "--force" "-UX86_UNIT_TESTING_ONLY" "--inline-suppr" )
list(APPEND CMAKE_CXX_CPPCHECK "--suppress=missingIncludeSystem")
