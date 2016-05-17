# output information about installation-directories and locations

message("")
message("* Compile to     : ${CMAKE_INSTALL_PREFIX}")
message("")

# Show infomation about the options selected during configuration

if (SERVER)
  message("* BUILD SERVER : Yes (default)")
else()
  message("* Build SERVER : No")
endif()

if (CLIENT)
  message("* BUILD CLIENT : Yes")
else()
  message("* Build CLIENT : No")
endif()

message("")

