#!/usr/bin/env awk -f

# Generate env.h from .env file
# Usage: awk -f scripts/generate_env.awk ../../.env > src/env.h

BEGIN {
  print "#ifndef CUAS_MESH_ENV_H"
  print "#define CUAS_MESH_ENV_H"
  print ""
  print "/***   GENERATED FILE - DO NOT EDIT   ***/"
  print ""
  print "// Generated from .env by scripts/generate_env.awk"
  print ""
}

/^[[:space:]]*$/ { next }
/^[[:space:]]*#/ { next }

{
    pos = index($0, "=")
    if (pos > 0) {
        key = substr($0, 1, pos-1)
        value = substr($0, pos+1)
        gsub(/^[ \t]+/, "", key)
        gsub(/[ \t]+$/, "", key)
        gsub(/^[ \t]+/, "", value)
        gsub(/[ \t]+$/, "", value)
        if (key ~ /^[A-Z_][A-Z0-9_]*$/) {
            if (substr(value, 1, 1) != "\"" || substr(value, length(value), 1) != "\"") {
                value = "\"" value "\""
            }
            print "#define " key " " value
        }
    }
}

END {
    print ""
    print "#endif // CUAS_MESH_ENV_H"
}
