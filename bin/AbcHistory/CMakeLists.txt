##-****************************************************************************
##  multiverse - a next generation storage back-end for Alembic
##  Copyright 2015 J CUBE Inc. Tokyo, Japan.             
##
##  Licensed under the Apache License, Version 2.0 (the "License");
##  you may not use this file except in compliance with the License.
##  You may obtain a copy of the License at
##                                                                          
##      http://www.apache.org/licenses/LICENSE-2.0
##
##  Unless required by applicable law or agreed to in writing, software
##  distributed under the License is distributed on an "AS IS" BASIS,
##  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
##  See the License for the specific language governing permissions and
##  limitations under the License.
##-****************************************************************************

ADD_EXECUTABLE( abchistory AbcHistory.cpp)
TARGET_LINK_LIBRARIES( abchistory ${CORE_LIBS} )

set_target_properties(abchistory PROPERTIES
    INSTALL_RPATH_USE_LINK_PATH TRUE
    INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib)

INSTALL(TARGETS abchistory DESTINATION bin)
