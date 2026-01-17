if(NOT DEFINED FMT)
  message(FATAL_ERROR "FMT not set")
endif()
if(NOT DEFINED INPUT)
  message(FATAL_ERROR "INPUT not set")
endif()
if(NOT DEFINED OUTPUT)
  message(FATAL_ERROR "OUTPUT not set")
endif()

execute_process(
  COMMAND "${FMT}" -s "${INPUT}"
  OUTPUT_FILE "${OUTPUT}"
  RESULT_VARIABLE _res
)

if(NOT _res EQUAL 0)
  message(FATAL_ERROR "fmt failed with code ${_res}")
endif()
