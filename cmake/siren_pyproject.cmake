file(READ ${CMAKE_CURRENT_LIST_DIR}/../pyproject.toml PYPROJECT_CONTENTS)
string(REGEX MATCH "\\[project\\](\n(([A-Za-z0-9_])+([ \t\n\r])*=([ \t\n\r])*(\".*\"|\[.*\]|true|false))?([ \t\n\r])*)*([ \t]*name[ \t]*=[ \t]*\"siren\"[ \t]*)[ \t\n\r]+([ \t]*version[ \t]*=[ \t]\"[0-9\.]+\")" SIREN_VERSION_BLOCK ${PYPROJECT_CONTENTS})
string(REGEX MATCH "([ \t]*version[ \t]*=[ \t]\"[0-9\.]+\")" SIREN_VERSION_LINE ${SIREN_VERSION_BLOCK})
string(REGEX MATCH "\"[0-9\.]+\"" SIREN_VERSION_STRING ${SIREN_VERSION_LINE})
string(REGEX MATCH "[0-9\.]+" SIREN_VERSION ${SIREN_VERSION_STRING})