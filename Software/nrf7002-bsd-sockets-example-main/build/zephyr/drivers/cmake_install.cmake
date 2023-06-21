# Install script for directory: C:/ncs/v2.4.0/zephyr/drivers

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files/Zephyr-Kernel")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "TRUE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "C:/ncs/toolchains/31f4403e35/opt/zephyr-sdk/arm-zephyr-eabi/bin/arm-zephyr-eabi-objdump.exe")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/sylve/OneDrive/Bureau/Telemtetry-for-the-Formula-Student/Software/nrf7002-bsd-sockets-example-main/build/zephyr/drivers/disk/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/sylve/OneDrive/Bureau/Telemtetry-for-the-Formula-Student/Software/nrf7002-bsd-sockets-example-main/build/zephyr/drivers/interrupt_controller/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/sylve/OneDrive/Bureau/Telemtetry-for-the-Formula-Student/Software/nrf7002-bsd-sockets-example-main/build/zephyr/drivers/misc/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/sylve/OneDrive/Bureau/Telemtetry-for-the-Formula-Student/Software/nrf7002-bsd-sockets-example-main/build/zephyr/drivers/pcie/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/sylve/OneDrive/Bureau/Telemtetry-for-the-Formula-Student/Software/nrf7002-bsd-sockets-example-main/build/zephyr/drivers/usb/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/sylve/OneDrive/Bureau/Telemtetry-for-the-Formula-Student/Software/nrf7002-bsd-sockets-example-main/build/zephyr/drivers/usb_c/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/sylve/OneDrive/Bureau/Telemtetry-for-the-Formula-Student/Software/nrf7002-bsd-sockets-example-main/build/zephyr/drivers/clock_control/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/sylve/OneDrive/Bureau/Telemtetry-for-the-Formula-Student/Software/nrf7002-bsd-sockets-example-main/build/zephyr/drivers/console/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/sylve/OneDrive/Bureau/Telemtetry-for-the-Formula-Student/Software/nrf7002-bsd-sockets-example-main/build/zephyr/drivers/entropy/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/sylve/OneDrive/Bureau/Telemtetry-for-the-Formula-Student/Software/nrf7002-bsd-sockets-example-main/build/zephyr/drivers/gpio/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/sylve/OneDrive/Bureau/Telemtetry-for-the-Formula-Student/Software/nrf7002-bsd-sockets-example-main/build/zephyr/drivers/led/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/sylve/OneDrive/Bureau/Telemtetry-for-the-Formula-Student/Software/nrf7002-bsd-sockets-example-main/build/zephyr/drivers/mbox/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/sylve/OneDrive/Bureau/Telemtetry-for-the-Formula-Student/Software/nrf7002-bsd-sockets-example-main/build/zephyr/drivers/ethernet/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/sylve/OneDrive/Bureau/Telemtetry-for-the-Formula-Student/Software/nrf7002-bsd-sockets-example-main/build/zephyr/drivers/pinctrl/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/sylve/OneDrive/Bureau/Telemtetry-for-the-Formula-Student/Software/nrf7002-bsd-sockets-example-main/build/zephyr/drivers/serial/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/sylve/OneDrive/Bureau/Telemtetry-for-the-Formula-Student/Software/nrf7002-bsd-sockets-example-main/build/zephyr/drivers/timer/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/sylve/OneDrive/Bureau/Telemtetry-for-the-Formula-Student/Software/nrf7002-bsd-sockets-example-main/build/zephyr/drivers/wifi/cmake_install.cmake")
endif()

