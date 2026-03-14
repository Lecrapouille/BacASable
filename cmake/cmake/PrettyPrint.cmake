###############################################################################
# Formatted Console Output
#
# Provides functions for pretty-printing status messages with borders.
# Automatically detects terminal width.
#
# Functions:
#   print_box(<title>)                  - Print a single-line boxed title
#   print_section_start(<title>)        - Start a bordered section
#   print_section_line(<label> <value>) - Print a key-value line
#   print_section_separator()           - Print a separator line
#   print_section_end()                 - End the bordered section
#
###############################################################################


###############################################################################
# Detect Terminal Width
###############################################################################
function(_detect_terminal_width out_var)
    # Try to get terminal width from environment or tput
    if(DEFINED ENV{COLUMNS})
        set(width $ENV{COLUMNS})
    else()
        execute_process(
            COMMAND tput cols
            OUTPUT_VARIABLE width
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
            RESULT_VARIABLE result
        )
        if(NOT result EQUAL 0 OR NOT width)
            set(width 80)
        endif()
    endif()
    
    # Clamp to reasonable range and account for "-- " prefix from message(STATUS)
    if(width GREATER 120)
        set(width 120)
    elseif(width LESS 40)
        set(width 40)
    endif()
    
    # Subtract 4 for "-- " prefix added by message(STATUS)
    math(EXPR width "${width} - 4")
    
    set(${out_var} ${width} PARENT_SCOPE)
endfunction()

# Detect once at configure time
_detect_terminal_width(_PP_WIDTH)

###############################################################################
# print_box(<title>)
# Print a boxed title (single line with borders).
##############################################################################
function(print_box title)
    set(width ${_PP_WIDTH})
    math(EXPR inner_width "${width} - 2")
    
    # Top border
    message(STATUS "")
    string(REPEAT "═" ${inner_width} line)
    message(STATUS "╔${line}╗")
    
    # Centered title
    string(LENGTH "${title}" title_len)
    math(EXPR padding "(${inner_width} - ${title_len}) / 2")
    math(EXPR padding_right "${inner_width} - ${title_len} - ${padding}")
    if(padding LESS 0)
        set(padding 0)
    endif()
    if(padding_right LESS 0)
        set(padding_right 0)
    endif()
    string(REPEAT " " ${padding} left_pad)
    string(REPEAT " " ${padding_right} right_pad)
    message(STATUS "║${left_pad}${title}${right_pad}║")
    
    # Bottom border
    message(STATUS "╚${line}╝")
    message(STATUS "")
endfunction()

###############################################################################
# print_section_start(<title>)
# Print the top of a bordered box with centered title.
##############################################################################
function(print_section_start title)
    set(width ${_PP_WIDTH})
    math(EXPR inner_width "${width} - 2")
    
    # Top border
    string(REPEAT "═" ${inner_width} top_line)
    message(STATUS "")
    message(STATUS "╔${top_line}╗")
    
    # Centered title
    string(LENGTH "${title}" title_len)
    math(EXPR padding "(${inner_width} - ${title_len}) / 2")
    math(EXPR padding_right "${inner_width} - ${title_len} - ${padding}")
    if(padding LESS 0)
        set(padding 0)
    endif()
    if(padding_right LESS 0)
        set(padding_right 0)
    endif()
    string(REPEAT " " ${padding} left_pad)
    string(REPEAT " " ${padding_right} right_pad)
    message(STATUS "║${left_pad}${title}${right_pad}║")
    
    # Separator after title
    string(REPEAT "═" ${inner_width} sep_line)
    message(STATUS "╠${sep_line}╣")
endfunction()

###############################################################################
# print_section_line(<label> <value>)
# Print a key-value line inside the box.
##############################################################################
function(print_section_line label value)
    set(width ${_PP_WIDTH})
    math(EXPR inner_width "${width} - 2")
    
    set(content "  ${label}: ${value}")
    string(LENGTH "${content}" content_len)
    math(EXPR padding "${inner_width} - ${content_len}")
    
    if(padding LESS 0)
        set(padding 0)
        string(SUBSTRING "${content}" 0 ${inner_width} content)
    endif()
    
    string(REPEAT " " ${padding} right_pad)
    message(STATUS "║${content}${right_pad}║")
endfunction()

###############################################################################
# print_section_separator()
# Print a separator line inside the box.
##############################################################################
function(print_section_separator)
    set(width ${_PP_WIDTH})
    math(EXPR inner_width "${width} - 2")
    string(REPEAT "─" ${inner_width} sep_line)
    message(STATUS "╟${sep_line}╢")
endfunction()

###############################################################################
# print_section_end()
# Print the bottom border of the box.
##############################################################################
function(print_section_end)
    set(width ${_PP_WIDTH})
    math(EXPR inner_width "${width} - 2")
    string(REPEAT "═" ${inner_width} bottom_line)
    message(STATUS "╚${bottom_line}╝")
    message(STATUS "")
endfunction()

###############################################################################
# print_status_line(<icon> <text>)
# Print a status line with icon (for build summary).
##############################################################################
function(print_status_line icon text)
    set(width ${_PP_WIDTH})
    math(EXPR inner_width "${width} - 2")
    
    set(content "  ${icon} ${text}")
    string(LENGTH "${content}" content_len)
    math(EXPR padding "${inner_width} - ${content_len}")
    
    if(padding LESS 0)
        set(padding 0)
    endif()
    
    string(REPEAT " " ${padding} right_pad)
    message(STATUS "║${content}${right_pad}║")
endfunction()

###############################################################################
# print_empty_line()
# Print an empty line inside the box.
##############################################################################
function(print_empty_line)
    set(width ${_PP_WIDTH})
    math(EXPR inner_width "${width} - 2")
    string(REPEAT " " ${inner_width} empty)
    message(STATUS "║${empty}║")
endfunction()
