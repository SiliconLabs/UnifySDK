# Asset tag database component

add_library(db src/aoa_db.c)

target_include_directories(db
  PUBLIC
  include
  PRIVATE
)

target_link_libraries(db bluetooth_api)
