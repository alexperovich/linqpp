set(INCLUDES
  ../inc/pch.h
  ../inc/enumerable.hpp
)

set(SOURCES
  main.cpp
  select.cpp
  where.cpp
)

add_precompiled_header(pch.h pch.cpp SOURCES)

add_executable(main
  ${SOURCES}
  ${INCLUDES}
)

