# DecoyPDF Module

target_sources(
    shellcode-pe
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/DecoyPDF.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/embedded_pdf.cpp
)

target_sources(
    test5shellcode-pe
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/DecoyPDF.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/embedded_pdf.cpp
)
