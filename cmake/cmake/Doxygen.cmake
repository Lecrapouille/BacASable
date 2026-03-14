###############################################################################
# Documentation Generation
#
# This module configures Doxygen for generating API documentation from
# source code comments.
#
# Prerequisites:
#   - Doxygen: sudo apt install doxygen
#   - Graphviz (for diagrams): sudo apt install graphviz
#
# Usage:
#   cmake -S . -B build -DENABLE_DOXYGEN=ON
#   cmake --build build --target docs
#
# Generated files:
#   - build/docs/html/           : HTML documentation
#   - build/docs/html/index.html : Entry point
###############################################################################


###############################################################################
# ENABLE_DOXYGEN - Doxygen documentation generation
#
# Generate HTML documentation from source comments with Doxygen.
#
# Usage:
#   cmake -S . -B build -DENABLE_DOXYGEN=ON
#   cmake --build build --target docs
###############################################################################

option(ENABLE_DOXYGEN "Enable Doxygen documentation generation" OFF)


if(ENABLE_DOXYGEN)

    # Find Doxygen
    find_package(Doxygen
        OPTIONAL_COMPONENTS dot  # Graphviz for diagrams
    )

    if(NOT DOXYGEN_FOUND)
        message(WARNING
            "Doxygen not found. Install with: sudo apt install doxygen graphviz\n"
            "Documentation generation will be disabled."
        )
        return()
    endif()

    message(STATUS "Doxygen found: ${DOXYGEN_EXECUTABLE}")

    # Doxygen Configuration
    # These variables configure doxygen_add_docs() behavior.
    # They correspond to Doxyfile settings.

    # Project information
    set(DOXYGEN_PROJECT_NAME "${PROJECT_NAME}")
    set(DOXYGEN_PROJECT_NUMBER "${PROJECT_VERSION}")
    set(DOXYGEN_PROJECT_BRIEF "${PROJECT_DESCRIPTION}")

    # Input settings
    set(DOXYGEN_RECURSIVE YES)
    set(DOXYGEN_EXTRACT_ALL YES)
    set(DOXYGEN_EXTRACT_PRIVATE YES)
    set(DOXYGEN_EXTRACT_STATIC YES)

    # Source browsing
    set(DOXYGEN_SOURCE_BROWSER YES)
    set(DOXYGEN_INLINE_SOURCES NO)
    set(DOXYGEN_STRIP_CODE_COMMENTS NO)
    set(DOXYGEN_REFERENCED_BY_RELATION YES)
    set(DOXYGEN_REFERENCES_RELATION YES)

    # Output settings
    set(DOXYGEN_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/docs")
    set(DOXYGEN_GENERATE_HTML YES)
    set(DOXYGEN_GENERATE_LATEX NO)
    set(DOXYGEN_HTML_OUTPUT "html")

    # HTML appearance
    set(DOXYGEN_HTML_COLORSTYLE_HUE 220)
    set(DOXYGEN_HTML_COLORSTYLE_SAT 100)
    set(DOXYGEN_HTML_COLORSTYLE_GAMMA 80)
    set(DOXYGEN_HTML_DYNAMIC_SECTIONS YES)
    set(DOXYGEN_GENERATE_TREEVIEW YES)
    set(DOXYGEN_DISABLE_INDEX NO)
    set(DOXYGEN_FULL_SIDEBAR NO)

    # Diagrams (requires Graphviz)
    if(DOXYGEN_DOT_FOUND)
        set(DOXYGEN_HAVE_DOT YES)
        set(DOXYGEN_DOT_IMAGE_FORMAT svg)
        set(DOXYGEN_INTERACTIVE_SVG YES)
        set(DOXYGEN_CALL_GRAPH YES)
        set(DOXYGEN_CALLER_GRAPH YES)
        set(DOXYGEN_CLASS_DIAGRAMS YES)
        set(DOXYGEN_COLLABORATION_GRAPH YES)
        set(DOXYGEN_UML_LOOK YES)
        set(DOXYGEN_DOT_UML_DETAILS YES)
        message(STATUS "Graphviz found: diagrams enabled")
    else()
        set(DOXYGEN_HAVE_DOT NO)
        message(STATUS "Graphviz not found: diagrams disabled. Install: sudo apt install graphviz")
    endif()

    # Warnings
    set(DOXYGEN_WARN_IF_UNDOCUMENTED YES)
    set(DOXYGEN_WARN_IF_DOC_ERROR YES)
    set(DOXYGEN_WARN_NO_PARAMDOC YES)

    # Exclusions
    set(DOXYGEN_EXCLUDE_PATTERNS
        "*/build/*"
        "*/tests/*"
        "*/.git/*"
        "*/cmake/*"
    )

    # Create Documentation Target
    #
    # Usage: cmake --build build --target docs
    #
    # doxygen_add_docs() creates a target that:
    #   1. Generates Doxyfile from the DOXYGEN_* variables
    #   2. Runs doxygen to generate documentation
    doxygen_add_docs(docs
        ${CMAKE_SOURCE_DIR}
        COMMENT "Generating API documentation with Doxygen..."
    )

    # Documentation Clean Target
    #
    # Usage: cmake --build build --target docs-clean
    add_custom_target(docs-clean
        COMMENT "Cleaning documentation..."
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/docs
        VERBATIM
    )

    # Summary
    message(STATUS "Documentation target 'docs' created")
    message(STATUS "  Run: cmake --build build --target docs")
    message(STATUS "  Output: build/docs/html/index.html")

    add_enabled_feature("Doxygen")

endif()
